// emitter.cpp

#include "Common.hpp"
#include "Emitter.hpp"

Emitter::Emitter(ContextoArcanos& ca)
  : contextoArcanos(ca) {
  llvm_modulo  = std::make_unique<llvm::Module>("ArcanaModulo", llvm_ctx);
  llvm_builder = std::make_unique<llvm::IRBuilder<>>(llvm_ctx);
}

// --- LLVM --- //
llvm::Type* Emitter::obtenerTipoLLVM(std::shared_ptr<ArcanaType> tipo) {
  if (!tipo) { return nullptr; }

  switch (tipo->kind) {
    case TypeKind::VOID: {
      return llvm::Type::getVoidTy(llvm_ctx);
    }

    case TypeKind::INTEGER: {
      return llvm::Type::getIntNTy(llvm_ctx, tipo->getBitSize());
    }

    case TypeKind::FLOAT: {
      switch(tipo->getBitSize()) {
        case 16 : { return llvm::Type::getHalfTy  (llvm_ctx); }
        case 32 : { return llvm::Type::getFloatTy (llvm_ctx); }
        case 64 : { return llvm::Type::getDoubleTy(llvm_ctx); }
        case 128: { return llvm::Type::getFP128Ty (llvm_ctx); }
        default : { return nullptr; }
      }
    }

    default: {
      return nullptr;
    }

  }
}

void Emitter::generarArchivoIR(const std::filesystem::path& nombreArchivo) {
  std::error_code ec;

  llvm::raw_fd_ostream archivo(std::string(nombreArchivo), ec, llvm::sys::fs::OF_None);

  if (ec) {
    std::cerr << "Error al abrir el archivo apra escribir IR: " << ec.message() << '\n';
    return ;

  }

  llvm_modulo->print(archivo, nullptr);

  // Imprimir por stdout
  // llvm_modulo->print(llvm::errs(), nullptr);

}

//... Everything after this line has to be triple checked.

// --- Expresiones --- //
void Emitter::visitar(ExprNumero* nodo) { //...
  if (nodo->tipo_resuelto.valor->kind == TypeKind::FLOAT) {
    llvm_valor = llvm::ConstantFP::get(llvm_ctx, llvm::APFloat(std::stod(nodo->valor)));

  } else {
    int bits = nodo->tipo_resuelto.valor->getBitSize();
    llvm_valor = llvm::ConstantInt::get(llvm_ctx, llvm::APInt(bits, nodo->valor, 10));

  }
}

void Emitter::visitar(ExprVariable* nodo) {

  if (bloques_arcano_activos.count(nodo->nombre)) {
    bloques_arcano_activos[nodo->nombre]->accept(this);
    return ;
  }

  llvm::AllocaInst* alloca = nullptr;
  for (auto it = llvm_scopes.rbegin(); it != llvm_scopes.rend(); ++it) {
    if (it->count(nodo->nombre)) {
      alloca = (*it)[nodo->nombre];
      break;
    }
  }

  if (!alloca) {
    std::cerr << "Error: Variable '" << nodo->nombre << "' no encontrada.\n";
    return;
  }

  //llvm::Type* tipoLLVM = obtenerTipoLLVM(nodo->tipo_resuelto.valor);
  llvm::Type* tipoLLVM = alloca->getAllocatedType();

  llvm_valor = llvm_builder->CreateLoad(tipoLLVM, alloca, nodo->nombre + "_val");

}

void Emitter::visitar(ExprArray* nodo) {

}

void Emitter::visitar(ExprUnaria* nodo) {
  if (nodo->operador == TipoOperador::PTR_REF) {
    if (auto* var = dynamic_cast<ExprVariable*>(nodo->operando.get())) {
      if (llvm_scopes.back().count(var->nombre)) {
        llvm_valor = llvm_scopes.back()[var->nombre];
        return ;
      }
    }

    return ;
  }

  nodo->operando->accept(this);
  llvm::Value* val = llvm_valor;

  if (!val) { return ; }

  switch (nodo->operador) { //...

    case TipoOperador::BITWISE_NO:
    case TipoOperador::LOGICO_NO: {
      llvm_valor = llvm_builder->CreateNot(val, "not_temp");
      break;
    }

    default: {
      std::cout << "[129, emitter.cpp] Error: Operador unario no implementado.";
      exit(1);
    }
  }

}

