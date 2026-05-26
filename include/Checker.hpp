// Checker.hpp

#pragma once

#include "Common.hpp"

class Checker;

class TraitChecker {
private:
  Checker& checker;

public:
  TraitChecker(Checker& c);

};

class Checker : public ASTVisitor {
private:
  GestorTablas& tablas;
  std::vector<std::unique_ptr<Sentencia>>& ast;
  ErrorHandler& errHandler;
  TypeFactory& typeFactory;
  ContextoArcanos& contextoArcanos;

  std::map<std::string, Sentencia*> bloquesArcanoActivos;
  std::vector<SentenciaLlamadaArcano*> pilaLlamadasArcano;

  TraitChecker traits;

public:
  Checker(GestorTablas& t, std::vector<std::unique_ptr<Sentencia>>& a, ErrorHandler& e, TypeFactory& tf, ContextoArcanos& ca);

  void verificarNodo(std::unique_ptr<Sentencia>& nodo);
  Dt   verificarOperandos(const Dt& izq, const Dt& der, const TipoOperador op);
  void verificarPrograma();

  // --- Verificadores ---
  std::shared_ptr<ArcanaType> verificarSuma    (const Dt& izq, const Dt& der);
  std::shared_ptr<ArcanaType> verificarResta   (const Dt& izq, const Dt& der);
  std::shared_ptr<ArcanaType> verificarMult    (const Dt& izq, const Dt& der);
  std::shared_ptr<ArcanaType> verificarDiv     (const Dt& izq, const Dt& der);
  std::shared_ptr<ArcanaType> verificarPotencia(const Dt& izq, const Dt& der);

  std::shared_ptr<ArcanaType> verificarSwap    (const Dt& izq, const Dt& der);
  std::shared_ptr<ArcanaType> verificarTernary (const Dt& izq, const Dt& der);

  std::shared_ptr<ArcanaType> verificarCmpRelacional(const Dt& izq, const Dt& der);
  std::shared_ptr<ArcanaType> verificarCmpIgualdad  (const Dt& izq, const Dt& der);

  // --- Utilidad ---
  bool esCasteoValido(const Dt& tipo_original, const Dt& tipo_destino);
  std::unique_ptr<Expresion> forzarTipo(std::unique_ptr<Expresion> hijo, const Dt& tipoEsperado);

  inline std::string generarFirma(const std::string& nombre, const std::vector<Dt>& tiposArgs) {
    std::string firma = nombre;
    for (const auto& tipo : tiposArgs) {
      firma += "_" + tipo.tipoString();
    }
    return firma;
  }

  // --- AST Visitor ---

  void visitar(ErrorNode* nodo) override {}

  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
  void visitar(ExprLiteral* nodo) override { //...

    std::visit(overloaded {
      [&](NumberData& d) {
        bool tiene_punto = (d.valor.find('.') != std::string::npos);
        bool scientific  = (d.valor.find('e') != std::string::npos);

        char suf = ' ';
        int bits = -1 ;

        if (!d.sufijo.empty()) {
          suf = d.sufijo[0];
          if (d.sufijo.size() > 1) {
            bits = std::stoi(d.sufijo.substr(1)); // Should check if suffix has letters in positions 1..n
          }
        }

        if (bits != -1) {
          if (bits < 8 || !isPowerOf2(bits)) {
            //... Error: Sufijo inválido
          }
        }

        std::shared_ptr<ArcanaType> res;

        switch (suf) {
          case 'u':
          case 'i': {
            if (tiene_punto) {
              //... Error: Un literal entero can't have a decimal point
            }
            res = typeFactory.getInteger(bits == -1 ? 32 : bits, (suf == 'u'));
            break;
          }

          case 'f': {
            res = typeFactory.getFloat(bits == -1 ? 64 : bits);
            break;
          }

          default: { //... Añadir comprobación de overflow
            if (tiene_punto || scientific) {
              res = typeFactory.getFloat(64);

            } else {
              res = typeFactory.getInteger(32, false);

            }
            break;
          }
        }

        nodo->tipo_resuelto = Dt(res);
      },

      [&](BooleanData& d) {
        nodo->tipo_resuelto = Dt(typeFactory.getBoolean());
      },

      [&](CharData& d) {
        char suf = ' ';
        int bits = -1;

        if (!d.sufijo.empty()) {
          suf = d.sufijo[0];
          if (d.sufijo.size() > 1) {
            bits = std::stoi(d.sufijo.substr(1)); // Should check if suffix has letters in positions 1..n
          }
        }

        if (bits != -1) {
          if (bits < 8 || !isPowerOf2(bits)) {
            //... Error: Sufijo inválido
          }
        }

        std::shared_ptr<ArcanaType> res;

        switch (suf) {
          case 'c': {
            res = typeFactory.getChar(bits == -1 ? 8 : bits);
            break;
          }
          default: {
            res = typeFactory.getChar(8);
            break;
          }
        }

        nodo->tipo_resuelto = Dt(res);

      },

      [&](StringData& d) {},
      [&](RuleData& d) {}
    }, nodo->datos);

  }

