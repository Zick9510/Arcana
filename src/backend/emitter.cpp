// emitter.cpp

#include "Common.hpp"
#include "Emitter.hpp"


/* --- Trait Emitter Handler --- */
TraitEmitter::TraitEmitter(Emitter& e)
  : emitter(e) {}

void TraitEmitter::despacharTrait(Bloque* nodo, size_t idx) {
  if (idx >= nodo->traits.size()) {

    for (const auto& inst : nodo->instrucciones) {
      if (emitter.llvmBuilder->GetInsertBlock()->getTerminator()) { break; }
      inst->accept(&emitter);
    }

    return ;

  }

  auto trait = nodo->traits[idx];

  switch (trait) {
    case BT::LOOP: {
      handleLoop(nodo, idx);
      break;
    }

    default: {
      despacharTrait(nodo, idx + 1);
      break;
    }

  }
}

void TraitEmitter::handleLoop(Bloque* nodo, size_t idx) {
  llvm::Function* func = emitter.llvmBuilder->GetInsertBlock()->getParent();

  llvm::BasicBlock* redo_bb = llvm::BasicBlock::Create(emitter.llvmCtx, "trait.loop.redo", func);
  llvm::BasicBlock* end_bb  = llvm::BasicBlock::Create(emitter.llvmCtx, "trait.loop.end");

  emitter.llvmBuilder->CreateBr(redo_bb);
  emitter.llvmBuilder->SetInsertPoint(redo_bb);

  emitter.pilaBreaks.push_back(end_bb );
  emitter.pilaRedos .push_back(redo_bb);

  despacharTrait(nodo, idx + 1);

  if (!emitter.llvmBuilder->GetInsertBlock()->getTerminator()) {
    emitter.llvmBuilder->CreateBr(redo_bb);
  }

  emitter.pilaRedos .pop_back();
  emitter.pilaBreaks.pop_back();

  func->insert(func->end(), end_bb);
  emitter.llvmBuilder->SetInsertPoint(end_bb);

}

/* --- Emitter --- */
Emitter::Emitter(ContextoArcanos& ca, GestorTablas& t)
  : contextoArcanos(ca), tablas(t), traits(*this) {
  llvmModulo  = std::make_unique<llvm::Module>("ArcanaModulo", llvmCtx);
  llvmBuilder = std::make_unique<llvm::IRBuilder<>>(llvmCtx);

  tablas.prepareForEmitter();

}

// --- LLVM --- //
llvm::Type* Emitter::obtenerTipoLLVM(std::shared_ptr<ArcanaType> tipo) {
  if (!tipo) { return nullptr; }

  switch (tipo->kind) {

    case TypeKind::VOID: {
      return llvm::Type::getVoidTy(llvmCtx);
    }

    case TypeKind::BOOLEAN: {
      return llvm::Type::getInt1Ty(llvmCtx);
    }

    case TypeKind::CHAR   :
    case TypeKind::INTEGER: {
      return llvm::Type::getIntNTy(llvmCtx, tipo->getBitSize());
    }

    case TypeKind::FLOAT: {
      switch(tipo->getBitSize()) {
        case 16 : { return llvm::Type::getHalfTy    (llvmCtx); }
        case 32 : { return llvm::Type::getFloatTy   (llvmCtx); }
        case 64 : { return llvm::Type::getDoubleTy  (llvmCtx); }
        case 80 : { return llvm::Type::getX86_FP80Ty(llvmCtx); }
        case 128: { return llvm::Type::getFP128Ty   (llvmCtx); }
        default : { return nullptr; }
      }
    }

    case TypeKind::POINTER: {
      return llvm::PointerType::getUnqual(llvmCtx);
    }

    case TypeKind::STRUCT: {
      return llvmStructs[tipo->toString()];
    }

    default: {
      return nullptr;
    }

  }

}

