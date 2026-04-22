// Emitter.hpp

#pragma once

#include "Common.hpp"

class Emitter : public ASTVisitor {
private:
  llvm::LLVMContext                                      llvm_ctx            ;
  std ::unique_ptr<llvm::Module>                         llvm_modulo         ;
  std ::unique_ptr<llvm::IRBuilder<>>                    llvm_builder        ;
  llvm::Value*                                           llvm_valor = nullptr;
  std ::vector<std::map<std::string, llvm::AllocaInst*>> llvm_scopes         ;

  std::vector<llvm::BasicBlock*> pila_breaks   ;
  std::vector<llvm::BasicBlock*> pila_continues;

  ContextoArcanos& contextoArcanos;

  std::map<std::string, Sentencia*> bloques_arcano_activos;

public:

  Emitter(ContextoArcanos& ca);

  void generarArchivoIR(const std::filesystem::path& nombreArchivo);
  llvm::Type* obtenerTipoLLVM(std::shared_ptr<ArcanaType> tipo);

  void visitar(ExprNumero  * nodo) override;
  void visitar(ExprVariable* nodo) override;
  void visitar(ExprArray   * nodo) override;

  void visitar(ExprUnaria * nodo) override;
  void visitar(ExprBinaria* nodo) override;
  void visitar(ExprCasteo * nodo) override;

  void visitar(ExprRango * nodo) override;
  void visitar(ExprAcceso* nodo) override;

  void visitar(ExprFuncCall* nodo) override;

  void visitar(Bloque* nodo) override;
 
  void visitar(SentenciaAsignarVar * nodo) override;
  void visitar(SentenciaExpr* nodo) override;
 
  void visitar(SentenciaReasignacionVar* nodo) override;
 
  void visitar(SentenciaSi  * nodo) override;
  void visitar(SentenciaSino* nodo) override;
 
  void visitar(SentenciaMientras* nodo) override;

  void visitar(SentenciaBreak   * nodo) override;
  void visitar(SentenciaContinue* nodo) override;

  void visitar(SentenciaReturn  * nodo) override;
  void visitar(SentenciaFuncDecl* nodo) override;
 
  void visitar(SentenciaEscritura* nodo) override;
 
  void visitar(SentenciaArcano       * nodo) override;
  void visitar(SentenciaLlamadaArcano* nodo) override;

};