  void visitar(ExprBinaria* nodo) override {
    nodo->izquierda->accept(this);
    nodo->derecha  ->accept(this);

    Dt tipo_izq = nodo->izquierda->tipo_resuelto;
    Dt tipo_der = nodo->derecha  ->tipo_resuelto;

    std::shared_ptr<ArcanaType> tipo_c = promoverTipos(tipo_izq.valor, tipo_der.valor);

    nodo->tipo_resuelto = verificarOperandos(tipo_izq, tipo_der, nodo->operador);

    if (nodo->tipo_resuelto.valor->kind != TypeKind::DESCONOCIDO) {
      nodo->izquierda = forzarTipo(std::move(nodo->izquierda), Dt(tipo_c));
      nodo->derecha   = forzarTipo(std::move(nodo->derecha)  , Dt(tipo_c));
    }

  }

  void visitar(ExprTernaria* nodo) override {
    nodo->condicion ->accept(this);
    nodo->rama_true ->accept(this);
    nodo->rama_false->accept(this);

    Dt tipo_izq = nodo->rama_true ->tipo_resuelto;
    Dt tipo_der = nodo->rama_false->tipo_resuelto;

    nodo->tipo_resuelto = verificarOperandos(tipo_izq, tipo_der, TipoOperador::TERNARY);

    if (nodo->tipo_resuelto.valor->kind != TypeKind::DESCONOCIDO) {
      nodo->rama_true  = forzarTipo(std::move(nodo->rama_true) , nodo->tipo_resuelto);
      nodo->rama_false = forzarTipo(std::move(nodo->rama_false), nodo->tipo_resuelto);
    }

  }

  void visitar(ExprCasteo* nodo) override {
    nodo->expresion->accept(this);

    Dt tipo_original = nodo->expresion->tipo_resuelto;
    Dt tipo_destino  = nodo->tipo_casteo;

    if (esCasteoValido(tipo_original, tipo_destino)) {
      nodo->tipo_resuelto = tipo_destino;

    } else {
      errHandler.report(CE::E_CASTEO_INVALIDO, nodo->pos);
      nodo->tipo_resuelto = Dt(typeFactory.getUnknown());

    }
  }

  //void visitar(ExprVariable* nodo) override { //...
  //  if (bloquesArcanoActivos.count(nodo->nombre)) {
  //    auto* bloque_fuente = bloquesArcanoActivos[nodo->nombre];
  //    auto bloque_conado = bloque_fuente->clonar();
  //  }
  //  //if (bloquesArcanoActivos.count(nodo->nombre)) {
  //  //  auto* bloque = bloquesArcanoActivos[nodo->nombre];
  //  //  bloque->accept(this);
  //  //  if (auto* s_expr = dynamic_cast<SentenciaExpr*>(bloque)) {
  //  //    nodo->tipo_resuelto = s_expr->expresion->tipo_resuelto;
  //  //  } else {
  //  //    nodo->tipo_resuelto = Dt(typeFactory.getUnknown());
  //  //  }
  //  //  return ;
  //  //}
  //  InfoVariable* info = tablas.buscarVariable(nodo->nombre);
  //  if (info != nullptr) {
  //    nodo->tipo_resuelto = info->tipo;
  //    return ;
  //  }
  //  std::cerr << "Error: Variable '" << nodo->nombre << "' no encontrada.\n";
  //}

