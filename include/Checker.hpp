// Checker.hpp

#pragma once

#include "Common.hpp"

#include "ConstEval.hpp"

class Checker;

class TraitChecker {
private:
  Checker& checker;

public:
  TraitChecker(Checker& c);

  void despacharTrait(Bloque* nodo, size_t idx);

  void handleNoscope(Bloque* nodo, size_t idx);

};

enum class ModoChecker {
  REGISTRO,
  VERIFICACION
};

class Checker : public ASTVisitor {
private:
  GestorTablas& tablas;
  std::vector<std::unique_ptr<Sentencia>>& ast;
  ErrorHandler& errHandler;
  TypeFactory& typeFactory;
  ContextoArcanos& contextoArcanos;

  std::map<std::string,    Sentencia*> bloquesArcanoActivos;
  std::map<std::string,    Expresion*> varsArcanosActivos  ;
  std::vector<SentenciaLlamadaArcano*> pilaLlamadasArcano  ;

  TraitChecker traits;
  friend class TraitChecker;

  ModoChecker mode = ModoChecker::REGISTRO;
  std::string structActual = "";


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
  inline std::string getDunder(TipoOperador op) { //...
    switch (op) {
      case TipoOperador::SUMA: { return dunder::ADD; }
      default: { return ""; }
    }
  }

  bool esCasteoValido(const Dt& tipo_original, const Dt& tipo_destino);
  std::unique_ptr<Expresion> forzarTipo(std::unique_ptr<Expresion> hijo, const Dt& tipoEsperado);

  // --- AST Visitor ---

  // Expresiones

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

    std::string dunder = getDunder(nodo->operador);

    if (!dunder.empty() && tipo_izq.valor->kind == TypeKind::STRUCT) {
     auto struct_type = std::static_pointer_cast<StructType>(tipo_izq.valor);

      std::string firma = struct_type->info->nombre + "_" + generarFirma(dunder, {tipo_der});
      std::cout << "[182, Checker.hpp] firma: '" << firma << "'\n";

      auto it_metodo = struct_type->info->metodos.find(firma);
      if (it_metodo != struct_type->info->metodos.end()) {
        nodo->tipo_resuelto = it_metodo->second.tipo_retorno;
        nodo->overload = firma;
        return ;
      }
    }

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

    if (nodo->elementos.empty()) {
      nodo->tipo_resuelto.valor = typeFactory.getArray(typeFactory.getUnknown(), 0);
      return ;
    }

    nodo->elementos[0]->accept(this);
    auto tipo_base = nodo->elementos[0]->tipo_resuelto.valor;

    for (size_t i = 1; i < nodo->elementos.size(); ++i) {
      nodo->elementos[i]->accept(this);
      tipo_base = promoverTipos(tipo_base, nodo->elementos[i]->tipo_resuelto.valor);
    }

    for (size_t i = 0; i < nodo->elementos.size(); ++i) {
      nodo->elementos[i] = forzarTipo(std::move(nodo->elementos[i]), Dt(tipo_base));
    }

