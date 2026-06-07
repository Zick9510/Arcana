// ConstEval.hpp

#pragma once

#include "Common.hpp"

class ConstantEvaluator : public ASTVisitor {
private:
  LiteralData result;
  bool success = true;

  double getNumericValue(const LiteralData& data) {
    return std::visit([](const auto& arg) -> double {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, NumberData>) {
        return std::stod(arg.valor);
      }
      if constexpr (std::is_same_v<T, BooleanData>) {
        return arg.valor == "true" ? 1.0 : 0.0;
      }
      return 0.0;
    }, data);
  }

  bool getBooleanValue(const LiteralData& data) {
    return std::visit([](const auto& arg) -> bool {
      using T = std::decay_t<decltype(arg)>;
      if constexpr (std::is_same_v<T, BooleanData>) {
        return arg.valor == "true";
      }
      if constexpr (std::is_same_v<T, NumberData>) {
        return std::stod(arg.valor) != 0.0;
      }
      return false;
    }, data);
  }

public:
  ConstantEvaluator() = default;

  std::optional<LiteralData> eval(Expresion* expr) {
    success = true;
    if (!expr) { return  std::nullopt; }
    expr->accept(this);
    if (success) { return result; }
    return std::nullopt;
  }

  void visitar(ErrorNode* nodo) override { success = false; }
  void visitar(ExprLiteral* nodo) override { result = nodo->datos; }
  void visitar(ExprBinaria* nodo) override {
    nodo->izquierda->accept(this);
    if (!success) { return ; }
    LiteralData val_izq = result;

    nodo->derecha->accept(this);
    if (!success) { return ; }
    LiteralData val_der = result;

    double izq = getNumericValue(val_izq);
    double der = getNumericValue(val_der);
    double res = 0.0;
    bool res_bool = false;
    bool is_relational = false;

    switch (nodo->operador) { //...
      case TipoOperador::SUMA : { res = izq + der; break; }
      case TipoOperador::RESTA: { res = izq - der; break; }
      default: { success = false; break; }
    }

    if (is_relational) {
      result = BooleanData{ res_bool ? "true" : "false" };

    } else {
      if (res == static_cast<long long>(res)) {
        result = NumberData{ std::to_string(static_cast<long long>(res)), ""};

      } else {
        result = NumberData{ std::to_string(res), ""};

      }
    }

  }

  void visitar(ExprUnaria* nodo) override {
    nodo->operando->accept(this);
    if (!success) { return ; }
    LiteralData val = result;

    switch (nodo->operador) {
      case TipoOperador::RESTA: {
        double v = getNumericValue(val);
        result = NumberData{ std::to_string(-v), ""};
        break;
      }

      case TipoOperador::LOGICO_NO: {
        bool b = getBooleanValue(val);
        result = BooleanData{ !b ? "true" : "false" };
        break;
      }

      default: {
        success = false;
        break;
      }
    }
  }

  void visitar(ExprTernaria* nodo) override {
    nodo->condicion->accept(this);
    if (!success) { return ; }
    bool cond = getBooleanValue(result);
    cond ? nodo->rama_true->accept(this) : nodo->rama_false->accept(this);
  }

  void visitar(ExprCasteo* nodo)               override { nodo->expresion->accept(this); }
  void visitar(ExprVariable* nodo)             override { success = false; }
  void visitar(ExprRango* nodo)                override { success = false; }
  void visitar(ExprAcceso* nodo)               override { success = false; }
  void visitar(ExprAccesoPunto* nodo)          override { success = false; }
  void visitar(ExprFuncCall* nodo)             override { success = false; }
  void visitar(ExprInitList* nodo)             override { success = false; }
  void visitar(ExprArray* nodo)                override { success = false; }
  void visitar(Bloque* nodo)                   override { success = false; }
  void visitar(SentenciaAsignarVar* nodo)      override { success = false; }
  void visitar(SentenciaExpr* nodo)            override { success = false; }
  void visitar(SentenciaReasignacionVar* nodo) override { success = false; }
  void visitar(SentenciaSi* nodo)              override { success = false; }
  void visitar(SentenciaSino* nodo)            override { success = false; }
  void visitar(SentenciaMientras* nodo)        override { success = false; }
  void visitar(SentenciaBreak* nodo)           override { success = false; }
  void visitar(SentenciaContinue* nodo)        override { success = false; }
  void visitar(SentenciaRedo* nodo)            override { success = false; }
  void visitar(SentenciaReturn* nodo)          override { success = false; }
  void visitar(SentenciaFuncDecl* nodo)        override { success = false; }
  void visitar(SentenciaStruct* nodo)          override { success = false; }
  void visitar(SentenciaEscritura* nodo)       override { success = false; }
  void visitar(SentenciaArcano* nodo)          override { success = false; }
  void visitar(SentenciaLlamadaArcano* nodo)   override { success = false; }
  void visitar(SentenciaMetaDirective* nodo)   override { success = false; }
  void visitar(SentenciaTemplate* nodo)        override { success = false; }
};
