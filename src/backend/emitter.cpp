// emitter.cpp

#include "Common.hpp"
#include "Emitter.hpp"

Emitter::Emitter(ContextoArcanos& ca, GestorTablas& t)
  : contextoArcanos(ca), tablas(t) {
  llvm_modulo  = std::make_unique<llvm::Module>("ArcanaModulo", llvm_ctx);
  llvm_builder = std::make_unique<llvm::IRBuilder<>>(llvm_ctx);

  tablas.prepareForEmitter();

}

// --- LLVM --- //
llvm::Type* Emitter::obtenerTipoLLVM(std::shared_ptr<ArcanaType> tipo) {
  if (!tipo) { return nullptr; }

  switch (tipo->kind) {

    case TypeKind::VOID: {
      return llvm::Type::getVoidTy(llvm_ctx);
    }

    case TypeKind::BOOLEAN: {
      return llvm::Type::getInt1Ty(llvm_ctx);
    }

    case TypeKind::CHAR   :
    case TypeKind::INTEGER: {
      return llvm::Type::getIntNTy(llvm_ctx, tipo->getBitSize());
    }

    case TypeKind::FLOAT: {
      switch(tipo->getBitSize()) {
        case 16 : { return llvm::Type::getHalfTy    (llvm_ctx); }
        case 32 : { return llvm::Type::getFloatTy   (llvm_ctx); }
        case 64 : { return llvm::Type::getDoubleTy  (llvm_ctx); }
        case 80 : { return llvm::Type::getX86_FP80Ty(llvm_ctx); }
        case 128: { return llvm::Type::getFP128Ty   (llvm_ctx); }
        default : { return nullptr; }
      }
    }

    case TypeKind::POINTER: {
      return llvm::PointerType::getUnqual(llvm_ctx);
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
    std::cerr << "Error al abrir el archivo para escribir IR: " << ec.message() << '\n';
    return ;

  }

  llvm_modulo->print(archivo, nullptr);

  // Imprimir por stdout
  // llvm_modulo->print(llvm::errs(), nullptr);

}

//... Everything after this line has to be triple checked.

// --- Expresiones --- //
void Emitter::visitar(ExprLiteral* nodo) { //...
  auto tipo = nodo->tipo_resuelto.valor;
  int bits = tipo->getBitSize();

  switch (tipo->kind) {
    case TypeKind::INTEGER: {
      auto& data = std::get<NumberData>(nodo->datos);
      llvm_valor = llvm::ConstantInt::get(llvm_ctx, llvm::APInt(bits, data.valor, 10));
      break;
    }

    case TypeKind::FLOAT: {
      auto& data = std::get<NumberData>(nodo->datos);
      const llvm::fltSemantics* sem;

      if      (bits == 16 ) { sem = &llvm::APFloat::IEEEhalf         (); }
      else if (bits == 32 ) { sem = &llvm::APFloat::IEEEsingle       (); }
      else if (bits == 64 ) { sem = &llvm::APFloat::IEEEdouble       (); }
      else if (bits == 80 ) { sem = &llvm::APFloat::x87DoubleExtended(); } //... Should be
      else if (bits == 128) { sem = &llvm::APFloat::IEEEquad         (); } // else if (bits == llvm::APFloat::semanticsSizeInBits(llvm::APFloat::IEEEquad()))
      else                  { sem = &llvm::APFloat::IEEEdouble       (); }

      llvm_valor = llvm::ConstantFP::get(llvm_ctx, llvm::APFloat(*sem, data.valor));
      break;
    }

    case TypeKind::CHAR: {
      auto& data = std::get<CharData>(nodo->datos);

      llvm::APInt valor_char(bits, 0, false);

      for (char c : data.letra) {
        valor_char <<= 8;
        llvm::APInt byte_actual(bits, static_cast<unsigned char>(c), false);
        valor_char |= byte_actual;
      }

      llvm_valor = llvm::ConstantInt::get(llvm_ctx, valor_char);
      break;
    }

    default: {
      break;
    }
  }

}

void Emitter::visitar(ExprVariable* nodo) {
  //std::cout << "[82, emitter.cpp] ExprVariable\n";
  //std::cout << nodo->nombre << '\n';

  InfoVariable* info = tablas.buscarVariable(nodo->nombre);

  if (info && info->alloca) {
    llvm_valor = llvm_builder->CreateLoad(info->alloca->getAllocatedType(), info->alloca, "");
    return ;

  }

  if (bloques_arcano_activos.count(nodo->nombre)) {
    bloques_arcano_activos[nodo->nombre]->accept(this);
    return ;
  }

  std::cerr << "Error: Variable '" << nodo->nombre << "' no encontrada.\n";

}

void Emitter::visitar(ExprArray* nodo) {

}

void Emitter::visitar(ExprUnaria* nodo) {
  //std::cout << "[112, emitter.cpp] ExprUnaria\n";

  nodo->operando->accept(this);
  llvm::Value* val = llvm_valor;

  if (!val) { return ; }

  switch (nodo->operador) { //...

    case TipoOperador::BITWISE_NO:
    case TipoOperador::LOGICO_NO: {
      llvm_valor = llvm_builder->CreateNot(llvm_valor, "");
      break;
    }

    case TipoOperador::PTR_DEREF: {
      llvm::Value* ptr_val = val;

      auto tipo_base = nodo->operando->tipo_resuelto.valor->getUnderlyingType();
      llvm::Type* tipo_llvm = obtenerTipoLLVM(tipo_base);

      llvm_valor = llvm_builder->CreateLoad(tipo_llvm, ptr_val, "");
      break;

    }

    case TipoOperador::PTR_REF: {

      if (auto* var = dynamic_cast<ExprVariable*>(nodo->operando.get())) {
        InfoVariable* info = tablas.buscarVariable(var->nombre);
        if (info && info->alloca) {
          llvm_valor = info->alloca;
          return ;
        }
      }

      if (auto* unaria = dynamic_cast<ExprUnaria*>(nodo->operando.get())) {
        if (unaria->operador == TipoOperador::PTR_DEREF) {
          unaria->operando->accept(this);
          return ;
        }
      }

      std::cerr << "Error: Solo se puede tomar la dirección de L-Values\n";
      exit(1);

    }

    default: {
      std::cout << "[138, emitter.cpp] Error: Operador unario no implementado.";
      exit(1);
    }

  }

}

void Emitter::visitar(ExprBinaria* nodo) {
  //std::cout << "[146, emitter.cpp] ExprBinaria\n";
  nodo->izquierda->accept(this);
  llvm::Value* left = llvm_valor;

  nodo->derecha->accept(this);
  llvm::Value* right = llvm_valor;

  bool es_float = nodo->tipo_resuelto.valor->kind == TypeKind::FLOAT;

  switch (nodo->operador) { //...
    case TipoOperador::A_SUMA: {
      std::cout << "[183, emitter.cpp]\n";
      llvm_valor = es_float ? llvm_builder->CreateFAdd(left, right, "")
                            : llvm_builder->CreateAdd(left, right, "");
      break;
    }

    case TipoOperador::A_RESTA: {
      std::cout << "[189, emitter.cpp]\n";
      llvm_valor = es_float ? llvm_builder->CreateFSub(left, right, "")
                            : llvm_builder->CreateSub(left, right, "");
      break;
    }

    case TipoOperador::A_MULT: {
      std::cout << "[196, emitter.cpp]\n";
      llvm_valor = es_float ? llvm_builder->CreateFMul(left, right, "")
                            : llvm_builder->CreateMul(left, right, "");
      break;
    }

    case TipoOperador::CMP_MENOR: { //... Add signed / unsigned cmp support
      std::cout << "[203, emitter.cpp]\n";
      llvm_valor = es_float ? llvm_builder->CreateFCmpULT(left, right, "")
                            : llvm_builder->CreateICmpULT(left, right, "");
      break;
    }

    case TipoOperador::A_SWAP: {
      std::cout << "[211, emitter.cpp]\n";

      llvm::Value* ptr_l = nullptr;
      llvm::Value* ptr_r = nullptr;

      if (auto* var_izq = dynamic_cast<ExprVariable*>(nodo->izquierda.get())) {
        InfoVariable* info = tablas.buscarVariable(var_izq->nombre);
        if (info) { ptr_l = info->alloca; }

      } else if (auto* unaria = dynamic_cast<ExprUnaria*>(nodo->izquierda.get())) {
        if (unaria->operador == TipoOperador::PTR_DEREF) {
          unaria->operando->accept(this);
          ptr_l = llvm_valor;
        }
      }

      if (auto* var_der = dynamic_cast<ExprVariable*>(nodo->derecha.get())) {
        InfoVariable* info = tablas.buscarVariable(var_der->nombre);
        if (info) { ptr_r = info->alloca; }

      } else if (auto* unaria = dynamic_cast<ExprUnaria*>(nodo->derecha.get())) {
        if (unaria->operador == TipoOperador::PTR_DEREF) {
          unaria->operando->accept(this);
          ptr_r = llvm_valor;
        }
      }

      if (ptr_l && ptr_r) {
        llvm_builder->CreateStore(right, ptr_l);
        llvm_builder->CreateStore(left, ptr_r);
        llvm_valor = right;

      }

      break;
    }

    default: {
      std::cout << "[184, emitter.cpp]\n";
      break;
    }
  }

}

void Emitter::visitar(ExprCasteo* nodo) {
  //std::cout << "[192, emitter.cpp] ExprCasteo\n";
  nodo->expresion->accept(this);
  llvm::Value* val = llvm_valor;

  std::shared_ptr<ArcanaType> t_origen  = nodo->expresion->tipo_resuelto.valor;
  std::shared_ptr<ArcanaType> t_destino = nodo->tipo_resuelto.valor;

  if (t_destino->kind == TypeKind::BOOLEAN) {
    switch (t_origen->kind) {

      case TypeKind::CHAR   :
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
                                                 ""
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

  llvm_valor = llvm_builder->CreateCast(cast_op, val, tipo_destino_llvm, "");

}

void Emitter::visitar(ExprRango* nodo) {

}

void Emitter::visitar(ExprAcceso* nodo) {

}

void Emitter::visitar(ExprFuncCall* nodo) {
  //std::cout << "[247, emitter.cpp] ExprFuncCall\n";
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

  llvm_valor = llvm_builder->CreateCall(callee_f, args_v, "");

}

// --- Sentencias --- //

void Emitter::visitar(Bloque* nodo) {
  //std::cout << "[276, emitter.cpp] Bloque\n";

  tablas.entrarScope();

  for (const auto& i : nodo->instrucciones) {
    i->accept(this);

  }

  tablas.salirScope();

}

void Emitter::visitar(SentenciaAsignarVar* nodo) {
  //std::cout << "[289, emitter.cpp] SentenciaAsignarVar\n";

  llvm::Type* tipo_llvm = obtenerTipoLLVM(nodo->tipo_explicito.tipo.valor);

  llvm::AllocaInst* alloca = llvm_builder->CreateAlloca(tipo_llvm, nullptr, nodo->nombre);

  InfoVariable* info = tablas.buscarVariable(nodo->nombre);

  if (info) {
    info->alloca = alloca;
  }

  if (nodo->valor_inicial) {
    nodo->valor_inicial->accept(this);

    llvm_builder->CreateStore(llvm_valor, alloca);

  }

}

void Emitter::visitar(SentenciaExpr* nodo) {
  //std::cout << "[308, emitter.cpp] SentenciaExpr\n";
  if (nodo->expresion) {
    nodo->expresion->accept(this);
  }

}

void Emitter::visitar(SentenciaReasignacionVar* nodo) {
  llvm::Value* destino_ptr = nullptr;

  if (auto* var_izq = dynamic_cast<ExprVariable*>(nodo->izquierda.get())) {
    InfoVariable* info = tablas.buscarVariable(var_izq->nombre);
    if (info) { destino_ptr = info->alloca; }

  } else if (auto* unaria = dynamic_cast<ExprUnaria*>(nodo->izquierda.get())) {
    if (unaria->operador == TipoOperador::PTR_DEREF) {
      unaria->operando->accept(this);
      destino_ptr = llvm_valor;
    }
  }

  if (destino_ptr) {
    nodo->derecha->accept(this);
    llvm_builder->CreateStore(llvm_valor, destino_ptr);
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

  pila_breaks.push_back(merge_bb);
  pila_continues.push_back(cond_bb);

  funcion_actual->insert(funcion_actual->end(), loop_bb);
  llvm_builder->SetInsertPoint(loop_bb);

  nodo->rama_while->accept(this);

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
  //std::cout << "[486, emitter.cpp] SentenciaReturn\n";
  nodo->ret_value->accept(this);
  llvm_builder->CreateRet(llvm_valor);

}

void Emitter::visitar(SentenciaFuncDecl* nodo) {
  //std::cout << "[473, emitter.cpp] SentenciaFuncDecl\n";
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

  if (nodo->cuerpo_func.empty()) {
    return ;
  }

  llvm::BasicBlock* bb = llvm::BasicBlock::Create(llvm_ctx, "entry", f);
  llvm_builder->SetInsertPoint(bb);

  //llvm_scopes.push_back(std::map<std::string, llvm::AllocaInst*>());

  tablas.entrarScope();

  auto it_args_name = nodo->args_type.begin();
  for (auto &arg : f->args()) {
    const std::string& nombre_arg = it_args_name->first;

    llvm::AllocaInst* alloca = llvm_builder->CreateAlloca(arg.getType(), nullptr, "");
    llvm_builder->CreateStore(&arg, alloca);

    InfoVariable* info = tablas.buscarVariable(nombre_arg);
    if (info) { info->alloca = alloca; }

    it_args_name++;

  }

  if (!nodo->cuerpo_func.empty()) {
    for (const auto& inst : nodo->cuerpo_func) {
      inst->accept(this);
    }
  }

  //llvm_scopes.pop_back();

  llvm::verifyFunction(*f);

  tablas.salirScope();

}

void Emitter::visitar(SentenciaEscritura* nodo) {

}

void Emitter::visitar(SentenciaArcano* nodo) {
  //std::cout << "[529, emitter.cpp] SentenciaArcano\n";

}

void Emitter::visitar(SentenciaLlamadaArcano* nodo) { //...
  //std::cout << "[534, emitter.cpp] SentenciaLlamadaArcano\n";
  ArcaneDef& def = contextoArcanos.buscarDefinicionPorKeyword(nodo->nombre);

  if (nodo->indice_rama >= def.branches.size()) {
    std::cerr << "Error interno: Índice de rama fuera de rango para '" << nodo->nombre << "'.\n";
    exit(1);

  }

  ArcaneBranch* rama_elegida = &def.branches[nodo->indice_rama];

  tablas.entrarScope();

  auto backup_bloques = bloques_arcano_activos;

  for (const auto& [nombre_arg, ast_arg] : nodo->args) {
    ast_arg->accept(this);
    llvm::Value* valor_arg = llvm_valor;

    llvm::AllocaInst* alloca = llvm_builder->CreateAlloca(valor_arg->getType(), nullptr, nombre_arg);
    llvm_builder->CreateStore(valor_arg, alloca);

    InfoVariable* info = tablas.buscarVariable(nombre_arg);

    if (info) {
      info->alloca = alloca;

    } else {
      std::cerr << "Error: Argumento '" << nombre_arg << "' no encontrado.\n";

    }

  }

  for (const auto& [nombre_arg, ast_arg] : nodo->expr) {
    bloques_arcano_activos[nombre_arg] = ast_arg.get();
  }

  for (const auto& [nombre_arg, ast_arg] : nodo->code) {
    bloques_arcano_activos[nombre_arg] = ast_arg.get();
  }

  for (const auto& seg: rama_elegida->segmentos) {
    if (seg.br_cont) {
      seg.br_cont->accept(this);
    }
  }

  bloques_arcano_activos = backup_bloques;
  tablas.salirScope();

}
