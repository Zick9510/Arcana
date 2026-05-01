// Checker.hpp

#pragma once

#include "Common.hpp"

class Checker : public ASTVisitor {
private:
  GestorTablas& tablas;
  std::vector<std::unique_ptr<Sentencia>>& ast;
  ErrorHandler& errHandler;
  TypeFactory& typeFactory;
  ContextoArcanos& contextoArcanos;
  std::map<std::string, Sentencia*> bloques_arcano_activos;

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

  std::shared_ptr<ArcanaType> verificarCmpMenor(const Dt& izq, const Dt& der);

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

    //std::cout << "[106, Checker.hpp]\n";

    if (tipo.valor != typeFactory.getUnknown()) {
      nodo->tipo_resuelto = tipo;
      return ;
    }

    //std::cout << "[113, Checker.hpp]\n";

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

    if (bloques_arcano_activos.count(nodo->nombre)) {
      auto* bloque = bloques_arcano_activos[nodo->nombre];
      bloque->accept(this);

      if (auto* s_expr = dynamic_cast<SentenciaExpr*>(bloque)) {
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

    for (const auto& i : nodo->instrucciones) {
      i->accept(this);
    }

    tablas.salirScope();

  }

  void visitar(SentenciaAsignarVar* nodo) override {
    if (nodo->valor_inicial) { nodo->valor_inicial->accept(this); }

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
    info_func.linea = nodo->linea;

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
      for (const auto& inst : nodo->cuerpo_func) {
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

    //for (const auto& branch : nodo->def.branches) {
    //  tablas.entrarScope();
    //  for (const auto& segment : branch.segmentos) {
    //    for (const auto& [name, info] : segment.br_args) {
    //      tablas.añadirVariable(name, info);
    //    }
    //    if (segment.br_cont != nullptr) {
    //      segment.br_cont->accept(this);
    //    }
    //  }
    //  tablas.salirScope();
    //}

    def = nodo->def;

  }

  void visitar(SentenciaLlamadaArcano* nodo) override {

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

    auto backup_bloques = bloques_arcano_activos;

    for (const auto& [nombre_arg, ast_arg] : nodo->expr) {
      bloques_arcano_activos[nombre_arg] = ast_arg.get();
    }

    for (const auto& [nombre_arg, ast_arg] : nodo->code) {
      bloques_arcano_activos[nombre_arg] = ast_arg.get();
    }

    for (const auto& seg : rama->segmentos) {
      if (seg.br_cont) {
        seg.br_cont->accept(this);
      }
    }

    bloques_arcano_activos = backup_bloques;
    tablas.salirScope();

    //for (const auto& [name, arg_nodo] : nodo->args) {
    //  if (arg_nodo != nullptr) {
    //    arg_nodo->accept(this);
    //  }
    //}
    //
    //for (const auto& [name, arg_nodo] : nodo->expr) {
    //  if (arg_nodo != nullptr) {
    //    arg_nodo->accept(this);
    //  }
    //}
    //
    //for (const auto& [name, arg_nodo] : nodo->code) {
    //  if (arg_nodo != nullptr) {
    //    arg_nodo->accept(this);
    //  }
    //}

  }

};
