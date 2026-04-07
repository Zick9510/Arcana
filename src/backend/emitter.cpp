// emitter.cpp

#include "Common.hpp"
#include "Emitter.hpp"

// --- LLVM --- //
llvm::Type* Emitter::obtenerTipoLLVM(ArcanaType* tipo) {
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

// --- Expresiones --- //

void Emitter::visitar(ExprNumero* nodo) {

}

void Emitter::visitar(ExprVariable* nodo) {

}

void Emitter::visitar(ExprArray* nodo) {

}

void Emitter::visitar(ExprUnaria* nodo) {

}

void Emitter::visitar(ExprBinaria* nodo) {

}

void Emitter::visitar(ExprCasteo* nodo) {

}

void Emitter::visitar(ExprRango* nodo) {

}

void Emitter::visitar(ExprAcceso* nodo) {

}

void Emitter::visitar(ExprLlamadaArcano* nodo) {

}

// --- Sentencias --- //

void Emitter::visitar(Bloque* nodo) {

}

void Emitter::visitar(SentenciaVar* nodo) {

}

void Emitter::visitar(SentenciaExpr* nodo) {

}

void Emitter::visitar(SentenciaAsignacion* nodo) {

}

void Emitter::visitar(SentenciaSi* nodo) {

}

void Emitter::visitar(SentenciaSino* nodo) {

}

void Emitter::visitar(SentenciaMientras* nodo) {

}

void Emitter::visitar(SentenciaEscritura* nodo) {

}

void Emitter::visitar(SentenciaArcano* nodo) {

}

void Emitter::visitar(SentenciaLlamadaArcano* nodo) {

}
