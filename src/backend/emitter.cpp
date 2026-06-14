// emitter.cpp

#include "Emitter.hpp"

#include "Common.hpp"

#include "string_data.h"

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

    case BT::NOSCOPE: {
      handleNoscope(nodo, idx);
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

void TraitEmitter::handleNoscope(Bloque* nodo, size_t idx) {
  emitter.tablas.salirScope();
  despacharTrait(nodo, idx + 1);
  emitter.tablas.entrarScope();
}

/* --- Emitter --- */
Emitter::Emitter(ContextoArcanos& ca, GestorTablas& t)
  : contextoArcanos(ca), tablas(t), traits(*this) {
  llvmModulo  = std::make_unique<llvm::Module>("ArcanaModulo", llvmCtx);
  llvmBuilder = std::make_unique<llvm::IRBuilder<>>(llvmCtx);

  tablas.resetScope();

  // --- Runtime ---

  llvm::StringRef bc_data(reinterpret_cast<const char*>(string_bc), string_bc_len);
  std::unique_ptr<llvm::MemoryBuffer> buffer = llvm::MemoryBuffer::getMemBuffer(bc_data);

  llvm::SMDiagnostic error;

  std::unique_ptr<llvm::Module> runtime = llvm::parseIR(buffer->getMemBufferRef(), error, llvmCtx);

  if (!runtime) {
    error.print("Emitter", llvm::errs());
    exit(1);
  }

  bool link_error = llvm::Linker::linkModules(*llvmModulo, std::move(runtime));
  if (link_error) {
    std::cerr << "Error al funsionar el rutime\n";
    exit(1);
  }

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

    case TypeKind::ARRAY: {
      auto array_type = std::static_pointer_cast<ArrayType>(tipo);
      llvm::Type* tipo_base_llvm = obtenerTipoLLVM(array_type->getUnderlyingType());

      if (array_type->size != -1) {
        return llvm::ArrayType::get(tipo_base_llvm, array_type->size);
      }

      return llvm::PointerType::getUnqual(llvmCtx);
    }

    case TypeKind::POINTER: {
      return llvm::PointerType::getUnqual(llvmCtx);
    }

    case TypeKind::STRUCT: {
      return llvmStructs[tipo->toString()];
    }

    case TypeKind::STRING: {
      llvm::Type* i8_ptr = llvm::PointerType::getUnqual(llvmCtx);
      llvm::Type* i64_ty = llvm::Type::getInt64Ty(llvmCtx);

      return llvm::StructType::get(llvmCtx, { i8_ptr, i64_ty, i64_ty} );
    }

    default: {
      std::cout << "[136, emitter.cpp] NULLPTR: TIPO NO RECONOCIDO: '" << tipo->toString() << "'\n";
      return nullptr;
    }

  }

}