llvm::Value* Emitter::obtenerPuntero(Expresion* nodo) {

  if (auto* var = dynamic_cast<ExprVariable*>(nodo)) {
    InfoVariable* info = tablas.buscarVariable(var->nombre);
    return info ? info->alloca : nullptr;
  }

  if (auto* unaria = dynamic_cast<ExprUnaria*>(nodo)) {
    if (unaria->operador == TipoOperador::PTR_DEREF) {
      unaria->operando->accept(this);
      return llvmValor;
    }
  }

  if (auto* acceso = dynamic_cast<ExprAccesoPunto*>(nodo)) {
    llvm::Value* ptr_izq = obtenerPuntero(acceso->izquierda.get());
    if (!ptr_izq) { return nullptr; }

    auto tipo_izq = acceso->izquierda->tipo_resuelto.valor;
    auto struct_type = std::static_pointer_cast<StructType>(tipo_izq);
    const auto& orden_props = struct_type->info->orden_props;

    unsigned idx = 0;
    for (size_t i = 0; i < orden_props.size(); ++i) {
      if (orden_props[i] == acceso->propiedad) {
        idx = i;
        break;
      }
    }

    llvm::Type* tipo_struct_llvm = obtenerTipoLLVM(struct_type);
    return llvmBuilder->CreateStructGEP(tipo_struct_llvm, ptr_izq, idx, "");

  }

  return nullptr;

}

llvm::CmpInst::Predicate Emitter::obtenerPredicadoCmp(TipoOperador op, bool esFloat, bool esSigned) {

  if (esFloat) {
    switch (op) {

      case TipoOperador::CMP_IGUAL      : { return llvm::FCmpInst::FCMP_OEQ          ; }
      case TipoOperador::CMP_DISTINTO   : { return llvm::FCmpInst::FCMP_ONE          ; }
      case TipoOperador::CMP_MAYOR      : { return llvm::ICmpInst::FCMP_OGT          ; }
      case TipoOperador::CMP_MAYOR_IGUAL: { return llvm::ICmpInst::FCMP_OGE          ; }
      case TipoOperador::CMP_MENOR_IGUAL: { return llvm::ICmpInst::FCMP_OLE          ; }
      case TipoOperador::CMP_MENOR      : { return llvm::ICmpInst::FCMP_OLT          ; }
      default                           : { return llvm::ICmpInst::BAD_FCMP_PREDICATE; }

    }

  }

  switch (op) {
    case TipoOperador::CMP_IGUAL   : { return llvm::ICmpInst::ICMP_EQ; }
    case TipoOperador::CMP_DISTINTO: { return llvm::ICmpInst::ICMP_NE; }

    case TipoOperador::CMP_MAYOR: {
      return esSigned ? llvm::ICmpInst::ICMP_SGT : llvm::ICmpInst::ICMP_UGT;
    }

    case TipoOperador::CMP_MAYOR_IGUAL: {
      return esSigned ? llvm::ICmpInst::ICMP_SGE : llvm::ICmpInst::ICMP_UGE;
    }

    case TipoOperador::CMP_MENOR_IGUAL: {
      return esSigned ? llvm::ICmpInst::ICMP_SLE : llvm::ICmpInst::ICMP_ULE;
    }

    case TipoOperador::CMP_MENOR: {
      return esSigned ? llvm::ICmpInst::ICMP_SLT : llvm::ICmpInst::ICMP_ULT;
    }

    default: { return llvm::ICmpInst::BAD_ICMP_PREDICATE; }

  }

}

void Emitter::generarArchivoIR(const std::filesystem::path& nombreArchivo) {
  std::error_code ec;

  llvm::raw_fd_ostream archivo(std::string(nombreArchivo), ec, llvm::sys::fs::OF_None);

  if (ec) {
    std::cerr << "Error al abrir el archivo para escribir IR: " << ec.message() << '\n';
    return ;

  }

  llvmModulo->print(archivo, nullptr);

  // Imprimir por stdout
  // llvm_modulo->print(llvm::errs(), nullptr);

}

// --- Expresiones --- //

void Emitter::visitar(ErrorNode* nodo) {}

