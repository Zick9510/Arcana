// Emitter.hpp

#pragma once

#include "Common.hpp"

class Emitter : public ASTVisitor {
private:
public:

  void visitar(ExprNumero  * nodo) override;
  void visitar(ExprVariable* nodo) override;
  void visitar(ExprArray   * nodo) override;

  void visitar(ExprUnaria * nodo) override;
  void visitar(ExprBinaria* nodo) override;
  void visitar(ExprCasteo * nodo) override;

  void visitar(ExprRango * nodo) override;
  void visitar(ExprAcceso* nodo) override;

  void visitar(ExprLlamadaArcano* nodo) override;


  void visitar(Bloque* nodo) override;
 
  void visitar(SentenciaVar * nodo) override;
  void visitar(SentenciaExpr* nodo) override;
 
  void visitar(SentenciaAsignacion* nodo) override;
 
  void visitar(SentenciaSi  * nodo) override;
  void visitar(SentenciaSino* nodo) override;
 
  void visitar(SentenciaMientras* nodo) override;
 
  void visitar(SentenciaEscritura* nodo) override;
 
  void visitar(SentenciaArcano       * nodo) override;
  void visitar(SentenciaLlamadaArcano* nodo) override;
};