llvm::Value* Emitter::obtenerPuntero(Expresion* nodo) {

  if (auto* var = dynamic_cast<ExprVariable*>(nodo)) {
    InfoVariable* info = tablas.buscarVariable(var->nombre);
    if (info && info->alloca) {
      return info ? info->alloca : nullptr;

    }

    if (llvmThis != nullptr && !structActual.empty()) {
      InfoStruct* info_struct = tablas.buscarStruct(structActual);
      if (info_struct) {
        auto it = std::find(info_struct->orden_props.begin(), info_struct->orden_props.end(), var->nombre);
        if (it != info_struct->orden_props.end()) {
          int i = std::distance(info_struct->orden_props.begin(), it);
          return llvmBuilder->CreateStructGEP(llvmStructActual, llvmThis, i, "");
        }
      }
    }

    return nullptr;

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

  if (auto* acceso = dynamic_cast<ExprAcceso*>(nodo)) {
    llvm::Value* ptr_base = obtenerPuntero(acceso->contenedor.get());
    if (!ptr_base) { return nullptr; }

    acceso->rango->accept(this);
    llvm::Value* index_val = llvmValor;

    auto tipo_contenedor = acceso->contenedor->tipo_resuelto.valor;

    if (tipo_contenedor->kind == TypeKind::POINTER ||
        tipo_contenedor->kind == TypeKind::ARRAY   && std::static_pointer_cast<ArrayType>(tipo_contenedor)-> size == -1) {

      acceso->contenedor->accept(this);
      llvm::Value* ptr_val = llvmValor;

      auto tipo_base = tipo_contenedor->getUnderlyingType();
      llvm::Type* tipo_base_llvm = obtenerTipoLLVM(tipo_base);

      return llvmBuilder->CreateInBoundsGEP(tipo_base_llvm, ptr_val, index_val, "");

    }

    //if (tipo_contenedor->kind == TypeKind::ARRAY) {
    //  auto array_type = std::static_pointer_cast<ArrayType>(tipo_contenedor);
    //  if (array_type->size != -1) {
    //    llvm::Type* tipo_array_llvm = obtenerTipoLLVM(array_type);
    //    llvm::Value* cero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), 0);
    //    std::vector<llvm::Value*> indices = { cero, index_val };
    //    return llvmBuilder->CreateInBoundsGEP(tipo_array_llvm, ptr_base, indices, "");
    //  } else {
    //    llvm::Type* tipo_base_llvm = obtenerTipoLLVM(array_type->getUnderlyingType());
    //    return llvmBuilder->CreateInBoundsGEP(tipo_base_llvm, ptr_base, index_val, "");
    //  }
    //}

    if (tipo_contenedor->kind == TypeKind::ARRAY) {
      llvm::Value* ptr_base = obtenerPuntero(acceso->contenedor.get());
      if (!ptr_base) { return nullptr; }

      auto array_type = std::static_pointer_cast<ArrayType>(tipo_contenedor);
      llvm::Type* tipo_array_llvm = obtenerTipoLLVM(array_type);

      llvm::Value* cero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), 0);
      std::vector<llvm::Value*> indices = { cero, index_val };

      return llvmBuilder->CreateInBoundsGEP(tipo_array_llvm, ptr_base, indices, "");

    }

    if (tipo_contenedor->kind == TypeKind::STRING) {
      llvm::Type* str_ty = obtenerTipoLLVM(tipo_contenedor);
      llvm::Value* gep = llvmBuilder->CreateStructGEP(str_ty, ptr_base, 0, "");

      llvm::Value* data = llvmBuilder->CreateLoad(llvm::PointerType::getUnqual(llvmCtx), gep, "");

      return llvmBuilder->CreateInBoundsGEP(llvm::Type::getInt8Ty(llvmCtx), data, index_val, "");

    }
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
  std::cout << "[305, emitter.cpp] ExprLiteral\n";
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

    case TypeKind::STRING: {
      auto& data = std::get<StringData>(nodo->datos);

      llvm::Value* str = llvmBuilder->CreateGlobalString(data.contenido, "");

      llvm::Type* str_ty = obtenerTipoLLVM(tipo);
      llvm::Value* str_struct = llvm::UndefValue::get(str_ty);

      llvm::Value* len_val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmCtx), data.contenido.length());

      str_struct = llvmBuilder->CreateInsertValue(str_struct, str, 0);
      str_struct = llvmBuilder->CreateInsertValue(str_struct, len_val, 1);
      str_struct = llvmBuilder->CreateInsertValue(str_struct, len_val, 2);

      llvmValor = str_struct;
      break;
    }

    default: {
      break;
    }
  }

}

void Emitter::visitar(ExprVariable* nodo) {
  std::cout << "[360, emitter.cpp] ExprVariable\n";
  //std::cout << nodo->nombre << '\n';

  llvmPunteroBase = nullptr;
  InfoVariable* info = tablas.buscarVariable(nodo->nombre);

  if (info && info->alloca) {
    llvmPunteroBase = info->alloca;

    if (info->tipo.valor->kind == TypeKind::ARRAY) {
      llvmValor = info->alloca;

    } else {
      llvmValor = llvmBuilder->CreateLoad(info->alloca->getAllocatedType(), info->alloca, "");

    }

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

        llvmPunteroBase = ptr_campo;

        auto tipo_campo_s = info_struct->propiedades[nodo->nombre].tipo.valor;

        if (tipo_campo_s->kind == TypeKind::ARRAY) {
          llvmValor = ptr_campo;

        } else {
          llvm::Type* tipo_campo = obtenerTipoLLVM(info_struct->propiedades[nodo->nombre].tipo.valor);
          llvmValor = llvmBuilder->CreateLoad(tipo_campo, ptr_campo, nodo->nombre);

        }

        return ;
      }
    }
  }

  std::cerr << "Error: Variable '" << nodo->nombre << "' no encontrada.\n";

}

