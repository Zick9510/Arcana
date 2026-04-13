// emitter.cpp

#include "Common.hpp"
#include "Emitter.hpp"

Emitter::Emitter() {
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

void Emitter::generarArchivoIR(const std::string& nombreArchivo) {
  std::error_code ec;

  llvm::raw_fd_ostream archivo(nombreArchivo, ec, llvm::sys::fs::OF_None);

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
  llvm::AllocaInst* alloca = nullptr;
  for (auto it = llvm_scopes.rbegin(); it != llvm_scopes.rend(); ++it) {
    if (it->count(nodo->nombre)) {
      alloca = (*it)[nodo->nombre];
      break;
    }
  }

  llvm::Type* tipoLLVM = obtenerTipoLLVM(nodo->tipo_resuelto.valor);

  llvm_valor = llvm_builder->CreateLoad(tipoLLVM, alloca, nodo->nombre + "_val");

}

void Emitter::visitar(ExprArray* nodo) {

}

void Emitter::visitar(ExprUnaria* nodo) {

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

  if (!callee_f) {
    // El Checker debería haber evitado que llegemos a este punto
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

void Emitter::visitar(SentenciaSi* nodo) {

}

void Emitter::visitar(SentenciaSino* nodo) {

}

void Emitter::visitar(SentenciaMientras* nodo) {

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

  llvm::Type* tipo_ret = obtenerTipoLLVM(nodo->ret_type.tipo.valor);

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

void Emitter::visitar(SentenciaLlamadaArcano* nodo) {

}
