// Emitter.hpp

#pragma once

#include "Common.hpp"

#include <string>

class Emitter : public ASTVisitor {
  private:
    std::string codigo;

  public:
    std::string obtenerCodigo() const { return codigo; }

    // --- Expresiones --- //
 
    void visitar(ExprNumero* nodo) override {
      codigo += nodo->valor;
    }

    void visitar(ExprVariable* nodo) override {
      codigo += nodo->nombre;
    }

    void visitar(ExprArray* nodo) override { //... Array

    }

    void visitar(ExprUnaria* nodo) override { //...
      codigo += "(";
      if (nodo->es_prefijo) {
        codigo += operadorString(nodo->operador);
        nodo->operando->accept(this);
      } else {
        nodo->operando->accept(this);
        codigo += operadorString(nodo->operador);
      }
      codigo += ")";
    }

    void visitar(ExprBinaria* nodo) override {
      codigo += "(";
      nodo->izquierda->accept(this);
      codigo += " " + operadorString(nodo->operador) + " ";
      nodo->derecha->accept(this);
      codigo += ")";
    }

    void visitar(ExprCasteo* nodo) override {
      std::cerr << "Error: No implementado (ExprCasteo)\n";
    }

    void visitar(ExprRango* nodo) override {
      std::cerr << "Error: No implementado (ExprRango)\n";
    }

    void visitar(ExprAcceso* nodo) override {
      std::cerr << "Error: No implementado (ExprAcceso)\n";
    }

    void visitar(ExprLlamadaArcano* nodo) override {
      std::cerr << "Error: No implementado (ExprLlamadaArcano)\n";
    }

    // --- Sentencias --- //

    void visitar(Bloque* nodo) override {
      codigo += "{\n";
      for (const auto& sent : nodo->instrucciones) {
        codigo += "    ";
        sent->accept(this);
      }
      codigo += "}\n";
    }

    void visitar(SentenciaExpr* nodo) override {
      nodo->expresion->accept(this);
      codigo += ";\n";
    }

    void visitar(SentenciaVar* nodo) override {
      codigo += nodo->tipo_explicito + " " + nodo->nombre;
      if (nodo->valor_inicial) {
        codigo += " = ";
        nodo->valor_inicial->accept(this);
      }
      codigo += ";\n";
    }

    void visitar(SentenciaAsignacion* nodo) override {
      nodo->izquierda->accept(this);
      codigo += " = ";
      nodo->derecha->accept(this);
      codigo += ";\n";
    }

    void visitar(SentenciaSi* nodo) override {
      codigo += "if (";
      nodo->condicion->accept(this);
      codigo += ") ";
      nodo->rama_si->accept(this);

      if (nodo->rama_sino) {
        codigo += "else ";
        nodo->rama_sino->accept(this);
      }
    }

    void visitar(SentenciaSino* nodo) override {
      codigo += "else ";
      nodo->cuerpo->accept(this);
    }

    void visitar(SentenciaMientras* nodo) override {
      codigo += "while (";
      nodo->condicion->accept(this);
      codigo +=  ") ";
      nodo->rama_while->accept(this);
    }

    void visitar(SentenciaEscritura* nodo) override {
      std::cerr << "Error: No implementado (SentenciaEscritura)\n";
    }

    void visitar(SentenciaArcano* nodo) override {
      std::cerr << "Error: No implementado (SentenciaArcano)\n";
    }

    void visitar(SentenciaLlamadaArcano* nodo) override {
      std::cerr << "Error: No implementado (SentenciaLlamadaArcano)\n";
    }
};