    nodo->tipo_resuelto.valor = typeFactory.getArray(tipo_base, nodo->elementos.size());

  }


  void visitar(ExprUnaria* nodo) override { //...
    nodo->operando->accept(this);
    Dt tipo_op = nodo->operando->tipo_resuelto;

    if (tipo_op.valor->kind == TypeKind::DESCONOCIDO) {
      nodo->tipo_resuelto = tipo_op;
      return ;

    }

    switch (nodo->operador) { //...
      case TipoOperador::RESTA: {

        if (!tipo_op.valor->isNumeric()) {
          nodo->tipo_resuelto = Dt(typeFactory.getUnknown());
          std::cout << "Error: El tipo no es numérico\n"; //...
        }

        nodo->tipo_resuelto = tipo_op;

        break;
      }

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
        auto tipo_indice = nodo->inicio->tipo_resuelto.valor;

      if (tipo_indice->kind != TypeKind::INTEGER) {
        nodo->inicio = forzarTipo(std::move(nodo->inicio), Dt(typeFactory.getInteger(32, false)));

      }

      nodo->tipo_resuelto.valor = nodo->inicio->tipo_resuelto.valor;


    }

    if (nodo->fin) {
        nodo->fin->accept(this);
    }

    if (nodo->paso) {
        nodo->paso->accept(this);
    }

  }

  void visitar(ExprAcceso* nodo) override {
    nodo->contenedor->accept(this);
    nodo->rango->accept(this);

    auto tipo_contenedor = nodo->contenedor->tipo_resuelto.valor;
    auto tipo_rango = nodo->rango->tipo_resuelto.valor;

    if (tipo_contenedor->kind != TypeKind::ARRAY) {
      std::cerr << "Error: El objeto no es indexable\n";
      return ;
    }

    auto tipo_base = tipo_contenedor->getUnderlyingType();

    if (tipo_rango->kind == TypeKind::INTEGER) {
      nodo->tipo_resuelto.valor = tipo_base;

    } else if (tipo_rango->kind == TypeKind::RANGE) {
      nodo->tipo_resuelto.valor = tipo_contenedor;

    }

  }

  void visitar(ExprAccesoPunto* nodo) override {
    nodo->izquierda->accept(this);

    auto tipo_izq = nodo->izquierda->tipo_resuelto.valor;

    if (tipo_izq->kind == TypeKind::UNRESOLVED) {
      auto unresolved_type = std::static_pointer_cast<UnresolvedType>(tipo_izq);
      std::string nombre_struct = unresolved_type->pending_type;

      InfoStruct* info = tablas.buscarStruct(nombre_struct);
      if (info) {
        tipo_izq = typeFactory.getStruct(info);
        nodo->izquierda->tipo_resuelto = Dt(tipo_izq);

      } else {
        std::cerr << "Error: No se encontró la definición del struct '" << nombre_struct << "'\n";
        nodo->tipo_resuelto = Dt(typeFactory.getUnknown());
        return ;
      }
    }

    if (tipo_izq->kind != TypeKind::STRUCT) {
      std::cerr << "Error: El lado izquierdo del '.' debe ser un struct\n";
      nodo->tipo_resuelto = Dt(typeFactory.getUnknown());
      return ;
    }

    auto struct_type = std::static_pointer_cast<StructType>(tipo_izq);
    auto it_prop = struct_type->info->propiedades.find(nodo->propiedad);
    if (it_prop != struct_type->info->propiedades.end()) {
      nodo->tipo_resuelto = it_prop->second.tipo;
      return ;
    }

    std::string prefijo_metodo = struct_type->info->nombre + "_" + nodo->propiedad;
    bool es_metodo = false;

    for (const auto& [firma, info_func] : struct_type->info->metodos) {
      if (firma.find(prefijo_metodo) == 0) {
        es_metodo = true;
        break;
      }
    }

    if (es_metodo) { //...
      nodo->tipo_resuelto = Dt(typeFactory.getUnknown());
      return ;
    }

    std::cerr << "Error: El struct '" << struct_type->info->nombre << "' no tiene una propiedad o método llamado '" << nodo->propiedad << "'\n";
    exit(1);

  }


  void visitar(Bloque* nodo) override {

    tablas.entrarScope();

    traits.despacharTrait(nodo, 0);

    tablas.salirScope();

  }

  void visitar(SentenciaAsignarVar* nodo) override {

    if (auto* tipo_pendiente = dynamic_cast<UnresolvedType*>(nodo->tipo_explicito.tipo.valor.get())) {
      std::string nombre_tipo = tipo_pendiente->pending_type;

      std::cout << "[459, Checker.hpp] nombre_tipo: '" << nombre_tipo << "'\n";
      InfoStruct* info = tablas.buscarStruct(nombre_tipo);

      if (info != nullptr) {
        nodo->tipo_explicito.tipo = Dt(typeFactory.getStruct(info));

      } else {
        throw std::runtime_error("Error: El tipo '" + nombre_tipo + "' no está definido\n");

      }

    }

    if (nodo->size) {
      nodo->size->accept(this);

      if (!nodo->size->tipo_resuelto.valor->isNumeric()) {
        throw std::runtime_error("Error: El tamaño del arreglo '" + nodo->nombre + "' debe ser un entero\n");
      }

      ConstantEvaluator eval;
      auto res_size = eval.eval(nodo->size.get());
      if (!res_size.has_value()) {
        throw std::runtime_error("Error: El tamaño del arreglo '" + nodo->nombre + "' debe ser una expresion constante\n");
      }

      long long s;
      std::visit([&](const auto& arg) {
        using T = std::decay_t<decltype(arg)>;
        if constexpr (std::is_same_v<T, NumberData>) {
          s = std::stoll(arg.valor);
          if (s <= 0) {
            throw std::runtime_error("Error: El tamaño del arreglo '" + nodo->nombre + "' debe ser positivo\n");
          }
        }

      }, *res_size);

      nodo->size = forzarTipo(std::move(nodo->size), Dt(typeFactory.getInteger(64, true)));

      nodo->tipo_explicito.tipo.valor = typeFactory.getArray(nodo->tipo_explicito.tipo.valor, s);

    }

    if (nodo->valor_inicial) {

      if (auto* init_list = dynamic_cast<ExprInitList*>(nodo->valor_inicial.get())) {
        nodo->valor_inicial->tipo_resuelto = nodo->tipo_explicito.tipo;
      }

      nodo->valor_inicial->accept(this);

      Dt tipo_destino = nodo->tipo_explicito.tipo;

      nodo->valor_inicial = forzarTipo(std::move(nodo->valor_inicial), tipo_destino);

    }

    InfoVariable* info = tablas.buscarVariable(nodo->nombre);
    if (info == nullptr) { // Creamos la variable
      InfoVariable nueva_info;
      nueva_info.tipo = nodo->tipo_explicito.tipo.valor;
      tablas.añadirVariable(nodo->nombre, nueva_info);
      return ;

    } else { // Si existe, error

    }

  }

  void visitar(ExprFuncCall* nodo) override { //...

    if (auto* acceso = dynamic_cast<ExprAccesoPunto*>(nodo->callee.get())) {

      acceso->izquierda->accept(this);
      auto tipo_izq = acceso->izquierda->tipo_resuelto.valor;

      if (tipo_izq->kind == TypeKind::STRUCT) {
        auto struct_type = std::static_pointer_cast<StructType>(tipo_izq);

        std::vector<Dt> tipos_args;
        for (const auto& a : nodo->argumentos) {
          a.second->accept(this);
          tipos_args.push_back(a.second->tipo_resuelto);
        }

        std::string firma = struct_type->info->nombre + "_" + generarFirma(acceso->propiedad, tipos_args);

        auto it_metodo = struct_type->info->metodos.find(firma);
        if (it_metodo != struct_type->info->metodos.end()) {
          nodo->tipo_resuelto = it_metodo->second.tipo_retorno;

          auto instancia = std::move(acceso->izquierda);
          auto func_var = std::make_unique<ExprVariable>(firma);
          nodo->callee = std::move(func_var);

          //nodo->argumentos.insert(nodo->argumentos.begin(), {"", std::move(instancia)});
          auto ref_instancia = std::make_unique<ExprUnaria>(TipoOperador::PTR_REF, std::move(instancia), true);
          ref_instancia->accept(this);
          nodo->argumentos.insert(nodo->argumentos.begin(), {"", std::move(ref_instancia)});

          return ;

        } else {
          std::cerr << "Eerror: El struct '" << struct_type->info->nombre << "' no tiene un método que coincide con la firma '" << firma << "'\n";
          exit(1);

        }

      }

    }

    nodo->callee->accept(this);

    for (const auto& n : nodo->argumentos) {
      n.second->accept(this);
    }

    nodo->tipo_resuelto.valor = typeFactory.getInteger(32, false); //... Blindly assume it returns i32

  }

  void visitar(ExprInitList* nodo) override { //...

    if (auto* n = dynamic_cast<UnresolvedType*>(nodo->tipo_resuelto.valor.get())) {
      std::cout << "[555, Checker.hpp] n->pending_type: '" << n->pending_type << "'\n";
      InfoStruct* info = tablas.buscarStruct(n->pending_type);
      if (info != nullptr) {
        nodo->tipo_resuelto = Dt(typeFactory.getStruct(info));

      } else {
        std::cerr << "Error: Info es nullptr\n";

      }

    }

    auto struct_type = std::static_pointer_cast<StructType>(nodo->tipo_resuelto.valor);

    const auto& props_esperadas = struct_type->info->propiedades;
    const auto& orden_props = struct_type->info->orden_props;

    if (nodo->args.size() != props_esperadas.size()) {
      std::cerr << "Error: Argumentos mismatch en lista de inicialización\n";
      nodo->tipo_resuelto = Dt(typeFactory.getUnknown());
      return ;

    }

    std::vector<bool> prop_ocupada(props_esperadas.size(), false);

    for (auto& arg : nodo->args) {
      if (arg.name.has_value()) {
        bool encontrada = false;
        for (size_t i = 0; i < props_esperadas.size(); ++i) {
          if (orden_props[i] == arg.name.value()) {
            if (prop_ocupada[i]) {
              std::cerr << "Error: La propiedad '" << arg.name.value() << "' fue inicializada más de una vez\n";
              exit(1);
            }

            prop_ocupada[i] = true;
            encontrada = true;
            break;

          }
        }

        if (!encontrada) {
          std::cerr << "Error: El struct no tiene una propiedad llamada '" << arg.name.value() << "'\n";
          exit(1);
        }
      }
    }

    size_t idx = 0;
    for (auto& arg : nodo->args) {
      if (!arg.name.has_value()) {
        while (idx < orden_props.size() && prop_ocupada[idx]) {
          idx++;
        }
        if (idx < props_esperadas.size()) {
          arg.name = orden_props[idx];
          prop_ocupada[idx] = true;
        }
      }
    }

    for (auto& arg : nodo->args) {
      arg.value->accept(this);

      Dt tipo_esperado = props_esperadas.at(arg.name.value()).tipo;

      arg.value = forzarTipo(std::move(arg.value), tipo_esperado);

    }

  }

  // Sentencias
  void visitar(SentenciaExpr* nodo) override {
    nodo->expresion->accept(this);
  }

  void visitar(SentenciaReasignacionVar* nodo) override {
    nodo->izquierda->accept(this);
    nodo->derecha  ->accept(this);

    Dt t_destino = nodo->izquierda->tipo_resuelto;

    if (t_destino.valor->kind == TypeKind::DESCONOCIDO) {
      return ;
    }

    if (!nodo->izquierda->isLValue()) {
      std::cerr << "Error: Solo L-values pueden ser asigandos\n";
      return ;

    }

    nodo->derecha = forzarTipo(std::move(nodo->derecha), t_destino);

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
      std::cout << "[763, Checker.hpp] Bad Info"; //...
      exit(1);

    }

  }

  void visitar(SentenciaFuncDecl* nodo) override {

    for (auto& [nombre, info] : nodo->args_type) {
      if (auto* unresolved_type = dynamic_cast<UnresolvedType*>(info.tipo.valor.get())) {
      InfoStruct* info_struct = tablas.buscarStruct(unresolved_type->pending_type);
        if (info_struct) {
          info.tipo = Dt(typeFactory.getStruct(info_struct));
        }
      }
    }

    if (auto* unresolved_ret = dynamic_cast<UnresolvedType*>(nodo->ret_type.valor.get())) {
      InfoStruct* info_struct = tablas.buscarStruct(unresolved_ret->pending_type);
      if (info_struct) {
        nodo->ret_type = Dt(typeFactory.getStruct(info_struct));
      }
    }

    std::vector<Dt> tipos_params;
    for (const auto& [nombre, info] : nodo->args_type) {
      tipos_params.push_back(info.tipo);
    }

    std::string prefijo = structActual.empty() ? "" : structActual + "_";
    std::string firma = prefijo + generarFirma(nodo->nombre_func, tipos_params);
    nodo->firma_mangled = firma;
    std::cout << "[786, Checker.hpp] firma: '" << firma << "'\n";

    if (mode == ModoChecker::REGISTRO) {
      InfoFuncion info_func;
      info_func.nombre = nodo->nombre_func;
      info_func.tipo_retorno = nodo->ret_type;
      info_func.tipos_parametros = nodo->args_type;

      if (!tablas.añadirFunction(firma, info_func)) {
        std::cout << "[785, Checker.hpp] Error: Función redefinida\n";
        exit(1);

      }

      return ;

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

  void visitar(SentenciaStruct* nodo) override {

    std::string nombre_real = nodo->name;

    if (varsArcanosActivos.count(nombre_real)) {
      if (auto* e_var = dynamic_cast<ExprVariable*>(varsArcanosActivos[nombre_real])) {
        nombre_real = e_var->nombre;
        nodo->name = e_var->nombre;

      } else {
        throw std::runtime_error("Error: Se esperaba un identificador para la variable '" + nombre_real + "'\n");

      }
    }

    if (mode == ModoChecker::REGISTRO) {
      std::vector<std::unique_ptr<Sentencia>> props_expandidas;

      for (auto& prop : nodo->propiedades) {
        bool es_placeholder = false;

        if (auto* expr_stmt = dynamic_cast<SentenciaExpr*>(prop.get())) {
          if (auto* var_expr = dynamic_cast<ExprVariable*>(expr_stmt->expresion.get())) {
            if (bloquesArcanoActivos.count(var_expr->nombre)) {

              es_placeholder = true;
              auto* nodo_arg = bloquesArcanoActivos[var_expr->nombre];

              if (auto* bloque = dynamic_cast<Bloque*>(nodo_arg)) {
                for (const auto& inst : bloque->instrucciones) {
                  props_expandidas.push_back(inst->clonar());

                }

              } else {
                props_expandidas.push_back(nodo_arg->clonar());

              }
            }
          }
        }

        if (!es_placeholder) {
          props_expandidas.push_back(std::move(prop));
        }
      }

      nodo->propiedades = std::move(props_expandidas);

      InfoStruct info;
      info.nombre = nombre_real;

      if (!tablas.añadirStruct(nombre_real, info)) {
        throw std::runtime_error("Error: El struct '" + nombre_real + "' ya está definido\n");
      }

      InfoStruct* info_ptr = tablas.buscarStruct(nombre_real);

      for (const auto& p : nodo->propiedades) {
        p->accept(this);
        if (auto* var_decl = dynamic_cast<SentenciaAsignarVar*>(p.get())) {
          if (info_ptr->propiedades.count(var_decl->nombre)) {
            throw std::runtime_error("Error: La propiedad '" + var_decl->nombre + "'redefinida\n");
          }

          info_ptr->propiedades[var_decl->nombre] = var_decl->tipo_explicito;
          info_ptr->orden_props.push_back(var_decl->nombre);

        }
      }

      if (!nodo->metodos.empty()) {
        structActual = nombre_real;

        for (const auto& m : nodo->metodos) {
          m->accept(this);

          if (auto* func_decl = dynamic_cast<SentenciaFuncDecl*>(m.get())) {
            InfoFuncion* func_reg = tablas.buscarFunction(func_decl->firma_mangled);
            if (func_reg) {
              info_ptr->metodos[func_decl->firma_mangled] = *func_reg;
            }
          }
        }

        structActual = "";
      }

      //if (!nodo->metodos.empty()) {
      //  for (const auto& m : nodo->metodos) {
      //    if (auto* func_decl = dynamic_cast<SentenciaFuncDecl*>(m.get())) {
      //      InfoFuncion info_func;
      //      info_func.nombre = func_decl->nombre_func;
      //      info_func.tipo_retorno = func_decl->ret_type;
      //      info_func.tipos_parametros = func_decl->args_type;
      //      std::vector<Dt> tipos_args;
      //      for (const auto& a : func_decl->args_type) {
      //        tipos_args.push_back(a.second.tipo);
      //      }
      //      //std::string firma = nombre_real + "_" + generarFirma(func_decl->nombre_func, tipos_args);
      //      //std::cout << "[975, Checker.hpp] firma: '" << firma << "'\n";
      //      //func_decl->nombre_func = firma;
      //      //info.metodos[firma] = std::move(info_func);
      //      //tablas.añadirFunction(firma, info_func);
      //      info.metodos[func_decl->nombre_func] = std::move(info_func);
      //    }
      //  }
      //}
      //if (!tablas.añadirStruct(nombre_real, info)) {
      //  throw std::runtime_error("Error: El struct '" + nombre_real + "' ya está definido\n");
      //}

    } else {
      if (!nodo->metodos.empty()) {

        structActual = nombre_real;

        for (const auto& m : nodo->metodos) {
          m->accept(this);
        }

        structActual = "";

      }
    }
  }

  void visitar(SentenciaEscritura* nodo) override {

  }

  void visitar(SentenciaArcano* nodo) override {
    ArcaneDef& def = contextoArcanos.buscarDefinicionPorNombre(nodo->def.name);

    def = nodo->def;

  }

  void visitar(SentenciaLlamadaArcano* nodo) override {
    pilaLlamadasArcano.push_back(nodo);

    ArcaneDef& def = contextoArcanos.buscarDefinicionPorKeyword(nodo->nombre);
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

    auto backup_vars = varsArcanosActivos;
    for (const auto& [nombre_arg, ast_arg] : nodo->vars) {
      varsArcanosActivos[nombre_arg] = ast_arg.get();
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
        auto clon = seg.br_cont->clonar();
        clon->accept(this);
        nodo->nodos_expandidos.push_back(std::move(clon));

      }
    }

    varsArcanosActivos = backup_vars;
    bloquesArcanoActivos = backup_bloques;

    tablas.salirScope();

    pilaLlamadasArcano.pop_back();

  }

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