void Emitter::visitar(ExprBinaria* nodo) {
  nodo->izquierda->accept(this);
  llvm::Value* L = llvm_valor;

  nodo->derecha->accept(this);
  llvm::Value* R = llvm_valor;

  bool es_float = nodo->tipo_resuelto.valor->kind == TypeKind::FLOAT;

  switch (nodo->operador) { //...
    case TipoOperador::A_SUMA: {
      llvm_valor = es_float ? llvm_builder->CreateFAdd(L, R, "addtemp")
                            : llvm_builder->CreateAdd (L, R, "addtemp");
      break;
    }

    case TipoOperador::A_RESTA: {
      llvm_valor = es_float ? llvm_builder->CreateFSub(L, R, "subtemp")
                            : llvm_builder->CreateSub(L, R, "subtemp");
      break;
    }

    case TipoOperador::A_MULT: {
      llvm_valor = es_float ? llvm_builder->CreateFMul(L, R, "multtemp")
                            : llvm_builder->CreateMul(L, R, "multtemp");
      break;
    }

    default: {
      break;
    }
  }

}

void Emitter::visitar(ExprCasteo* nodo) {
  nodo->expresion->accept(this);
  llvm::Value* val = llvm_valor;

  std::shared_ptr<ArcanaType> t_origen  = nodo->expresion->tipo_resuelto.valor;
  std::shared_ptr<ArcanaType> t_destino = nodo->tipo_resuelto.valor;

  if (t_destino->kind == TypeKind::BOOLEAN) {
    switch (t_origen->kind) {

      case TypeKind::INTEGER: {
        llvm_valor = llvm_builder->CreateICmpNE(
          val,
          llvm::ConstantInt::get(val->getType(), 0),
          "cast_temp"
        );
        break;
      }

      case TypeKind::FLOAT: {
        llvm_valor = llvm_builder->CreateFCmpUNE(val,
                                                 llvm::ConstantFP::get(val->getType(), 0.0),
                                                 "cast_temp"
        );
        break;
      }


    }

    return ;
  }

  llvm::Type* tipo_destino_llvm = obtenerTipoLLVM(t_destino);

  bool origen_signo  = t_origen ->isSigned();
  bool destino_signo = t_destino->isSigned();

  llvm::Instruction::CastOps cast_op = llvm::CastInst::getCastOpcode(
    val, origen_signo, tipo_destino_llvm, destino_signo
  );

  llvm_valor = llvm_builder->CreateCast(cast_op, val, tipo_destino_llvm, "cast_temp");

}

void Emitter::visitar(ExprRango* nodo) {

}

void Emitter::visitar(ExprAcceso* nodo) {

}

void Emitter::visitar(ExprLlamadaArcano* nodo) {

}

void Emitter::visitar(ExprFuncCall* nodo) {
  auto* var_callee = dynamic_cast<ExprVariable*>(nodo->callee.get());
  if (!var_callee) {
    //...
    return ;
  }

  llvm::Function* callee_f = llvm_modulo->getFunction(var_callee->nombre);

  if (!callee_f) { // Trust me, there is no way the code ends up here.
    std::cerr << "Error: Función " << var_callee->nombre << "no encontrada.\n";
    return ;

  }

  std::vector<llvm::Value*> args_v;
  for (auto& arg : nodo->argumentos) {
    arg.second->accept(this);
    args_v.push_back(llvm_valor);

  }

  llvm_valor = llvm_builder->CreateCall(callee_f, args_v, "calltmp");

}

// --- Sentencias --- //

void Emitter::visitar(Bloque* nodo) {
  llvm_scopes.push_back(std::map<std::string, llvm::AllocaInst*>());

  for (const auto& i : nodo->instrucciones) {
    i->accept(this);

  }

  llvm_scopes.pop_back();

}

void Emitter::visitar(SentenciaVar* nodo) {
  llvm::Type* tipoLLVM = obtenerTipoLLVM(nodo->tipo_explicito.tipo.valor);

  llvm::AllocaInst* alloca = llvm_builder->CreateAlloca(tipoLLVM, nullptr, nodo->nombre);

  llvm_scopes.back()[nodo->nombre] = alloca;

  if (nodo->valor_inicial) {
    nodo->valor_inicial->accept(this);
    llvm::Value* valor_inicial = llvm_valor;

    llvm_builder->CreateStore(valor_inicial, alloca);

  }

}

void Emitter::visitar(SentenciaExpr* nodo) {
  if (nodo->expresion) {
    nodo->expresion->accept(this);
  }

}

void Emitter::visitar(SentenciaAsignacion* nodo) {
  nodo->derecha->accept(this);
  llvm::Value* valor_der = llvm_valor;

  auto* var_izq = dynamic_cast<ExprVariable*>(nodo->izquierda.get());

  if (var_izq) {
    llvm::AllocaInst* alloca = nullptr;
    for (auto it = llvm_scopes.rbegin(); it != llvm_scopes.rend(); ++it) {
      if (it->count(var_izq->nombre)) {
        alloca = (*it)[var_izq->nombre];
        break;
      }
    }

  llvm_builder->CreateStore(valor_der, alloca);

  }

}

