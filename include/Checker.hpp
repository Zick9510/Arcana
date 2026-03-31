// Checker.hpp

#pragma once

#include "Common.hpp"

class Checker : public ASTVisitor {
  private:
    GestorTablas tablas;
    std::vector<std::unique_ptr<Sentencia>>& ast;
    ErrorHandler& errHandler;

  public:
    Checker(GestorTablas t, std::vector<std::unique_ptr<Sentencia>>& a, ErrorHandler& e);

    void verificarNodo(std::unique_ptr<Sentencia>& nodo);
    Dt verificarOperandos(const Dt& izq, const Dt& der, const TipoOperador op);
    void verificarPrograma();

    // --- Verificadores ---
    Dt verificarSuma(const Dt& izq, const Dt& der);
    Dt verificarResta(const Dt& izq, const Dt& der);
    Dt verificarMult(const Dt& izq, const Dt& der);

    bool esCasteoValido(const Dt& tipo_original, const Dt& tipo_destino);

    // --- AST Visitor ---

    void visitar(ExprNumero* nodo) override { //... Hacer más robusto
      TipoPrimitivo tipo = TipoPrimitivo::DESCONOCIDO;

      bool tiene_punto = nodo->valor.contains('.');

      char suf = ' ';

      if (!nodo->sufijo.empty()) {
        suf = nodo->sufijo[0];
      }

      if (tiene_punto && suf != 'f') {
        //... Error, se esperaba que un float tenga decimal
      }

      switch (suf) { //... Cambiar para comprobar que sea potencia de 2 y simplificar el enum para poder
                     // construir literales tan grandes como C++ permita

        case 'i': {
          if        (nodo->sufijo == "i32" ) {
            tipo = TipoPrimitivo::INT      ;

          } else if (nodo->sufijo == "i64" ) {
            tipo = TipoPrimitivo::LONG     ;

          } else if (nodo->sufijo == "i128") {
            tipo = TipoPrimitivo::VERY_LONG;

          } else if (nodo->sufijo == "i256") {
            tipo = TipoPrimitivo::FULL_LONG;

          } else                             {
            //... Sufijo inexistente

          }

          break;
        }

        case 'f': {
          if        (nodo->sufijo == "f32" ) {
            tipo = TipoPrimitivo::FLOAT      ;

          } else if (nodo->sufijo == "f64" ) {
            tipo = TipoPrimitivo::DOUBLE     ;

          } else if (nodo->sufijo == "f128") {
            tipo = TipoPrimitivo::LONG_DOUBLE;

          } else                             {
            //... Sufijo inexistente

          }

          break;
        }

        case 's': {
          if (nodo->sufijo != "s16") {
            //... Sufijo inexistente

          } else                     {
            tipo = TipoPrimitivo::SHORT;

          }

          break;
        }

        case 'u': { //... Parsear sufijo u

          break;
        }

        case ' ': { break; } // No tenía sufijo

        default: {
          //... Sufijo inexistente
          break;
        }
      }

      if (tipo != TipoPrimitivo::DESCONOCIDO) {
        nodo->tipo_resuelto = tipo;
        return ;

      }

      //... Añadir comprobaciones de tamaño

      if (tiene_punto) {
        tipo = TipoPrimitivo::DOUBLE;

      } else {
        tipo = TipoPrimitivo::INT;

      }

      nodo->tipo_resuelto = tipo;

      return ;

      nodo->tipo_resuelto = TipoPrimitivo::DESCONOCIDO;

    }

    void visitar(ExprBinaria* nodo) override {
      nodo->izquierda->accept(this);
      nodo->derecha->accept(this);

      Dt tipo_izq = nodo->izquierda->tipo_resuelto;
      Dt tipo_der = nodo->derecha  ->tipo_resuelto;

      nodo->tipo_resuelto = verificarOperandos(tipo_izq, tipo_der, nodo->operador);
    }

    void visitar(ExprCasteo* nodo) override {
      nodo->expresion->accept(this);

      Dt tipo_original = nodo->expresion->tipo_resuelto;
      Dt tipo_destino  = nodo->tipo_casteo;

      if (esCasteoValido(tipo_original, tipo_destino)) {
        nodo->tipo_resuelto = tipo_destino;

      } else {
        errHandler.reportar(CE::ERR_CASTEO_INVALIDO, nodo->linea, {});
        nodo->tipo_resuelto = Dt(TipoPrimitivo::DESCONOCIDO);

      }
    }

    void visitar(ExprVariable* nodo) override { //...
      nodo->tipo_resuelto = Dt(TipoPrimitivo::INT);
    }

    void visitar(ExprArray* nodo) override {

    }

    void visitar(ExprUnaria* nodo) override {

    }

    void visitar(ExprLlamadaArcano* nodo) override {

    }

    void visitar(ExprRango* nodo) override {

    }

    void visitar(ExprAcceso* nodo) override {

    }

    void visitar(Bloque* nodo) override {

    }

    void visitar(SentenciaVar* nodo) override {
      //... Chequear la var
      nodo->valor_inicial->accept(this);
    }

    void visitar(SentenciaExpr* nodo) override {
      nodo->expresion->accept(this);
    }

    void visitar(SentenciaAsignacion* nodo) override {

    }

    void visitar(SentenciaSi* nodo) override {

    }

    void visitar(SentenciaSino* nodo) override {

    }

    void visitar(SentenciaMientras* nodo) override {

    }

    void visitar(SentenciaEscritura* nodo) override {

    }

    void visitar(SentenciaArcano* nodo) override {

    }

    void visitar(SentenciaLlamadaArcano* nodo) override {

    }
};