void Emitter::visitar(ExprArray* nodo) {
  size_t cantidad = nodo->elementos.size();

  llvm::Value* size_val = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), cantidad);

  auto tipo_array = std::static_pointer_cast<ArrayType>(nodo->tipo_resuelto.valor);
  llvm::Type* tipo_base_llvm = obtenerTipoLLVM(tipo_array->getUnderlyingType());

  llvm::AllocaInst* alloca = llvmBuilder->CreateAlloca(tipo_base_llvm, size_val, "");

  for (size_t i = 0; i < cantidad; ++i) {
    nodo->elementos[i]->accept(this);
    llvm::Value* valor_elem = llvmValor;

    llvm::Value* index_val = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), i);

    llvm::Value* ptr_elem = llvmBuilder->CreateGEP(tipo_base_llvm, alloca, index_val, "");

    llvmBuilder->CreateStore(valor_elem, ptr_elem);

  }

  llvmValor = alloca;

}

void Emitter::visitar(ExprUnaria* nodo) {
  std::cout << "[439, emitter.cpp] ExprUnaria\n";

  llvmPunteroBase = nullptr;

  if (nodo->operador == TipoOperador::PTR_REF) {
    llvm::Value* ptr = obtenerPuntero(nodo->operando.get());

    if (ptr) {
      llvmValor = ptr;

    } else {
      nodo->operando->accept(this);
      llvm::Value* val = llvmValor;
      llvm::AllocaInst* tmp_alloca = llvmBuilder->CreateAlloca(val->getType(), nullptr, "");
      llvmBuilder->CreateStore(val, tmp_alloca);
      llvmValor = tmp_alloca;

    }

    return ;

  }

  nodo->operando->accept(this);
  llvm::Value* val = llvmValor;

  if (!val) { return ; }

  llvmPunteroBase = nullptr;

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

      llvmPunteroBase = ptr_val;

      llvmValor = llvmBuilder->CreateLoad(tipo_llvm, ptr_val, "");
      break;

    }

    default: {
      std::cout << "[392, emitter.cpp] Error: Operador unario no implementado.";
      exit(1);
    }

  }

}

