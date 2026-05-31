// Emitter.hpp

#pragma once

#include "Common.hpp"

class Emitter;

class TraitEmitter {
private:
  Emitter& emitter;

public:
  TraitEmitter(Emitter& e);

  void despacharTrait(Bloque* nodo, size_t idx);

  void handleLoop(Bloque* nodo, size_t idx);
  void handleNoscope(Bloque* nodo, size_t idx);

};

class Emitter : public ASTVisitor {
private:
  llvm::LLVMContext                                      llvmCtx            ;
  std ::unique_ptr<llvm::Module>                         llvmModulo         ;
  std ::unique_ptr<llvm::IRBuilder<>>                    llvmBuilder        ;
  llvm::Value*                                           llvmValor = nullptr;
  std::map<std::string, llvm::StructType*>               llvmStructs        ;

  std::string structActual = "";
  llvm::StructType* llvmStructActual = nullptr;
  llvm::Value* llvmThis = nullptr;

  std::vector<llvm::BasicBlock*> pilaBreaks   ;
  std::vector<llvm::BasicBlock*> pilaContinues;
  std::vector<llvm::BasicBlock*> pilaRedos    ;

  bool enScopeGlobal = true;

  ContextoArcanos& contextoArcanos;
  GestorTablas&    tablas         ;
  TraitEmitter     traits         ;
  friend class TraitEmitter;

  std::map<std::string,    Sentencia*> bloquesArcanoActivos;
  std::map<std::string,    Expresion*> varsArcanosActivos  ;
  std::vector<SentenciaLlamadaArcano*> stackArcanos        ;

public:

  Emitter(ContextoArcanos& ca, GestorTablas& t);

  llvm::Type*              obtenerTipoLLVM    (std::shared_ptr<ArcanaType> tipo);
  llvm::Value*             obtenerPuntero     (Expresion* nodo);
  llvm::CmpInst::Predicate obtenerPredicadoCmp(TipoOperador op, bool esFloat, bool esSigned);

  void generarArchivoIR(const std::filesystem::path& nombreArchivo);

  void visitar(ErrorNode* nodo) override;

  void visitar(ExprLiteral * nodo) override;
  void visitar(ExprVariable* nodo) override;
  void visitar(ExprArray   * nodo) override;

  void visitar(ExprUnaria  * nodo) override;
  void visitar(ExprBinaria * nodo) override;
  void visitar(ExprTernaria* nodo) override;

  void visitar(ExprCasteo* nodo) override;

  void visitar(ExprRango      * nodo) override;
  void visitar(ExprAcceso     * nodo) override;
  void visitar(ExprAccesoPunto* nodo) override;

  void visitar(ExprFuncCall* nodo) override;

  void visitar(ExprInitList* nodo) override;

  void visitar(Bloque* nodo) override;
 
  void visitar(SentenciaAsignarVar * nodo) override;
  void visitar(SentenciaExpr* nodo) override;
 
  void visitar(SentenciaReasignacionVar* nodo) override;
 
  void visitar(SentenciaSi  * nodo) override;
  void visitar(SentenciaSino* nodo) override;
 
  void visitar(SentenciaMientras* nodo) override;

  void visitar(SentenciaBreak   * nodo) override;
  void visitar(SentenciaContinue* nodo) override;
  void visitar(SentenciaRedo    * nodo) override;

  void visitar(SentenciaReturn  * nodo) override;
  void visitar(SentenciaFuncDecl* nodo) override;

  void visitar(SentenciaStruct* nodo) override;
 
  void visitar(SentenciaEscritura* nodo) override;
 
  void visitar(SentenciaArcano       * nodo) override;
  void visitar(SentenciaLlamadaArcano* nodo) override;
  void visitar(SentenciaMetaDirective* nodo) override;

};