  void visitar(ExprVariable* nodo) override {
    std::cout << "[Checker.hpp 243] ExprVariable\n";

    if (bloquesArcanoActivos.empty()) {
      std::cout << "[Checker.hpp 246] bloquesArcanoActivos vacío\n";

    } else {
      std::cout << "[Checker.hpp 249] bloquesArcanoActivos:\n";
      for (const auto& [key, val] : bloquesArcanoActivos) {
        std::cout << "[Checker.hpp 251] key: '" << key << "'\n";

      }
      std::cout << '\n';

    }

    if (bloquesArcanoActivos.count(nodo->nombre)) {
      auto* bloque_fuente = bloquesArcanoActivos[nodo->nombre];
      bloque_fuente->accept(this);

      if (auto* s_expr = dynamic_cast<SentenciaExpr*>(bloque_fuente)) {
        nodo->tipo_resuelto = s_expr->expresion->tipo_resuelto;

      } else {
        nodo->tipo_resuelto = Dt(typeFactory.getUnknown());

      }

      return ;

    }

    InfoVariable* info = tablas.buscarVariable(nodo->nombre);
    if (info != nullptr) {
      nodo->tipo_resuelto = info->tipo;
      return ;
    }

    std::cerr << "Error: Variable '" << nodo->nombre << "' no encontrada.\n";

  }


  void visitar(ExprArray* nodo) override {
    for (const auto& elemento : nodo->elementos) {
      elemento->accept(this);

    }

    //... Asignar el tipo de dato al array y comprobar que todos los tipos de datos dentro sean iguales
  }


  void visitar(ExprUnaria* nodo) override { //...
    nodo->operando->accept(this);
    Dt tipo_op = nodo->operando->tipo_resuelto;

    if (tipo_op.valor->kind == TypeKind::DESCONOCIDO) {
      nodo->tipo_resuelto = tipo_op;
      return ;

    }

    switch (nodo->operador) { //...
      case TipoOperador::LOGICO_NO: {
        Dt tipo_bool = Dt(typeFactory.getBoolean());
        nodo->tipo_resuelto = tipo_bool;

        if (tipo_op != tipo_bool) {
          if (esCasteoValido(tipo_op, tipo_bool)) {
            nodo->operando = forzarTipo(std::move(nodo->operando), tipo_bool);

          } else {
            //... Error casteo inválido
            std::cout << "[186, Checker.hpp] Error casteo inválido.";
            exit(1);

          }
        }

        nodo->tipo_resuelto = tipo_bool;
        break;
      }

      case TipoOperador::PTR_DEREF: {

        if (tipo_op.valor->kind != TypeKind::POINTER) {
          //... Error
          std::cout << "[214, Checker.hpp] Error de tipo en punteros\n";
          nodo->tipo_resuelto = Dt(typeFactory.getUnknown());
          break;

        }

        auto tipo_base = tipo_op.valor->getUnderlyingType();

        if (tipo_base == nullptr) {
          //... Error
          std::cout << "[223, Checker.hpp] Error, puntero es nullptr\n";
          nodo->tipo_resuelto = Dt(typeFactory.getUnknown());
          break;

        }

        nodo->tipo_resuelto = Dt(tipo_base);

        break;

      }

      case TipoOperador::PTR_REF: {

        if (!nodo->operando->isLValue()) {
          //... Error, no se pueden tomar direcciones de una expresión temporal
          std::cout << "[240, Checker.hpp] Error: Dirección de R-Value\n";
          nodo->tipo_resuelto = Dt(typeFactory.getUnknown());
          break;

        }

        auto tipo_puntero = typeFactory.getPointer(tipo_op.valor);
        nodo->tipo_resuelto = Dt(tipo_puntero);
        break;

      }
    }
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

    tablas.entrarScope();

    for (auto& inst : nodo->instrucciones) {
      inst->accept(this);

    }

    tablas.salirScope();

  }