void Emitter::visitar(ExprBinaria* nodo) {
  std::cout << "[548, emitter.cpp] ExprBinaria\n";
  std::cout << "[549, emitter.cpp] nodo->overload: '" << nodo->overload << "'\n";

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
      nodo->izquierda->accept(this);
      llvm::Value* valor_izq = llvmValor;

      llvm::Type* tipo_valor = valor_izq->getType();
      ptr_this = llvmBuilder->CreateAlloca(tipo_valor, nullptr, "");
      llvmBuilder->CreateStore(valor_izq, ptr_this);

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
      std::cout << "[594, emitter.cpp]\n";

      if (nodo->tipo_resuelto.valor->kind == TypeKind::STRING) {
        std::cout << "[597, emitter.cpp] String\n";
        llvm::Type* str_ty = obtenerTipoLLVM(nodo->tipo_resuelto.valor);

        llvm::AllocaInst* ptr_out = llvmBuilder->CreateAlloca(str_ty, nullptr, "");
        llvm::AllocaInst* ptr_a   = llvmBuilder->CreateAlloca(str_ty, nullptr, "");
        llvm::AllocaInst* ptr_b   = llvmBuilder->CreateAlloca(str_ty, nullptr, "");

        llvmBuilder->CreateStore(left, ptr_a);
        llvmBuilder->CreateStore(right, ptr_b);

        llvm::Function* func_concat = llvmModulo->getFunction("str_concat");
        if (!func_concat) {
          std::cerr << "Error: No se encunetra 'str_concat'\n";
          return ;
        }

        llvmBuilder->CreateCall(func_concat, {ptr_out, ptr_a, ptr_b} );
        llvmValor = llvmBuilder->CreateLoad(str_ty, ptr_out, "");

      } else if (nodo->tipo_resuelto.valor->kind == TypeKind::POINTER) {
        llvm::Value* ptr_val = left->getType()->isPointerTy() ? left : right;
        llvm::Value* val     = left->getType()->isPointerTy() ? right : left;

        auto tipo_base = nodo->tipo_resuelto.valor->getUnderlyingType();
        llvm::Type* llvm_base = obtenerTipoLLVM(tipo_base);
        llvmValor = llvmBuilder->CreateGEP(llvm_base, ptr_val, val, "");

      } else {

        llvmValor = es_float ? llvmBuilder->CreateFAdd(left, right, "")
                             : llvmBuilder->CreateAdd(left, right, "");

      }
      break;

    }

    case TipoOperador::RESTA: {
      std::cout << "[635, emitter.cpp]\n";

      if (left->getType()->isPointerTy() && !right->getType()->isPointerTy()) {
        llvm::Value* val = llvmBuilder->CreateNeg(right, "");
        auto tipo_base = nodo->tipo_resuelto.valor->getUnderlyingType();
        llvm::Type* llvm_base = obtenerTipoLLVM(tipo_base);
        llvmValor = llvmBuilder->CreateGEP(llvm_base, left, val, "");

      } else {
        llvmValor = es_float ? llvmBuilder->CreateFSub(left, right, "")
                             : llvmBuilder->CreateSub(left, right, "");
      }

      break;

    }

    case TipoOperador::MULT: {
      std::cout << "[653, emitter.cpp]\n";

      if (nodo->izquierda->tipo_resuelto.valor->kind == TypeKind::STRING  &&
          nodo->derecha  ->tipo_resuelto.valor->kind == TypeKind::INTEGER ||
          nodo->izquierda->tipo_resuelto.valor->kind == TypeKind::INTEGER &&
          nodo->derecha  ->tipo_resuelto.valor->kind == TypeKind::STRING) {

        llvm::Type* str_ty = nullptr;

        if (nodo->derecha->tipo_resuelto.valor->kind == TypeKind::STRING) {
          str_ty = obtenerTipoLLVM(nodo->derecha  ->tipo_resuelto.valor);
        } else {
          str_ty = obtenerTipoLLVM(nodo->izquierda->tipo_resuelto.valor);

        }

        llvm::AllocaInst* ptr_out = llvmBuilder->CreateAlloca(str_ty, nullptr, "");
        llvm::AllocaInst* ptr_a   = llvmBuilder->CreateAlloca(str_ty, nullptr, "");
        llvmBuilder->CreateStore(left, ptr_a);
        llvm::Value* b = llvmBuilder->CreateIntCast(right, llvm::Type::getInt64Ty(llvmCtx), true, "");

        llvm::Function* func_repeat = llvmModulo->getFunction("str_repeat");
        if (!func_repeat) {
          std::cerr << "Error: No se encontró 'str_repeat'\n";
          return ;
        }

        llvmBuilder->CreateCall(func_repeat, {ptr_out, ptr_a, b});
        llvmValor = llvmBuilder->CreateLoad(str_ty, ptr_out, "");

      } else {
        llvmValor = es_float ? llvmBuilder->CreateFMul(left, right, "")
                             : llvmBuilder->CreateMul(left, right, "");
      }
      break;
    }

    case TipoOperador::CMP_MAYOR:
    case TipoOperador::CMP_MAYOR_IGUAL:
    case TipoOperador::CMP_IGUAL:
    case TipoOperador::CMP_DISTINTO:
    case TipoOperador::CMP_MENOR_IGUAL:
    case TipoOperador::CMP_MENOR: {
      std::cout << "[696, emitter.cpp]\n";
      llvm::CmpInst::Predicate pred = obtenerPredicadoCmp(nodo->operador, es_float, nodo->derecha->tipo_resuelto.valor->isSigned());
      llvmValor = llvmBuilder->CreateCmp(pred, left, right);
      break;
    }

    case TipoOperador::SWAP: {
      llvm::Value* ptr_l = obtenerPuntero(nodo->izquierda.get());
      llvm::Value* ptr_r = obtenerPuntero(nodo->derecha  .get());

      if (!ptr_l || !ptr_r) {
        std::cerr << "Error en swap\n";
        return ;
      }

      auto tipo = nodo->tipo_resuelto.valor;
      llvm::Type* tipo_llvm = obtenerTipoLLVM(tipo);

      llvm::Value* val_l = llvmBuilder->CreateLoad(tipo_llvm, ptr_l, "");
      llvm::Value* val_r = llvmBuilder->CreateLoad(tipo_llvm, ptr_r, "");

      llvmBuilder->CreateStore(val_r, ptr_l);
      llvmBuilder->CreateStore(val_l, ptr_r);

      llvmValor = val_r;
      llvmPunteroBase = nullptr;

      break;

    }

    default: {
      std::cout << "[741, emitter.cpp]\n";
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
  std::shared_ptr<ArcanaType> t_destino = nodo->           tipo_resuelto.valor;

  if (t_origen->kind == TypeKind::STRING && t_destino->kind == TypeKind::POINTER) {
    llvmValor = llvmBuilder->CreateExtractValue(val, {0}, "");
    return ;
  }

  if (t_destino->kind == TypeKind::BOOLEAN) {
    switch (t_origen->kind) { //... Todo: pointer -> bool

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
  std::cout << "[738, emitter.cpp] ExprRango\n";

  if (nodo->inicio && !nodo->fin && !nodo->paso) {
    nodo->inicio->accept(this);
    return ;
  }

  std::cerr << "Error: Slicing no implementado\n";
  llvmValor = nullptr;

}

void Emitter::visitar(ExprAcceso* nodo) {
  std::cout << "[751, emitter.cpp] ExprAcceso\n";
  llvm::Value* ptr_elemento = obtenerPuntero(nodo);

  if (!ptr_elemento) { return ; }

  llvmPunteroBase = ptr_elemento;

  llvm::Type* tipo_elemento_llvm = obtenerTipoLLVM(nodo->tipo_resuelto.valor);
  llvmValor = llvmBuilder->CreateLoad(tipo_elemento_llvm, ptr_elemento, "");

}

void Emitter::visitar(ExprAccesoPunto* nodo) {
  std::cout << "[763, emitter.cpp] ExprAccesoPunto\n";
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
  InfoFuncion* info = nodo->info;

  if (!info) {
    std::cerr << "Error !info\n";
    exit(1);
  }

  std::cout << "[1011, emitter.cpp] info->firma: '" << info->firma << "'\n";

  std::string llvm_name = info->is_external ? info->nombre : info->firma;
  llvm::Function* callee_f = llvmModulo->getFunction(llvm_name);

  if (!callee_f) {
    std::vector<llvm::Type*> args_type;
    for (const auto& param : info->tipos_parametros) {
      args_type.push_back((obtenerTipoLLVM(param.second.tipo.valor)));
    }

    llvm::Type* ret_type = obtenerTipoLLVM(info->tipo_retorno.valor);
    llvm::FunctionType* ft = llvm::FunctionType::get(ret_type, args_type, info->is_variadic);

    callee_f = llvm::Function::Create(
      ft,
      llvm::Function::ExternalLinkage,
      llvm_name,
      llvmModulo.get()
    );
  }

  std::vector<llvm::Value*> args_v;
  for (auto& arg: nodo->argumentos) {
    arg.second->accept(this);
    args_v.push_back(llvmValor);
  }

  llvmValor = llvmBuilder->CreateCall(callee_f, args_v, "");

}

void Emitter::visitar(ExprInitList* nodo) {
  std::cout << "[820, emitter.cpp] ExprInitList\n";
  auto struct_type = std::static_pointer_cast<StructType>(nodo->tipo_resuelto.valor);
  llvm::Type* struct_llvm_type = obtenerTipoLLVM(struct_type);

  llvm::Value* struct_val = llvm::UndefValue::get(struct_llvm_type);

  const auto& orden_props = struct_type->info->orden_props;

  for (const auto& arg : nodo->args) {

    arg.value->accept(this);
    llvm::Value* arg_val = llvmValor;

    unsigned idx = 0;

    if (arg.name.has_value()) {
      for (size_t i = 0; i < orden_props.size(); ++i) {
        if (orden_props[i] == arg.name.value()) {
          idx = i;
          break;

        }
      }
    }

    struct_val = llvmBuilder->CreateInsertValue(struct_val, arg_val, idx);

  }

  llvmValor = struct_val;

}

// --- Sentencias --- //

void Emitter::visitar(Bloque* nodo) {
  std::cout << "[856, emitter.cpp] Bloque\n";

  tablas.entrarScope();

  if (enScopeGlobal) {
    for (const auto& inst : nodo->instrucciones) {
      inst->accept(this);
    }

  } else {
    traits.despacharTrait(nodo, 0);

  }

  tablas.salirScope();

}

void Emitter::visitar(SentenciaAsignarVar* nodo) {
  std::cout << "[918, emitter.cpp] SentenciaAsignarVar\n";

  llvm::AllocaInst* alloca = nullptr;
  InfoVariable* info = tablas.buscarVariable(nodo->nombre);

  bool es_array = (nodo->tipo_explicito.tipo.valor->kind == TypeKind::ARRAY);
  llvm::Type* tipo_llvm = obtenerTipoLLVM(nodo->tipo_explicito.tipo.valor);
  llvm::Value* size_val = nullptr;

  if (es_array) {
    auto array_type = std::static_pointer_cast<ArrayType>(nodo->tipo_explicito.tipo.valor);

    if (array_type->size != -1) {
      alloca = llvmBuilder->CreateAlloca(tipo_llvm, nullptr, "");
      size_val = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), array_type->size);

    } else {
      llvm::Type* tipo_base_llvm = obtenerTipoLLVM(array_type->getUnderlyingType());

      if (nodo->size) {
        nodo->size->accept(this);
        size_val = llvmValor;
      } else if (nodo->valor_inicial) {
        if (auto expr_array = dynamic_cast<ExprArray*>(nodo->valor_inicial.get())) {
          size_val = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), expr_array->elementos.size());

        } else if (auto expr_var = dynamic_cast<ExprVariable*>(nodo->valor_inicial.get())) {
          InfoVariable* info_init = tablas.buscarVariable(expr_var->nombre);
          if (info_init && info_init->array_size) {
            size_val = info_init->array_size;
          }
        }
      }
      alloca = llvmBuilder->CreateAlloca(tipo_llvm, size_val, "");

    }

    //alloca = llvmBuilder->CreateAlloca(tipo_llvm, size_val, "");

  } else {
    alloca = llvmBuilder->CreateAlloca(tipo_llvm, nullptr, "");

  }

  if (nodo->valor_inicial) {
    nodo->valor_inicial->accept(this);
    llvm::Value* init_val = llvmValor;

    if (es_array) {

      auto array_type = std::static_pointer_cast<ArrayType>(nodo->tipo_explicito.tipo.valor);
      const llvm::DataLayout& dl = llvmModulo->getDataLayout();

      llvm::Type* tipo_base_llvm = obtenerTipoLLVM(array_type->getUnderlyingType());
      uint64_t bytes_elem = dl.getTypeAllocSize(tipo_base_llvm);
      llvm::Align align = dl.getPrefTypeAlign(tipo_base_llvm);

      llvm::Value* bytes_val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmCtx), bytes_elem);
      llvm::Value* size_i64  = llvmBuilder->CreateIntCast(size_val, llvm::Type::getInt64Ty(llvmCtx), false, "");
      llvm::Value* total_bytes = llvmBuilder->CreateMul(size_i64, bytes_val, "");

      llvm::Value* alloca_base = alloca;
      if (array_type->size != -1) {
        llvm::Value* cero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), 0);
        std::vector<llvm::Value*> indices = { cero, cero };
        alloca_base = llvmBuilder->CreateInBoundsGEP(tipo_llvm, alloca, indices, "");
      }

      llvm::Value* init_base = init_val;
      if (nodo->valor_inicial->tipo_resuelto.valor->kind == TypeKind::ARRAY) {
        auto arr_init_type = std::static_pointer_cast<ArrayType>(nodo->valor_inicial->tipo_resuelto.valor);
        if (arr_init_type->size != -1) {
          llvm::Type* tipo_arr_init_llvm = obtenerTipoLLVM(arr_init_type);
          llvm::Value* cero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), 0);
          std::vector<llvm::Value*> indices = { cero, cero };
          init_base = llvmBuilder->CreateInBoundsGEP(tipo_arr_init_llvm, init_val, indices, "");
        }
      }

      llvmBuilder->CreateMemCpy(alloca, align, init_val, align, total_bytes);

    } else {
      llvmBuilder->CreateStore(init_val, alloca);

    }

  } else if (nodo->tipo_explicito.tipo.valor->kind == TypeKind::STRUCT) {
    std::string name = nodo->tipo_explicito.tipo.valor->toString();
    llvm::Function* init_f = llvmModulo->getFunction(name + "_init");

    if (init_f) {
      llvmBuilder->CreateCall(init_f, {alloca}, "");
    }
  }

  if (info) {
    info->alloca = alloca;

    if (es_array) {
      info->array_size = size_val;
    }

  }

}