void Emitter::visitar(ExprLiteral* nodo) { //...
  auto tipo = nodo->tipo_resuelto.valor;
  int bits = tipo->getBitSize();

  switch (tipo->kind) {
    case TypeKind::INTEGER: {
      auto& data = std::get<NumberData>(nodo->datos);
      llvmValor = llvm::ConstantInt::get(llvmCtx, llvm::APInt(bits, data.valor, 10));
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

      llvmValor = llvm::ConstantFP::get(llvmCtx, llvm::APFloat(*sem, data.valor));
      break;
    }

    case TypeKind::BOOLEAN: {
      auto& data = std::get<BooleanData>(nodo->datos);
      llvmValor = data.valor == "true" ? llvm::ConstantInt::getTrue(llvmCtx) : llvm::ConstantInt::getFalse(llvmCtx);
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

      llvmValor = llvm::ConstantInt::get(llvmCtx, valor_char);
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
    llvmValor = llvmBuilder->CreateLoad(info->alloca->getAllocatedType(), info->alloca, "");
    return ;

  }

  if (bloquesArcanoActivos.count(nodo->nombre)) {
    bloquesArcanoActivos[nodo->nombre]->accept(this);
    return ;
  }

  if (llvmThis != nullptr && !structActual.empty()) {
    InfoStruct* info_struct = tablas.buscarStruct(structActual);
    if (info_struct) {
      auto it = std::find(info_struct->orden_props.begin(), info_struct->orden_props.end(), nodo->nombre);

      if (it != info_struct->orden_props.end()) {
        int i = std::distance(info_struct->orden_props.begin(), it);
        llvm::Value* ptr_campo = llvmBuilder->CreateStructGEP(llvmStructActual, llvmThis, i, "");

        llvm::Type* tipo_campo = obtenerTipoLLVM(info_struct->propiedades[nodo->nombre].tipo.valor);
        llvmValor = llvmBuilder->CreateLoad(tipo_campo, ptr_campo, nodo->nombre);
        return ;
      }
    }
  }

  std::cerr << "Error: Variable '" << nodo->nombre << "' no encontrada.\n";

}

void Emitter::visitar(ExprArray* nodo) {

}

void Emitter::visitar(ExprUnaria* nodo) {
  //std::cout << "[112, emitter.cpp] ExprUnaria\n";

  nodo->operando->accept(this);
  llvm::Value* val = llvmValor;

  if (!val) { return ; }

  switch (nodo->operador) { //...

    case TipoOperador::RESTA: {
      std::cout << "[269, emitter.cpp] Resta\n";
      llvmValor = nodo->tipo_resuelto.valor->kind == TypeKind::FLOAT ? llvmBuilder->CreateFNeg(llvmValor, "")
                                                                     : llvmBuilder->CreateNeg (llvmValor, "");
      break;
    }

    case TipoOperador::BITWISE_NO:
    case TipoOperador::LOGICO_NO: {
      llvmValor = llvmBuilder->CreateNot(llvmValor, "");
      break;
    }

    case TipoOperador::PTR_DEREF: {
      llvm::Value* ptr_val = val;

      auto tipo_base = nodo->operando->tipo_resuelto.valor->getUnderlyingType();
      llvm::Type* tipo_llvm = obtenerTipoLLVM(tipo_base);

      llvmValor = llvmBuilder->CreateLoad(tipo_llvm, ptr_val, "");
      break;

    }

    case TipoOperador::PTR_REF: {

      if (auto* var = dynamic_cast<ExprVariable*>(nodo->operando.get())) {
        InfoVariable* info = tablas.buscarVariable(var->nombre);
        if (info && info->alloca) {
          llvmValor = info->alloca;
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
      std::cout << "[308, emitter.cpp] Error: Operador unario no implementado.";
      exit(1);
    }

  }

}

void Emitter::visitar(ExprBinaria* nodo) {
  //std::cout << "[317, emitter.cpp] ExprBinaria\n";

  std::cout << "[386, emitter.cpp] nodo->overload: '" << nodo->overload << "'\n";

  if (!nodo->overload.empty()) {
    llvm::Function* func = llvmModulo->getFunction(nodo->overload);
    if (!func) {
      std::cerr << "Error: No se encontró el método '" << nodo->overload << "'\n";
      return ;
    }

    llvm::Value* ptr_this = nullptr;

    if (auto* var_izq = dynamic_cast<ExprVariable*>(nodo->izquierda.get())) {
      InfoVariable* info = tablas.buscarVariable(var_izq->nombre);
      if (info) { ptr_this = info->alloca; }
    }

    if (!ptr_this) {
      std::cerr << "Error: No se pudo obtener la dirección de memoria para 'this'\n";
      return ;
    }

    nodo->derecha->accept(this);
    llvm::Value* arg_derecha = llvmValor;

    std::vector<llvm::Value*> args_call = { ptr_this, arg_derecha };
    llvmValor = llvmBuilder->CreateCall(func, args_call, "");
    return ;

  }

  nodo->izquierda->accept(this);
  llvm::Value* left = llvmValor;

  nodo->derecha->accept(this);
  llvm::Value* right = llvmValor;

  bool es_float = nodo->tipo_resuelto.valor->kind == TypeKind::FLOAT;

  switch (nodo->operador) { //...
    case TipoOperador::SUMA: {
      std::cout << "[408, emitter.cpp]\n";
      llvmValor = es_float ? llvmBuilder->CreateFAdd(left, right, "")
                           : llvmBuilder->CreateAdd(left, right, "");
      break;
    }

    case TipoOperador::RESTA: {
      std::cout << "[415, emitter.cpp]\n";
      llvmValor = es_float ? llvmBuilder->CreateFSub(left, right, "")
                           : llvmBuilder->CreateSub(left, right, "");
      break;
    }

    case TipoOperador::MULT: {
      std::cout << "[422, emitter.cpp]\n";
      llvmValor = es_float ? llvmBuilder->CreateFMul(left, right, "")
                           : llvmBuilder->CreateMul(left, right, "");
      break;
    }

    case TipoOperador::CMP_MAYOR:
    case TipoOperador::CMP_MAYOR_IGUAL:
    case TipoOperador::CMP_IGUAL:
    case TipoOperador::CMP_DISTINTO:
    case TipoOperador::CMP_MENOR_IGUAL:
    case TipoOperador::CMP_MENOR: {
      std::cout << "[434, emitter.cpp]\n";
      llvm::CmpInst::Predicate pred = obtenerPredicadoCmp(nodo->operador, es_float, nodo->derecha->tipo_resuelto.valor->isSigned());
      llvmValor = llvmBuilder->CreateCmp(pred, left, right);
      break;
    }

    case TipoOperador::SWAP: {
      std::cout << "[441, emitter.cpp]\n";

      llvm::Value* ptr_l = nullptr;
      llvm::Value* ptr_r = nullptr;

      if (auto* var_izq = dynamic_cast<ExprVariable*>(nodo->izquierda.get())) {
        InfoVariable* info = tablas.buscarVariable(var_izq->nombre);
        if (info) { ptr_l = info->alloca; }

      } else if (auto* unaria = dynamic_cast<ExprUnaria*>(nodo->izquierda.get())) {
        if (unaria->operador == TipoOperador::PTR_DEREF) {
          unaria->operando->accept(this);
          ptr_l = llvmValor;
        }
      }

      if (auto* var_der = dynamic_cast<ExprVariable*>(nodo->derecha.get())) {
        InfoVariable* info = tablas.buscarVariable(var_der->nombre);
        if (info) { ptr_r = info->alloca; }

      } else if (auto* unaria = dynamic_cast<ExprUnaria*>(nodo->derecha.get())) {
        if (unaria->operador == TipoOperador::PTR_DEREF) {
          unaria->operando->accept(this);
          ptr_r = llvmValor;
        }
      }

      if (ptr_l && ptr_r) {
        llvmBuilder->CreateStore(right, ptr_l);
        llvmBuilder->CreateStore(left, ptr_r);
        llvmValor = right;

      }

      break;
    }

    default: {
      std::cout << "[479, emitter.cpp]\n";
      break;
    }
  }

}

void Emitter::visitar(ExprTernaria* nodo) {
  nodo->condicion->accept(this);
  llvm::Value* cond = llvmValor;

  llvm::Value* cero_cond = llvm::ConstantInt::get(cond->getType(), 0);
  cond = llvmBuilder->CreateICmpNE(cond, cero_cond, "");

  llvm::Function* function = llvmBuilder->GetInsertBlock()->getParent();

  llvm::BasicBlock* then_bb  = llvm::BasicBlock::Create(llvmCtx, "ternary.then", function);
  llvm::BasicBlock* else_bb  = llvm::BasicBlock::Create(llvmCtx, "ternary.else");
  llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(llvmCtx, "ternary.merge");

  llvmBuilder->CreateCondBr(cond, then_bb, else_bb);

  llvmBuilder->SetInsertPoint(then_bb);
  nodo->rama_true->accept(this);
  llvm::Value* val_true = llvmValor;
  then_bb = llvmBuilder->GetInsertBlock();
  llvmBuilder->CreateBr(merge_bb);

  function->insert(function->end(), else_bb);
  llvmBuilder->SetInsertPoint(else_bb);
  nodo->rama_false->accept(this);
  llvm::Value* val_false = llvmValor;
  else_bb = llvmBuilder->GetInsertBlock();
  llvmBuilder->CreateBr(merge_bb);

  function->insert(function->end(), merge_bb);
  llvmBuilder->SetInsertPoint(merge_bb);

  llvm::Type* tipo_res = obtenerTipoLLVM(nodo->tipo_resuelto.valor);
  llvm::PHINode* phi = llvmBuilder->CreatePHI(tipo_res, 2, "ternary.res");

  phi->addIncoming(val_true , then_bb);
  phi->addIncoming(val_false, else_bb);
  llvmValor = phi;

}

void Emitter::visitar(ExprCasteo* nodo) {
  //std::cout << "[192, emitter.cpp] ExprCasteo\n";
  nodo->expresion->accept(this);
  llvm::Value* val = llvmValor;

  std::shared_ptr<ArcanaType> t_origen  = nodo->expresion->tipo_resuelto.valor;
  std::shared_ptr<ArcanaType> t_destino = nodo->tipo_resuelto.valor;

  if (t_destino->kind == TypeKind::BOOLEAN) {
    switch (t_origen->kind) {

      case TypeKind::CHAR   :
      case TypeKind::INTEGER: {
        llvmValor = llvmBuilder->CreateICmpNE(
          val,
          llvm::ConstantInt::get(val->getType(), 0),
          ""
        );
        break;
      }

      case TypeKind::FLOAT: {
        llvmValor = llvmBuilder->CreateFCmpUNE(val,
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

  llvmValor = llvmBuilder->CreateCast(cast_op, val, tipo_destino_llvm, "");

}

void Emitter::visitar(ExprRango* nodo) {

}

void Emitter::visitar(ExprAcceso* nodo) {

}

void Emitter::visitar(ExprAccesoPunto* nodo) {
  nodo->izquierda->accept(this);
  llvm::Value* struct_agregado = llvmValor;

  if (!struct_agregado) { return ; }

  auto tipo_izq = nodo->izquierda->tipo_resuelto.valor;
  auto struct_type = std::static_pointer_cast<StructType>(tipo_izq);
  const auto& orden_props = struct_type->info->orden_props;

  unsigned idx = 0;
  bool es_prop = false;
  for (size_t i = 0; i < orden_props.size(); ++i) {
    if (orden_props[i] == nodo->propiedad) {
      idx = i;
      es_prop = true;
      break;
    }
  }

  if (es_prop) {
    llvmValor = llvmBuilder->CreateExtractValue(struct_agregado, idx, "");

  } else { //...

  }

}

void Emitter::visitar(ExprFuncCall* nodo) {
  //std::cout << "[247, emitter.cpp] ExprFuncCall\n";
  auto* var_callee = dynamic_cast<ExprVariable*>(nodo->callee.get());
  if (!var_callee) {
    //...
    return ;
  }

  llvm::Function* callee_f = llvmModulo->getFunction(var_callee->nombre);

  if (!callee_f) { // Trust me, there is no way the code ends up here.
    std::cerr << "Error: Función " << var_callee->nombre << "no encontrada.\n";
    return ;

  }

  std::vector<llvm::Value*> args_v;
  for (auto& arg : nodo->argumentos) {
    arg.second->accept(this);
    args_v.push_back(llvmValor);

  }

  llvmValor = llvmBuilder->CreateCall(callee_f, args_v, "");

}

void Emitter::visitar(ExprInitList* nodo) {
  auto struct_type = std::static_pointer_cast<StructType>(nodo->tipo_resuelto.valor);
  llvm::Type* struct_llvm_type = obtenerTipoLLVM(struct_type);

  llvm::Value* struct_val = llvm::UndefValue::get(struct_llvm_type);

  const auto& orden_props = struct_type->info->orden_props;

  for (const auto& arg : nodo->args) {

    arg.value->accept(this);
    llvm::Value* arg_val = llvmValor;

    unsigned idx = 0;
    for (size_t i = 0; i < orden_props.size(); ++i) {
      if (orden_props[i] == arg.name.value()) {
        idx = i;
        break;

      }
    }

    struct_val = llvmBuilder->CreateInsertValue(struct_val, arg_val, idx);

  }

  llvmValor = struct_val;

}

// --- Sentencias --- //

void Emitter::visitar(Bloque* nodo) {
  //std::cout << "[276, emitter.cpp] Bloque\n";

  tablas.entrarScope();

  traits.despacharTrait(nodo, 0);

  tablas.salirScope();

}

void Emitter::visitar(SentenciaAsignarVar* nodo) {
  //std::cout << "[289, emitter.cpp] SentenciaAsignarVar\n";

  llvm::Type* tipo_llvm = obtenerTipoLLVM(nodo->tipo_explicito.tipo.valor);
  llvm::AllocaInst* alloca = llvmBuilder->CreateAlloca(tipo_llvm, nullptr, nodo->nombre);

  InfoVariable* info = tablas.buscarVariable(nodo->nombre);

  if (nodo->valor_inicial) {
    nodo->valor_inicial->accept(this);
    llvmBuilder->CreateStore(llvmValor, alloca);

  }

  if (info) {
    info->alloca = alloca;
  }


}

void Emitter::visitar(SentenciaExpr* nodo) {
  //std::cout << "[308, emitter.cpp] SentenciaExpr\n";
  if (nodo->expresion) {
    nodo->expresion->accept(this);
  }

}

void Emitter::visitar(SentenciaReasignacionVar* nodo) {
  llvm::Value* destino_ptr = obtenerPuntero(nodo->izquierda.get());

  nodo->derecha->accept(this);
  llvm::Value* valor_asignar = llvmValor;

  llvmBuilder->CreateStore(valor_asignar, destino_ptr);

}

void Emitter::visitar(SentenciaSi* nodo) { //...
  nodo->condicion->accept(this);
  llvm::Value* cond_v = llvmValor;

  llvm::Value* cero =
    llvm::ConstantInt::get(llvmCtx,
                           llvm::APInt(cond_v->getType()->getIntegerBitWidth(),0)
                           );

  cond_v = llvmBuilder->CreateICmpNE(cond_v, cero, "ifcond");

  llvm::Function* funcion_actual = llvmBuilder->GetInsertBlock()->getParent();

  llvm::BasicBlock* then_bb  = llvm::BasicBlock::Create(llvmCtx, "then", funcion_actual);
  llvm::BasicBlock* else_bb  = llvm::BasicBlock::Create(llvmCtx, "else");
  llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(llvmCtx, "ifcont");

  llvmBuilder->CreateCondBr(cond_v, then_bb, else_bb);

  llvmBuilder->SetInsertPoint(then_bb);
  nodo->rama_si->accept(this);

  if (!llvmBuilder->GetInsertBlock()->getTerminator()) {
    llvmBuilder->CreateBr(merge_bb);
  }

  funcion_actual->insert(funcion_actual->end(), else_bb);
  llvmBuilder->SetInsertPoint(else_bb);

  if (nodo->rama_sino) {
    nodo->rama_sino->accept(this);
  }

  if (!llvmBuilder->GetInsertBlock()->getTerminator()) {
    llvmBuilder->CreateBr(merge_bb);
  }

  if (merge_bb->use_empty()) {
    delete merge_bb;

  } else {
    funcion_actual->insert(funcion_actual->end(), merge_bb);
    llvmBuilder->SetInsertPoint(merge_bb);

  }
}

void Emitter::visitar(SentenciaSino* nodo) {
  if (nodo->cuerpo) {
    nodo->cuerpo->accept(this);
  }

}

void Emitter::visitar(SentenciaMientras* nodo) {
  llvm::Function* funcion_actual = llvmBuilder->GetInsertBlock()->getParent();

  llvm::BasicBlock* cond_bb  = llvm::BasicBlock::Create(llvmCtx, "while.cond", funcion_actual);
  llvm::BasicBlock* loop_bb  = llvm::BasicBlock::Create(llvmCtx, "while.body");
  llvm::BasicBlock* else_bb  = llvm::BasicBlock::Create(llvmCtx, "while.else");
  llvm::BasicBlock* merge_bb = llvm::BasicBlock::Create(llvmCtx, "while.end");

  llvmBuilder->CreateBr(cond_bb);

  llvmBuilder->SetInsertPoint(cond_bb);
  nodo->condicion->accept(this);
  llvm::Value* cond_v = llvmValor;

  llvm::Value* cero =
    llvm::ConstantInt::get(llvmCtx,
                           llvm::APInt(cond_v->getType()->getIntegerBitWidth(), 0)
                           );
  cond_v = llvmBuilder->CreateICmpNE(cond_v, cero, "whilecond");

  llvmBuilder->CreateCondBr(cond_v, loop_bb, else_bb);

  pilaBreaks.push_back(merge_bb);
  pilaContinues.push_back(cond_bb);

  funcion_actual->insert(funcion_actual->end(), loop_bb);
  llvmBuilder->SetInsertPoint(loop_bb);

  nodo->rama_while->accept(this);

  if (!llvmBuilder->GetInsertBlock()->getTerminator()) {
    llvmBuilder->CreateBr(cond_bb);
  }

  pilaContinues.pop_back();
  pilaBreaks.pop_back();

  funcion_actual->insert(funcion_actual->end(), else_bb);
  llvmBuilder->SetInsertPoint(else_bb);

  if (nodo->rama_sino) {
    nodo->rama_sino->accept(this);
  }

  if (!llvmBuilder->GetInsertBlock()->getTerminator()) {
    llvmBuilder->CreateBr(merge_bb);
  }

  funcion_actual->insert(funcion_actual->end(), merge_bb);
  llvmBuilder->SetInsertPoint(merge_bb);

}

void Emitter::visitar(SentenciaBreak* nodo) {
  if (pilaBreaks.empty()) { //... This should not be here
    std::cerr << "Error: 'break' fuera de un bucle.\n";
    return ;
  }

  llvm::BasicBlock* bloque_salida = pilaBreaks.back();
  llvmBuilder->CreateBr(bloque_salida);

}

void Emitter::visitar(SentenciaContinue* nodo) {
  if (pilaContinues.empty()) { //...
    std::cerr << "Error: 'continue' fuera de un bucle.\n";
    return ;
  }

  llvm::BasicBlock* bloque_condicion = pilaContinues.back();
  llvmBuilder->CreateBr(bloque_condicion);

}

void Emitter::visitar(SentenciaRedo* nodo) {
  if (pilaRedos.empty()) { //...
    std::cerr << "Error: 'redo' fuera de un bucle.\n";
    return ;
  }

  llvm::BasicBlock* bloque_salida = pilaRedos.back();
  llvmBuilder->CreateBr(bloque_salida);

}

void Emitter::visitar(SentenciaReturn* nodo) {
  //std::cout << "[486, emitter.cpp] SentenciaReturn\n";
  nodo->ret_value->accept(this);
  llvmBuilder->CreateRet(llvmValor);

}

void Emitter::visitar(SentenciaFuncDecl* nodo) {
  //std::cout << "[473, emitter.cpp] SentenciaFuncDecl\n";
  std::vector<llvm::Type*> tipo_args;

  if (!structActual.empty()) {
    tipo_args.push_back(llvm::PointerType::getUnqual(llvmCtx));
  }

  for (auto const& [nombre, info] : nodo->args_type) {
    tipo_args.push_back((obtenerTipoLLVM(info.tipo.valor)));
  }

  llvm::Type* tipo_ret = obtenerTipoLLVM(nodo->ret_type.valor);

  llvm::FunctionType* ft = llvm::FunctionType::get(tipo_ret, tipo_args, false);

  llvm::Function* f = llvm::Function::Create(
    ft,
    llvm::Function::ExternalLinkage,
    nodo->nombre_func,
    llvmModulo.get()
  );

  if (nodo->cuerpo_func.empty()) {
    return ;
  }

  llvm::BasicBlock* bb = llvm::BasicBlock::Create(llvmCtx, "entry", f);
  llvmBuilder->SetInsertPoint(bb);

  //llvm_scopes.push_back(std::map<std::string, llvm::AllocaInst*>());

  tablas.entrarScope();

  auto arg_it = f->arg_begin();
  if (!structActual.empty()) {
    llvmThis = &(*arg_it);
    llvmThis->setName("this");
    arg_it++;
  }

  auto it_args_name = nodo->args_type.begin();
  //for (auto &arg : f->args()) {
  //  const std::string& nombre_arg = it_args_name->first;
  //  llvm::AllocaInst* alloca = llvmBuilder->CreateAlloca(arg.getType(), nullptr, "");
  //  llvmBuilder->CreateStore(&arg, alloca);
  //  InfoVariable* info = tablas.buscarVariable(nombre_arg);
  //  if (info) { info->alloca = alloca; }
  //  it_args_name++;
  //}

  for (; arg_it != f->arg_end(); ++arg_it) {
    llvm::Argument& arg = * arg_it;
    const std::string& nombre_arg = it_args_name->first;
    arg.setName(nombre_arg);

    llvm::AllocaInst* alloca = llvmBuilder->CreateAlloca(arg.getType(), nullptr, "");
    llvmBuilder->CreateStore(&arg, alloca);

    InfoVariable* info = tablas.buscarVariable(nombre_arg);
    if (info) { info->alloca = alloca; }

    it_args_name++;

  }

  if (!nodo->cuerpo_func.empty()) {
    for (const auto& inst : nodo->cuerpo_func) {
      if (llvmBuilder->GetInsertBlock()->getTerminator()) { break; }
      inst->accept(this);
    }
  }

  //llvm_scopes.pop_back();

  llvm::verifyFunction(*f);

  tablas.salirScope();

  llvmThis = nullptr;

}

void Emitter::visitar(SentenciaStruct* nodo) {
  llvm::StructType* struct_type = llvm::StructType::create(llvmCtx, nodo->name);
  llvmStructs[nodo->name] = struct_type;

  std::vector<llvm::Type*> tipos_elementos;
  for (auto& prop : nodo->propiedades) {
    auto* nodo_prop = static_cast<SentenciaAsignarVar*>(prop.get());
    llvm::Type* tipo_prop = obtenerTipoLLVM(nodo_prop->tipo_explicito.tipo.valor);
    tipos_elementos.push_back(tipo_prop);

  }

  struct_type->setBody(tipos_elementos, false);

  structActual = nodo->name;
  llvmStructActual = struct_type;

  for (auto& m : nodo->metodos) {
    m->accept(this);
  }

  structActual = "";
  llvmStructActual = nullptr;

}

void Emitter::visitar(SentenciaEscritura* nodo) {

}

void Emitter::visitar(SentenciaArcano* nodo) {
  std::cout << "[803, emitter.cpp] SentenciaArcano\n";

}

void Emitter::visitar(SentenciaLlamadaArcano* nodo) { //...
  std::cout << "[809, emitter.cpp] SentenciaLlamadaArcano\n";
  ArcaneDef& def = contextoArcanos.buscarDefinicionPorKeyword(nodo->nombre);

  stackArcanos.push_back(nodo);

  if (nodo->indice_rama >= def.branches.size()) {
    std::cerr << "Error interno: Índice de rama fuera de rango para '" << nodo->nombre << "'.\n";
    exit(1);

  }

  ArcaneBranch* rama_elegida = &def.branches[nodo->indice_rama];

  tablas.entrarScope();

  auto backup_bloques = bloquesArcanoActivos;

  for (const auto& [nombre_arg, ast_arg] : nodo->args) {
    ast_arg->accept(this);
    llvm::Value* valor_arg = llvmValor;

    llvm::AllocaInst* alloca = llvmBuilder->CreateAlloca(valor_arg->getType(), nullptr, nombre_arg);
    llvmBuilder->CreateStore(valor_arg, alloca);

    InfoVariable* info = tablas.buscarVariable(nombre_arg);

    if (info) {
      info->alloca = alloca;

    } else {
      std::cerr << "Error: Argumento '" << nombre_arg << "' no encontrado.\n";

    }

  }

  for (const auto& [nombre_arg, ast_arg] : nodo->expr) {
    bloquesArcanoActivos[nombre_arg] = ast_arg.get();
  }

  for (const auto& [nombre_arg, ast_arg] : nodo->code) {
    bloquesArcanoActivos[nombre_arg] = ast_arg.get();
  }

  for (const auto& seg: rama_elegida->segmentos) {
    if (seg.br_cont) {
      seg.br_cont->accept(this);
    }
  }

  bloquesArcanoActivos = backup_bloques;
  tablas.salirScope();

  stackArcanos.pop_back();

}

void Emitter::visitar(SentenciaMetaDirective* nodo) {
  std::cout << "[871, emitter.cpp] SentenciaMetaDirective\n";

  switch (nodo->id) {

    case MetaID::CHAIN: {

      if (stackArcanos.empty()) {
        throw std::runtime_error("Error: stackArcanos está vacío");
      }

      auto* arg_literal = dynamic_cast<ExprLiteral*>(nodo->args[0].get());
      if (!arg_literal || !std::holds_alternative<RuleData>(arg_literal->datos)) {
        throw std::runtime_error("Error: ?chain espera una regla como argumento");
      }
      std::string target_rule = std::get<RuleData>(arg_literal->datos).rule;

      SentenciaLlamadaArcano* llamada_actual = stackArcanos.back();
      SentenciaLlamadaArcano* cadena         = nullptr;

      for (const auto& chain : llamada_actual->chains) {
        if (chain->rule_tag == target_rule) { // cadena->rule_tag
          cadena = chain.get();
          break;
        }
      }

      if (cadena) {
        stackArcanos.push_back(cadena);
        auto backup_bloques = bloquesArcanoActivos;

        for (const auto& [nombre_arg, ast_arg] : cadena->expr) {
          bloquesArcanoActivos[nombre_arg] = ast_arg.get();
        }
        for (const auto& [nombre_arg, ast_arg] : cadena->code) {
          bloquesArcanoActivos[nombre_arg] = ast_arg.get();
        }

        if (nodo->body) {
          nodo->body->accept(this);
        }

        bloquesArcanoActivos = backup_bloques;
        stackArcanos.pop_back();

      }
      break;

    }

    default: { break; }

  }

}