  void visitar(SentenciaAsignarVar* nodo) override {
    if (nodo->valor_inicial) {
      nodo->valor_inicial->accept(this);

      Dt tipo_destino = nodo->tipo_explicito.tipo;

      nodo->valor_inicial = forzarTipo(std::move(nodo->valor_inicial), tipo_destino);

    }

    InfoVariable* info = tablas.buscarVariable(nodo->nombre);
    if (info == nullptr) { // Si la variable no existe, creamos una
      InfoVariable nueva_info;
      nueva_info.tipo = nodo->tipo_explicito.tipo.valor;
      tablas.añadirVariable(nodo->nombre, nueva_info);
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

  void visitar(SentenciaReasignacionVar* nodo) override {
    nodo->izquierda->accept(this);
    nodo->derecha  ->accept(this);

    //... Check if the left side and right side have the same type
  }

  void visitar(SentenciaSi* nodo) override {
    nodo->condicion->accept(this);
    nodo->condicion = forzarTipo(std::move(nodo->condicion), Dt(typeFactory.getBoolean()));

    nodo->rama_si  ->accept(this);

    if (nodo->rama_sino) {
      nodo->rama_sino->accept(this);

    }

  }

  void visitar(SentenciaSino* nodo) override {
    nodo->cuerpo->accept(this);

  }

  void visitar(SentenciaMientras* nodo) override {
    nodo->condicion ->accept(this);
    nodo->condicion = forzarTipo(std::move(nodo->condicion), Dt(typeFactory.getBoolean()));

    nodo->rama_while->accept(this);

    if (nodo->rama_sino) {
      nodo->rama_sino->accept(this);

    }

  }

  void visitar(SentenciaBreak* nodo) override { //...

  }

  void visitar(SentenciaContinue* nodo) override { //...

  }

  void visitar(SentenciaRedo* nodo) override { //...

  }

  void visitar(SentenciaReturn* nodo) override { //...
    if (!nodo->ret_value) { return; }

    nodo->ret_value->accept(this);

    InfoFuncion* info = tablas.getCurrentFunction();

    if (info) {
      nodo->ret_value = forzarTipo(std::move(nodo->ret_value), info->tipo_retorno);
      nodo->ret_type.valor = nodo->ret_value->tipo_resuelto.valor;

    } else {
      std::cout << "[325, Checker.hpp] Bad Info"; //...
      exit(1);

    }

  }

  void visitar(SentenciaFuncDecl* nodo) override {
    std::vector<Dt> tipos_params;
    for (const auto& [nombre, info] : nodo->args_type) {
      tipos_params.push_back(info.tipo);
    }

    std::string firma = generarFirma(nodo->nombre_func, tipos_params);
    nodo->firma_mangled = firma;

    InfoFuncion info_func;
    info_func.nombre = nodo->nombre_func;
    info_func.tipo_retorno = nodo->ret_type;
    info_func.tipos_parametros = nodo->args_type;

    if (!tablas.añadirFunction(firma, info_func)) {
      std::cout << "[338, Checker.hpp] Error: Función redefinida\n";
      exit(1);

    }

    InfoFuncion* ptr_func = tablas.buscarFunction(firma);
    tablas.pushFunction(ptr_func);
    tablas.entrarScope();

    for (const auto& [nombre, info] : nodo->args_type) {
      tablas.añadirVariable(nombre, info);
    }

    if (!nodo->cuerpo_func.empty()) {

      for (auto& inst : nodo->cuerpo_func) {
        inst->accept(this);
      }

    }

    tablas.salirScope();
    tablas.popFunction();

  }

  void visitar(SentenciaEscritura* nodo) override {

  }

  void visitar(SentenciaArcano* nodo) override {
    ArcaneDef& def = contextoArcanos.buscarDefinicionPorNombre(nodo->def.name);

    def = nodo->def;

  }

  void visitar(SentenciaLlamadaArcano* nodo) override {

    pilaLlamadasArcano.push_back(nodo);

    ArcaneDef& def     = contextoArcanos.buscarDefinicionPorKeyword(nodo->nombre);
    ArcaneBranch* rama = &def.branches[nodo->indice_rama];

    tablas.entrarScope();

    for (const auto& [nombre_arg, ast_arg] : nodo->args) {
      ast_arg->accept(this);

      InfoVariable info;

      if (auto* s_expr = dynamic_cast<SentenciaExpr*>(ast_arg.get())) {
        info.tipo = s_expr->expresion->tipo_resuelto;
      } else {
        info.tipo = Dt(typeFactory.getUnknown());
      }

      tablas.añadirVariable(nombre_arg, info);

    }

    auto backup_bloques = bloquesArcanoActivos;

    for (const auto& [nombre_arg, ast_arg] : nodo->expr) {
      bloquesArcanoActivos[nombre_arg] = ast_arg.get();
    }

    for (const auto& [nombre_arg, ast_arg] : nodo->code) {
      bloquesArcanoActivos[nombre_arg] = ast_arg.get();
    }

    for (const auto& seg : rama->segmentos) {
      if (seg.br_cont) {
        seg.br_cont->accept(this);
      }
    }

    //for (const auto& c : nodo->chains) {
    //  c->accept(this);
    //}

    auto bloque_expandido = std::make_unique<Bloque>();
    for (const auto& seg : rama->segmentos) {
      if (seg.br_cont) {
        bloque_expandido->instrucciones.push_back(seg.br_cont->clonar());
      }
    }

    bloque_expandido->accept(this);

    bloquesArcanoActivos = backup_bloques;
    tablas.salirScope();

    pilaLlamadasArcano.pop_back();

  }

  //void visitar(SentenciaMetaDirective* nodo) override { //...
  //  switch (nodo->id) {
  //    case MetaID::CHAIN: {
  //      if (nodo->args.size() != 1) {
  //        throw std::runtime_error("Error: ?chain espera exactamente 1 argumento.");
  //      }
  //      auto* arg_literal = dynamic_cast<ExprLiteral*>(nodo->args[0].get());
  //      if (!arg_literal || !std::holds_alternative<RuleData>(arg_literal->datos)) {
  //        throw std::runtime_error("Error: El argumento de ?chain deber ser una regla.");
  //      }
  //      if (!flagMetaDirectivas) {
  //        if (nodo->body) { nodo->body->accept(this); }
  //        return ;
  //      }
  //      std::string target_rule = std::get<RuleData>(arg_literal->datos).rule;
  //      if (pilaLlamadasArcano.empty()) {
  //        throw std::runtime_error("Error: ?chain solo puede usarse dentro del cuerpo de un arcano.");
  //      }
  //      SentenciaLlamadaArcano* llamada_actual = pilaLlamadasArcano.back();
  //      SentenciaLlamadaArcano* nodo_cadena    = nullptr;
  //      for (const auto& cadena : llamada_actual->chains) {
  //        std::cout << "[596 Checker.hpp] " << cadena->rule_tag << ' ' << target_rule << '\n';
  //        if (cadena->rule_tag == target_rule) {
  //          nodo_cadena = cadena.get();
  //          break;
  //        }
  //      }
  //      if (nodo_cadena) {
  //        pilaLlamadasArcano.push_back(nodo_cadena);
  //        auto backup_bloques = bloquesArcanoActivos;
  //        for (const auto& [nombre_arg, ast_arg] : nodo_cadena->expr) {
  //          bloquesArcanoActivos[nombre_arg] = ast_arg.get();
  //        }
  //        for (const auto& [nombre_arg, ast_arg] : nodo_cadena->code) {
  //          bloquesArcanoActivos[nombre_arg] = ast_arg.get();
  //        }
  //        if (currentPtr && nodo->body) {
  //          std::cout << "[651 Checker.hpp]\n";
  //          auto clon = nodo->body->clonar();
  //          *currentPtr = std::move(clon);
  //          (*currentPtr)->accept(this);
  //        }
  //        bloquesArcanoActivos = backup_bloques;
  //        pilaLlamadasArcano.pop_back();
  //      } else if (currentPtr) {
  //        *currentPtr = std::make_unique<Bloque>();
  //      }
  //      break;
  //    }
  //    default: { break; }
  //  }
  //}

  void visitar(SentenciaMetaDirective* nodo) override {

    switch (nodo->id) {
      case MetaID::CHAIN: {
        if (nodo->args.size() != 1) {
          throw std::runtime_error("Error: ?chain esperaba 1 argumento");
        }

        if (pilaLlamadasArcano.empty()) {
          throw std::runtime_error("Error: ?chain solo puede usarse dentro del cuerpo de un arcano.");
        }

        auto* arg_literal = dynamic_cast<ExprLiteral*>(nodo->args[0].get());
        if (!arg_literal || !std::holds_alternative<RuleData>(arg_literal->datos)) {
          throw std::runtime_error("Error: El argumento de ?chain debe ser una regla");
        }

        std::string target_rule = std::get<RuleData>(arg_literal->datos).rule;

        SentenciaLlamadaArcano* llamada_actual = pilaLlamadasArcano.back();
        SentenciaLlamadaArcano* nodo_cadena    = nullptr;

        for (const auto& cadena : llamada_actual->chains) {
          if (cadena->rule_tag == target_rule) {
            nodo_cadena = cadena.get();
            break;
          }
        }

        if (nodo->body && nodo_cadena) {
          pilaLlamadasArcano.push_back(nodo_cadena);
          auto backup_bloques = bloquesArcanoActivos;

          for (const auto& [nombre_arg, ast_arg] : nodo_cadena->expr) {
            bloquesArcanoActivos[nombre_arg] = ast_arg.get();
          }

          for (const auto& [nombre_arg, ast_arg] : nodo_cadena->code) {
            bloquesArcanoActivos[nombre_arg] = ast_arg.get();
          }

          nodo->body->accept(this);
          bloquesArcanoActivos = backup_bloques;
          pilaLlamadasArcano.pop_back();

        } else if (nodo->body) {
          nodo->body->accept(this);

        }


      }
      default: { break; }
    }

  }

};