void Emitter::visitar(SentenciaExpr* nodo) {
  std::cout << "[752, emitter.cpp] SentenciaExpr\n";
  if (nodo->expresion) {
    nodo->expresion->accept(this);
  }

}

void Emitter::visitar(SentenciaReasignacionVar* nodo) {
  std::cout << "[1018, emitter.cpp] SentenciaReasignacionVar\n";

  llvm::Value* destino_ptr = obtenerPuntero(nodo->izquierda.get());

  nodo->derecha->accept(this);

  llvm::Value* valor_asignar = llvmValor;
  auto tipo_resuelto = nodo->derecha->tipo_resuelto.valor;

  if (tipo_resuelto->kind == TypeKind::ARRAY) {

    auto tipo_izq = nodo->izquierda->tipo_resuelto.valor;
    if (tipo_izq->kind == TypeKind::ARRAY) {
      auto array_izq = std::static_pointer_cast<ArrayType>(tipo_izq);
      if (array_izq->size != -1) {
        llvm::Type* tipo_array_llvm = obtenerTipoLLVM(array_izq);
        llvm::Value* cero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), 0);
        std::vector<llvm::Value*> indices = { cero, cero};
        destino_ptr = llvmBuilder->CreateInBoundsGEP(tipo_array_llvm, destino_ptr, indices, "");
      }
    }

    if (auto array_der = std::static_pointer_cast<ArrayType>(tipo_resuelto)) {
      if (array_der->size != -1 && llvm::isa<llvm::AllocaInst>(valor_asignar)) {
        llvm::Type* tipo_array_llvm_der = obtenerTipoLLVM(array_der);
        llvm::Value* cero = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), 0);
        std::vector<llvm::Value*> indices = { cero, cero };
        valor_asignar = llvmBuilder->CreateInBoundsGEP(tipo_array_llvm_der, valor_asignar, indices, "");
      }
    }

    auto array_type = std::static_pointer_cast<ArrayType>(tipo_resuelto);
    llvm::Type* tipo_array_llvm = obtenerTipoLLVM(array_type);

    const llvm::DataLayout& dl = llvmModulo->getDataLayout();
    llvm::Align align;
    llvm::Value* total_bytes = nullptr;

    if (array_type->size != -1) {
      uint64_t bytes = dl.getTypeAllocSize(tipo_array_llvm);
      align = dl.getPrefTypeAlign(tipo_array_llvm);
      total_bytes = llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmCtx), bytes);

    } else {
      llvm::Type* tipo_base_llvm = obtenerTipoLLVM(array_type->getUnderlyingType());
      uint64_t bytes_elem = dl.getTypeAllocSize(tipo_base_llvm);
      align = dl.getPrefTypeAlign(tipo_base_llvm);

      llvm::Value* bytes_val = llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmCtx), bytes_elem);
      llvm::Value* size_val  = nullptr;

      if (auto expr_array = dynamic_cast<ExprArray*>(nodo->derecha.get())) {
        size_val = llvm::ConstantInt::get(llvm::Type::getInt32Ty(llvmCtx), expr_array->elementos.size());

      } else if (auto expr_var = dynamic_cast<ExprVariable*>(nodo->derecha.get())) {
        InfoVariable* info = tablas.buscarVariable(expr_var->nombre);

        if (info && info->array_size) {
          size_val = info->array_size;
        }
      }

      if (size_val) {
        llvm::Value* size_i64 = llvmBuilder->CreateIntCast(size_val, llvm::Type::getInt64Ty(llvmCtx), false, "");
        total_bytes = llvmBuilder->CreateMul(size_i64, bytes_val, "");

      } else {
        total_bytes = llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmCtx), 0);

      }
    }

    llvmBuilder->CreateMemCpy(destino_ptr, align, valor_asignar, align, total_bytes);

  } else {
    llvmBuilder->CreateStore(valor_asignar, destino_ptr);

  }

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
  std::cout << "[910, emitter.cpp] SentenciaReturn\n";
  nodo->ret_value->accept(this);
  llvmBuilder->CreateRet(llvmValor);

}