void Emitter::visitar(SentenciaSi* nodo) { //...
  nodo->condicion->accept(this);
  llvm::Value* cond_v = llvm_valor;

  llvm::Value* cero =
    llvm::ConstantInt::get(llvm_ctx,
                           llvm::APInt(cond_v->getType()->getIntegerBitWidth(),0)
                           );

  cond_v = llvm_builder->CreateICmpNE(cond_v, cero, "ifcond");

  llvm::Function* funcion_actual = llvm_builder->GetInsertBlock()->getParent();

  llvm::BasicBlock* then_bb  = llvm::BasicBlock::Create(llvm_ctx, "then", funcion_actual);
  llvm::BasicBlock* else_bb  = llvm::BasicBlock::Create(llvm_ctx, "else");
  llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(llvm_ctx, "ifcont");

  llvm_builder->CreateCondBr(cond_v, then_bb, else_bb);

  llvm_builder->SetInsertPoint(then_bb);
  nodo->rama_si->accept(this);

  if (!llvm_builder->GetInsertBlock()->getTerminator()) {
    llvm_builder->CreateBr(merge_bb);
  }

  funcion_actual->insert(funcion_actual->end(), else_bb);
  llvm_builder->SetInsertPoint(else_bb);

  if (nodo->rama_sino) {
    nodo->rama_sino->accept(this);
  }

  if (!llvm_builder->GetInsertBlock()->getTerminator()) {
    llvm_builder->CreateBr(merge_bb);
  }

  if (merge_bb->use_empty()) {
    delete merge_bb;

  } else {
    funcion_actual->insert(funcion_actual->end(), merge_bb);
    llvm_builder->SetInsertPoint(merge_bb);

  }
}

void Emitter::visitar(SentenciaSino* nodo) {
  if (nodo->cuerpo) {
    nodo->cuerpo->accept(this);
  }

}

void Emitter::visitar(SentenciaMientras* nodo) {
  llvm::Function* funcion_actual = llvm_builder->GetInsertBlock()->getParent();

  llvm::BasicBlock* cond_bb  = llvm::BasicBlock::Create(llvm_ctx, "while.cond", funcion_actual);
  llvm::BasicBlock* loop_bb  = llvm::BasicBlock::Create(llvm_ctx, "while.body");
  llvm::BasicBlock* else_bb  = llvm::BasicBlock::Create(llvm_ctx, "while.else");
  llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(llvm_ctx, "while.end");

  llvm_builder->CreateBr(cond_bb);

  llvm_builder->SetInsertPoint(cond_bb);
  nodo->condicion->accept(this);
  llvm::Value* cond_v = llvm_valor;

  llvm::Value* cero =
    llvm::ConstantInt::get(llvm_ctx,
                           llvm::APInt(cond_v->getType()->getIntegerBitWidth(), 0)
                           );
  cond_v = llvm_builder->CreateICmpNE(cond_v, cero, "whilecond");

  llvm_builder->CreateCondBr(cond_v, loop_bb, else_bb);

  pila_continues.push_back(cond_bb);
  pila_breaks.push_back(merge_bb);

  funcion_actual->insert(funcion_actual->end(), loop_bb);
  llvm_builder->SetInsertPoint(loop_bb);

  if (nodo->rama_while) {
    nodo->rama_while->accept(this);
  }

  if (!llvm_builder->GetInsertBlock()->getTerminator()) {
    llvm_builder->CreateBr(cond_bb);
  }

  pila_continues.pop_back();
  pila_breaks.pop_back();

  funcion_actual->insert(funcion_actual->end(), else_bb);
  llvm_builder->SetInsertPoint(else_bb);

  if (nodo->rama_sino) {
    nodo->rama_sino->accept(this);
  }

  if (!llvm_builder->GetInsertBlock()->getTerminator()) {
    llvm_builder->CreateBr(merge_bb);
  }

  funcion_actual->insert(funcion_actual->end(), merge_bb);
  llvm_builder->SetInsertPoint(merge_bb);

}

void Emitter::visitar(SentenciaBreak* nodo) {
  if (pila_breaks.empty()) { //...
    std::cerr << "Error: 'break' fuera de un bucle.\n";
    return ;
  }

  llvm::BasicBlock* bloque_salida = pila_breaks.back();
  llvm_builder->CreateBr(bloque_salida);

}

void Emitter::visitar(SentenciaContinue* nodo) {
  if (pila_continues.empty()) { //...
    std::cerr << "Error: 'continue' fuera de un bucle.\n";
    return ;
  }

  llvm::BasicBlock* bloque_condicion = pila_continues.back();
  llvm_builder->CreateBr(bloque_condicion);

}

