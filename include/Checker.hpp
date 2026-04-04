// Checker.hpp

#pragma once

#include "Common.hpp"

class Checker : public ASTVisitor {
private:
  GestorTablas& tablas;
  std::vector<std::unique_ptr<Sentencia>>& ast;
  ErrorHandler& errHandler;
  TypeFactory& typeFactory;

public:
  Checker(GestorTablas& t, std::vector<std::unique_ptr<Sentencia>>& a, ErrorHandler& e, TypeFactory& tf);

  void verificarNodo(std::unique_ptr<Sentencia>& nodo);
  Dt verificarOperandos(const Dt& izq, const Dt& der, const TipoOperador op);
  void verificarPrograma();

  // --- Verificadores ---
  std::shared_ptr<ArcanaType> verificarSuma    (const Dt& izq, const Dt& der);
  std::shared_ptr<ArcanaType> verificarResta   (const Dt& izq, const Dt& der);
  std::shared_ptr<ArcanaType> verificarMult    (const Dt& izq, const Dt& der);
  std::shared_ptr<ArcanaType> verificarDiv     (const Dt& izq, const Dt& der);
  std::shared_ptr<ArcanaType> verificarPotencia(const Dt& izq, const Dt& der);

  bool esCasteoValido(const Dt& tipo_original, const Dt& tipo_destino);

  // --- AST Visitor ---

  void visitar(ExprNumero* nodo) override { //... Hacer más robusto
    Dt tipo;
    tipo.valor = typeFactory.getUnknown();

    bool tiene_punto = nodo->valor.contains('.');
    bool scientific  = nodo->valor.contains('e');

    char suf = ' ';
    int suf_num   ;

    if (!nodo->sufijo.empty()) { // [f/u/i][num]
      std::cout << "[43, Checker.hpp]: " << nodo->sufijo << '\n';
      suf     = nodo->sufijo[0];
    }

    if (nodo->sufijo.size() > 1) {
      suf_num = std::stoi(nodo->sufijo.substr(1));

    } else if (scientific) {
      suf_num = 64;

    } else {
      suf_num = -1; // Flag para "Elegir automáticamente"

    }

    if (!isPowerOf2(suf_num) || suf_num < 8 && suf_num != -1) {
      //... Error, el sufijo tiene que tener una potencia de 2 mayor o igual a 8
    }

    switch (suf) {
      case 'i':
      case 'u': {
        if (suf_num == -1) {
          suf_num = 32;
        }

        tipo.valor = typeFactory.getInteger(suf_num, (suf == 'u'));
        break;
      }

      case 'f': {
        if (suf_num == -1) {
          suf_num = 64;
        }

        tipo.valor = typeFactory.getFloat(suf_num);
        break;

      }

      case ' ': { // No tenía sufijo
        break;
      }

    }

    if (tipo.valor != typeFactory.getUnknown()) {
      nodo->tipo_resuelto = tipo;
      return ;
    }


    //... Añadir comprobaciones de tamaño
    if (tiene_punto || scientific) {
      tipo.valor = typeFactory.getFloat(64);

    } else {
      tipo.valor = typeFactory.getInteger(32, false);

    }

    nodo->tipo_resuelto = tipo;

  }

  void visitar(ExprBinaria* nodo) override {
    nodo->izquierda->accept(this);
    nodo->derecha  ->accept(this);

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
      nodo->tipo_resuelto = Dt(typeFactory.getUnknown());

    }
  }

  void visitar(ExprVariable* nodo) override { //...

  }

  void visitar(ExprArray* nodo) override {
    for (const auto& elemento : nodo->elementos) {
      elemento->accept(this);

    }

    //... Asignar el tipo de dato al array y comprobar que todos los tipos de datos dentro sean iguales
  }


  void visitar(ExprUnaria* nodo) override {
    nodo->operando->accept(this);
    //... Check if operando is a valid operand
  }

  void visitar(ExprLlamadaArcano* nodo) override {

  }

  void visitar(ExprRango* nodo) override {

    if (nodo->inicio) {
        nodo->inicio->accept(this);

    }

    if (nodo->fin    ) {
        nodo->fin   ->accept(this);

    }

    if (nodo->paso   ) {
        nodo->paso  ->accept(this);

    }

  }

  void visitar(ExprAcceso* nodo) override {
    nodo->contenedor->accept(this);
    nodo->rango->accept(this);

  }

  void visitar(Bloque* nodo) override {

  }

  void visitar(SentenciaVar* nodo) override { //... Implementar líneas en los NodoAST
    std::cout << "[188 Checker.hpp]\n";
    InfoVariable* info = tablas.buscarVariable(nodo->nombre, 0);
    std::cout << "[190 Checker.hpp]\n";
    nodo->valor_inicial->accept(this);
    std::cout << "[192 Checker.hpp]\n";

    if (info != nullptr) {
      //... Ya existe en algún scope
    } else {
      std::cout << "[197 Checker.hpp]\n";
      nodo->tipo_explicito.tipo.valor;
      std::cout << "[199 Checker.hpp]\n";
      nodo->valor_inicial->tipo_resuelto.valor;
      std::cout << "[201 Checker.hpp]\n";
      nodo->nombre;
      std::cout << "[203 Checker.hpp]\n";
      *info;
      std::cout << "[205 Checker.hpp]\n";

      if (nodo->tipo_explicito.tipo.valor != nodo->valor_inicial->tipo_resuelto.valor) {
        //... Los tipos no coinciden, hay que comprobar que el derecho quepa en el izquierdo
      } else { // Los tipos coinciden
        tablas.añadirVariable(nodo->nombre, *info, 0);
      }
      std::cout << "[212 Checker.hpp]\n";
    }

  }

  void visitar(SentenciaExpr* nodo) override {
    nodo->expresion->accept(this);
  }

  void visitar(SentenciaAsignacion* nodo) override {
    std::cout << "[212 Checker.hpp]\n";
    nodo->izquierda->accept(this);
    std::cout << "[214 Checker.hpp]\n";
    nodo->derecha  ->accept(this);
    std::cout << "[216 Checker.hpp]\n";

    //... Check if the left side and right side have the same type
  }

  void visitar(SentenciaSi* nodo) override { //...

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