void Emitter::visitar(SentenciaFuncDecl* nodo) {
  std::cout << "[917, emitter.cpp] SentenciaFuncDecl\n";

  bool scope_anterior = enScopeGlobal;
  enScopeGlobal = false;

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
    nodo->firma_mangled,
    llvmModulo.get()
  );

  if (nodo->cuerpo_func.empty()) {
    return ;
  }

  llvm::BasicBlock* bb = llvm::BasicBlock::Create(llvmCtx, "entry", f);
  llvmBuilder->SetInsertPoint(bb);

  tablas.entrarScope();

  auto arg_it = f->arg_begin();
  if (!structActual.empty()) {
    llvmThis = &(*arg_it);
    llvmThis->setName("this");
    arg_it++;
  }

  auto it_args_name = nodo->args_type.begin();

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

    if (!llvmBuilder->GetInsertBlock()->getTerminator()) {
      if (tipo_ret->isVoidTy()) {
        llvmBuilder->CreateRetVoid();

      } else {
        llvmBuilder->CreateUnreachable();

      }
    }
  }

  llvm::verifyFunction(*f);

  tablas.salirScope();

  llvmThis = nullptr;

  enScopeGlobal = scope_anterior;

}

void Emitter::visitar(SentenciaStruct* nodo) {
  std::cout << "[1322, emitter.cpp] SentenciaStruct\n";

  tablas.entrarScope();

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

  llvm::FunctionType* init_ft = llvm::FunctionType::get(
    llvm::Type::getVoidTy(llvmCtx),
    {llvm::PointerType::getUnqual(llvmCtx)},
    false
  );
  llvm::Function* init_f = llvm::Function::Create(
    init_ft,
    llvm::Function::InternalLinkage,
    nodo->name + "_init",
    llvmModulo.get()
  );

  llvm::BasicBlock* prev_bb = llvmBuilder->GetInsertBlock();
  llvm::BasicBlock* init_bb = llvm::BasicBlock::Create(llvmCtx, "entry", init_f);
  llvmBuilder->SetInsertPoint(init_bb);

  llvm::Value* prev_this = llvmThis;
  llvmThis = init_f->getArg(0);
  llvmThis->setName("this");

  for (size_t i = 0; i < nodo->propiedades.size(); ++i) {
    auto* nodo_prop = static_cast<SentenciaAsignarVar*>(nodo->propiedades[i].get());

    if (nodo_prop->valor_inicial) {
      nodo_prop->valor_inicial->accept(this);
      llvm::Value* init_val = llvmValor;
      llvm::Value* ptr_campo = llvmBuilder->CreateStructGEP(llvmStructActual, llvmThis, i, "");

      auto tipo = nodo_prop->tipo_explicito.tipo.valor;
      bool es_array = (tipo->kind == TypeKind::ARRAY);
      bool es_fijo = false;

      if (es_array) {
        auto arr_t = std::static_pointer_cast<ArrayType>(tipo);
        es_fijo = (arr_t->size != -1);
      }

      if (es_array && es_fijo) {
        llvm::Type* tipo_prop_llvm = obtenerTipoLLVM(tipo);
        const llvm::DataLayout& dl = llvmModulo->getDataLayout();

        uint64_t bytes = dl.getTypeAllocSize(tipo_prop_llvm);
        llvm::Align align = dl.getPrefTypeAlign(tipo_prop_llvm);
        llvm::Value* total_bytes = llvm::ConstantInt::get(llvm::Type::getInt64Ty(llvmCtx), bytes);

        llvmBuilder->CreateMemCpy(ptr_campo, align, init_val, align, total_bytes);

      } else {
        llvmBuilder->CreateStore(init_val, ptr_campo);

      }
    }
  }

  llvmBuilder->CreateRetVoid();
  if (prev_bb) { llvmBuilder->SetInsertPoint(prev_bb); }

  for (auto& m : nodo->metodos) {
    m->accept(this);
  }

  structActual = "";
  llvmStructActual = nullptr;
  llvmThis = prev_this;

  tablas.salirScope();

}