void Emitter::visitar(SentenciaReturn* nodo) {
  nodo->ret_value->accept(this);
  llvm_builder->CreateRet(llvm_valor);

}

void Emitter::visitar(SentenciaFuncDecl* nodo) {
  std::vector<llvm::Type*> tipo_args;

  for (auto const& [nombre, info] : nodo->args_type) {
    tipo_args.push_back((obtenerTipoLLVM(info.tipo.valor)));
  }

  llvm::Type* tipo_ret = obtenerTipoLLVM(nodo->ret_type.valor);

  llvm::FunctionType* ft = llvm::FunctionType::get(tipo_ret, tipo_args, false);

  llvm::Function* f = llvm::Function::Create(
    ft,
    llvm::Function::ExternalLinkage,
    nodo->nombre_func,
    llvm_modulo.get()
  );

  if (!nodo->cuerpo_func) {
    return ;
  }

  llvm::BasicBlock* bb = llvm::BasicBlock::Create(llvm_ctx, "entry", f);
  llvm_builder->SetInsertPoint(bb);

  llvm_scopes.push_back(std::map<std::string, llvm::AllocaInst*>());

  unsigned int idx = 0;
  auto it_args = nodo->args_type.begin();

  for (auto &arg : f->args()) {
    const std::string& nombre_arg = it_args->first;
    arg.setName(nombre_arg);

    llvm::AllocaInst* alloca = llvm_builder->CreateAlloca(arg.getType(), nullptr, nombre_arg);
    llvm_builder->CreateStore(&arg, alloca);

    llvm_scopes.back()[nombre_arg] = alloca;

    it_args++;

  }

  nodo->cuerpo_func->accept(this);

  llvm_scopes.pop_back();

  llvm::verifyFunction(*f);

}

void Emitter::visitar(SentenciaEscritura* nodo) {

}

void Emitter::visitar(SentenciaArcano* nodo) {

}

void Emitter::visitar(SentenciaLlamadaArcano* nodo) { //...
  ArcaneDef& def = contextoArcanos.buscarDefinicionPorKeyword(nodo->nombre);

  if (nodo->indice_rama >= def.branches.size()) {
    std::cerr << "Error interno (AST corrupto): Índice de rama fuera de rango para '" << nodo->nombre << "'.\n";
    exit(1);
  }

  ArcaneBranch* rama_elegida = &def.branches[nodo->indice_rama];

  //for (auto& branch : def.branches) {
  //  if (branch.segmentos[0].br_key == nodo->nombre) {

  //    size_t params_esperados = branch.segmentos[0].br_args.size();

  //    size_t args_pasados = 0;
  //    for (auto const& [nom, ast] : nodo->argumentos) {
  //      bool es_codigo = false;
  //      for (auto const& arg_def : def.args) {
  //        if (arg_def.contenido == nom && arg_def.tipo_dato == TPA::CODE) {
  //          es_codigo = true;
  //          break;
  //        }
  //      }
  //      if (!es_codigo) { args_pasados++; }
  //    }

  //    if (params_esperados == args_pasados) {
  //      rama_elegida = &branch;
  //      break;
  //    }
  //  }
  //}

  //if (!rama_elegida) {
  //  std::cerr << "Error: No se pudo resolver la rama a emitir para el Arcano '" << nodo->nombre << "'.\n";
  //  return ;
  //}

  llvm_scopes.push_back(std::map<std::string, llvm::AllocaInst*>());
  auto backup_bloques = bloques_arcano_activos;

  for (const auto& [nombre_arg, ast_arg] : nodo->argumentos) {
    if (!ast_arg) { continue; }

    bool es_lazy = false;

    for (const auto& arg_def : def.args) {
      if (arg_def.contenido == nombre_arg &&
         (arg_def.tipo_dato == TPA::CODE  ||
          arg_def.tipo_dato == TPA::EXPR  )) {
        es_lazy = true;
        break;
      }
    }

    if (es_lazy) {
      bloques_arcano_activos[nombre_arg] = ast_arg.get();

    } else {
      ast_arg->accept(this);
      llvm::Value* valor_arg = llvm_valor;

      llvm::AllocaInst* alloca = llvm_builder->CreateAlloca(valor_arg->getType(), nullptr, nombre_arg);
      llvm_builder->CreateStore(valor_arg, alloca);
      llvm_scopes.back()[nombre_arg] = alloca;

    }

  }

  for (const auto& seg: rama_elegida->segmentos) {
    if (seg.br_cont) {
      seg.br_cont->accept(this);
    }
  }

  bloques_arcano_activos = backup_bloques;
  llvm_scopes.pop_back();

}
