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

    if (tiene_punto) {
      if (suf != 'f' && suf != ' ') {
        //... Error, tiene punto y el sufijo no es float
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
    InfoVariable* info = tablas.buscarVariable(nodo->nombre);

    if (info != nullptr) {
      nodo->tipo_resuelto = info->tipo;

    } else {
      //... La variable no existe

    }

  }

  void visitar(ExprArray* nodo) override {
    for (const auto& elemento : nodo->elementos) {
      elemento->accept(this);

    }

    //... Asignar el tipo de dato al array y comprobar que todos los tipos de datos dentro sean iguales
  }


  void visitar(ExprUnaria* nodo) override {
    nodo->operando->accept(this);
    nodo->tipo_resuelto = nodo->operando->tipo_resuelto;
    //... Check if operando is a valid operand
    //... Check if type is unsigned
  }

  void visitar(ExprLlamadaArcano* nodo) override {

  }

  void visitar(ExprRango* nodo) override {

    if (nodo->inicio) {
        nodo->inicio->accept(this);

    }

    if (nodo->fin   ) {
        nodo->fin   ->accept(this);

    }

    if (nodo->paso  ) {
        nodo->paso  ->accept(this);

    }

  }

  void visitar(ExprAcceso* nodo) override {
    nodo->contenedor->accept(this);
    nodo->rango->accept(this);

  }

  void visitar(Bloque* nodo) override {
    for (const auto& i : nodo->instrucciones) {
      i->accept(this);
    }
  }

  void visitar(SentenciaVar* nodo) override { //... Implementar líneas en los NodoAST
    if (nodo->valor_inicial) { nodo->valor_inicial->accept(this); }

    InfoVariable* info = tablas.buscarVariable(nodo->nombre);
    if (info == nullptr) { // Si la variable no existe, creamos una
      InfoVariable nueva_info;
      nueva_info.tipo = nodo->tipo_explicito.tipo.valor;
      tablas.añadirVariable(nodo->nombre, nueva_info, 0);
      return ;

    } else { // Si existe, error

    }

  }

  void visitar(ExprFuncCall* nodo) override { //...
    //... Params, for now, we just blindly assume the output is an integer
    nodo->tipo_resuelto.valor = typeFactory.getInteger(32, false);

    for (const auto& n : nodo->argumentos) {
      n.second->accept(this);
    }
  }

  void visitar(SentenciaExpr* nodo) override {
    nodo->expresion->accept(this);
  }

  void visitar(SentenciaAsignacion* nodo) override {
    nodo->izquierda->accept(this);
    nodo->derecha  ->accept(this);

    //... Check if the left side and right side have the same type
  }

  void visitar(SentenciaSi* nodo) override { //...

  }

  void visitar(SentenciaSino* nodo) override {

  }

  void visitar(SentenciaMientras* nodo) override {

  }

  void visitar(SentenciaReturn* nodo) override { //...
    nodo->ret_type.valor = typeFactory.getUnknown(); //...
    nodo->ret_value->accept(this);

  }

  void visitar(SentenciaFuncDecl* nodo) override { //...
    tablas.entrarScope();

    for (const auto& [nombre, info] : nodo->args_type) {
      tablas.añadirVariable(nombre, info, 0);
    }

    if (nodo->cuerpo_func != nullptr) {
      nodo->cuerpo_func->accept(this);
    }

    tablas.salirScope();

  }

  void visitar(SentenciaEscritura* nodo) override {

  }

  void visitar(SentenciaArcano* nodo) override {

  }

  void visitar(SentenciaLlamadaArcano* nodo) override {

  }
};