void Emitter::visitar(SentenciaEscritura* nodo) {

}

void Emitter::visitar(SentenciaArcano* nodo) {
  std::cout << "[1034, emitter.cpp] SentenciaArcano\n";

}

void Emitter::visitar(SentenciaLlamadaArcano* nodo) { //...
  std::cout << "[1039, emitter.cpp] SentenciaLlamadaArcano\n";
  ArcaneDef& def = contextoArcanos.buscarDefinicionPorKeyword(nodo->nombre);

  stackArcanos.push_back(nodo);

  if (nodo->indice_rama >= def.branches.size()) {
    std::cerr << "Error interno: Índice de rama fuera de rango para '" << nodo->nombre << "'.\n";
    exit(1);

  }

  ArcaneBranch* rama_elegida = &def.branches[nodo->indice_rama];

  tablas.entrarScope();

  auto backup_vars = varsArcanosActivos;
  for (const auto& [nombre_arg, ast_arg] : nodo->vars) {
    varsArcanosActivos[nombre_arg] = ast_arg.get();
  }

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

  //for (const auto& seg: rama_elegida->segmentos) {
  //  if (seg.br_cont) {
  //    seg.br_cont->accept(this);
  //  }
  //}

  for (const auto& inst : nodo->nodos_expandidos) {
    inst->accept(this);
  }

  varsArcanosActivos = backup_vars;
  bloquesArcanoActivos = backup_bloques;

  tablas.salirScope();

  stackArcanos.pop_back();

}

void Emitter::visitar(SentenciaMetaDirective* nodo) {
  std::cout << "[1108, emitter.cpp] SentenciaMetaDirective\n";

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

void Emitter::visitar(SentenciaTemplate* nodo) {}

void Emitter::visitar(SentenciaInclude* nodo) {}
