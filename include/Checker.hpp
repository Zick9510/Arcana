// Checker.hpp

#pragma once

#include "ConstEval.hpp"

#include "Common.hpp"

class Checker;

class TraitChecker {
private:
  Checker& checker;

public:
  TraitChecker(Checker& c);

  void despacharTrait(Bloque* nodo, size_t idx);

  void handleNoscope(Bloque* nodo, size_t idx);

};

class TemplateHandler : public ASTVisitor {
private:
  std::unordered_map<std::string, Dt> reemplazos;
  TypeFactory& typeFactory;

  Dt aplicarReemplazo(Dt tipoOriginal) {
    if (!tipoOriginal.valor) { return tipoOriginal; }

    if (tipoOriginal.valor->kind == TypeKind::TEMPLATE_PARAM) {
      auto param_type = std::static_pointer_cast<TemplateParamType>(tipoOriginal.valor);

      if (reemplazos.count(param_type->name)) {
        return reemplazos[param_type->name];
      }

      return tipoOriginal;

    }

    if (tipoOriginal.valor->kind == TypeKind::TEMPLATE_INSTANCE) {
      auto instance_type = std::static_pointer_cast<TemplateInstanceStructType>(tipoOriginal.valor);
      Dt tipo_base = Dt(tipoOriginal.valor->getUnderlyingType());
      std::vector<Dt> new_args;
      bool necesita_reemplazo = false;

      for (const auto& arg : instance_type->argumentos) {
        Dt arg_reemplazo = aplicarReemplazo(arg);
        new_args.push_back(arg_reemplazo);

        if (arg_reemplazo.valor != arg.valor) {
          necesita_reemplazo = true;
        }
      }

      if (necesita_reemplazo) {
        return Dt(typeFactory.getTemplateInstance(instance_type->name, new_args));

      }

      return tipoOriginal;
    }

    if (tipoOriginal.valor->kind == TypeKind::POINTER) {
      Dt tipo_base = Dt(tipoOriginal.valor->getUnderlyingType());
      Dt nueva_base = aplicarReemplazo(tipo_base);

      if (nueva_base.valor != tipo_base.valor) {
        return Dt(typeFactory.getPointer(nueva_base.valor));
      }

      return tipoOriginal;

    }

    if (tipoOriginal.valor->kind == TypeKind::ARRAY) {
      auto array_type = std::static_pointer_cast<ArrayType>(tipoOriginal.valor);
      Dt tipo_base = Dt(array_type->getUnderlyingType());
      Dt nueva_base = aplicarReemplazo(tipo_base);

      if (nueva_base.valor != tipo_base.valor) {
        return Dt(typeFactory.getArray(nueva_base.valor, array_type->size));
      }

      return tipoOriginal;

    }

    return tipoOriginal;

  }

public:
  TemplateHandler(std::unordered_map<std::string, Dt> r, TypeFactory& tf)
    : reemplazos(r), typeFactory(tf) {}

  void visitar(ErrorNode* nodo) override {}
  void visitar(ExprLiteral* nodo) override {}

  void visitar(ExprVariable* nodo) override {
    nodo->tipo_resuelto = aplicarReemplazo(nodo->tipo_resuelto);

  }

  void visitar(ExprArray* nodo) override {
    for (auto& e : nodo->elementos) {
      e->accept(this);
    }

    nodo->tipo_resuelto = aplicarReemplazo(nodo->tipo_resuelto);

  }

  void visitar(ExprUnaria* nodo) override {
    nodo->operando->accept(this);
    nodo->tipo_resuelto = aplicarReemplazo(nodo->tipo_resuelto);

  }

  void visitar(ExprBinaria* nodo) override {
    nodo->izquierda->accept(this);
    nodo->derecha  ->accept(this);
    nodo->tipo_resuelto = aplicarReemplazo(nodo->tipo_resuelto);

  }

  void visitar(ExprTernaria* nodo) override {
    nodo->condicion ->accept(this);
    nodo->rama_true ->accept(this);
    nodo->rama_false->accept(this);

  }

  void visitar(ExprCasteo* nodo) override {
    nodo->expresion->accept(this);
    nodo->tipo_casteo = aplicarReemplazo(nodo->tipo_casteo);

  }

  void visitar(ExprRango* nodo) override {
    if (nodo->inicio) {
      nodo->inicio->accept(this);
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

  }

  void visitar(ExprAccesoPunto* nodo) override {
    nodo->tipo_resuelto = aplicarReemplazo(nodo->tipo_resuelto);
    nodo->izquierda->accept(this);

  }

  void visitar(ExprFuncCall* nodo) override {
    nodo->callee->accept(this);

    for (auto& arg : nodo->argumentos) {
      arg.second->accept(this);
    }

    nodo->tipo_resuelto = aplicarReemplazo(nodo->tipo_resuelto);

  }

  void visitar(ExprInitList* nodo) override {
    nodo->tipo_resuelto = aplicarReemplazo(nodo->tipo_resuelto);
    for (auto& arg : nodo->args) {
      if (arg.value) { arg.value->accept(this); }
    }
  }

  void visitar(Bloque* nodo) override {
    for (auto& inst : nodo->instrucciones) {
      inst->accept(this);
    }
  }

  void visitar(SentenciaAsignarVar* nodo) override {

    nodo->tipo_explicito.tipo = aplicarReemplazo(nodo->tipo_explicito.tipo);

    if (nodo->valor_inicial) { nodo->valor_inicial->accept(this); }
    if (nodo->size)          { nodo->size         ->accept(this); }

  }

  void visitar(SentenciaExpr* nodo) override {
    nodo->expresion->accept(this);
    nodo->expresion->tipo_resuelto = aplicarReemplazo(nodo->expresion->tipo_resuelto);

  }

  void visitar(SentenciaReasignacionVar* nodo) override {
    nodo->izquierda->accept(this);
    nodo->derecha  ->accept(this);

  }

  void visitar(SentenciaSi* nodo) override {}
  void visitar(SentenciaSino* nodo) override {}
  void visitar(SentenciaMientras* nodo) override {}
  void visitar(SentenciaBreak* nodo) override {}
  void visitar(SentenciaContinue* nodo) override {}
  void visitar(SentenciaRedo* nodo) override {}

  void visitar(SentenciaReturn* nodo) override {
    if (!nodo->ret_value) { return ; }

    nodo->ret_type = aplicarReemplazo(nodo->ret_type);
    nodo->ret_value->accept(this);

  }

  void visitar(SentenciaFuncDecl* nodo) override {

    nodo->ret_type = aplicarReemplazo(nodo->ret_type);

    for (auto& [nombre, info] : nodo->args_type) {
      info.tipo = aplicarReemplazo(info.tipo);
    }

    for (auto& inst : nodo->cuerpo_func) {
      inst->accept(this);
    }

  }

  void visitar(SentenciaStruct* nodo) override {
    for (auto& prop : nodo->propiedades) {
      prop->accept(this);
    }

    for (auto& metodo : nodo->metodos) {
      metodo->accept(this);
    }

  }

  void visitar(SentenciaEscritura* nodo) override {}
  void visitar(SentenciaArcano* nodo) override {}
  void visitar(SentenciaLlamadaArcano* nodo) override {}
  void visitar(SentenciaMetaDirective* nodo) override {}
  void visitar(SentenciaTemplate* nodo) override {}
  void visitar(SentenciaInclude* nodo) override {}

};

enum class ModoChecker {
  REGISTRO,
  VERIFICACION
};

class Checker : public ASTVisitor {
private:
  GestorTablas& tablas;
  std::vector<std::unique_ptr<Sentencia>>& ast;
  std::vector<std::unique_ptr<Sentencia>> templates;
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
      case TipoOperador::SUMA : { return dunder::ADD; }
      case TipoOperador::RESTA: { return dunder::SUB; }

      default                 : { return          ""; }
    }
  }

  inline Dt getClangType(CXType tipoClang) {

    bool es_const = clang_isConstQualifiedType(tipoClang);

    std::shared_ptr<ArcanaType> tipo = nullptr;

    switch (tipoClang.kind) {

      case CXType_Void: {
        tipo = typeFactory.getVoid();
        break;
      }

      case CXType_Bool: {
        tipo = typeFactory.getBoolean();
        break;
      }

      // Char
      case CXType_Char_U:
      case CXType_UChar :
      case CXType_Char_S:
      case CXType_SChar : { tipo = typeFactory.getChar(8); break; }

      // Signed Integers
      case CXType_Short   : { tipo = typeFactory.getInteger(16 , false); break; }
      case CXType_Int     : { tipo = typeFactory.getInteger(32 , false); break; }
      case CXType_Long    :
      case CXType_LongLong: { tipo = typeFactory.getInteger(64 , false); break; }
      case CXType_Int128  : { tipo = typeFactory.getInteger(128, false); break; }

      // Unsgined Integers
      case CXType_UShort   : { tipo = typeFactory.getInteger(16, true); break; }
      case CXType_UInt     : { tipo = typeFactory.getInteger(32, true); break; }
      case CXType_ULong    :
      case CXType_ULongLong: { tipo = typeFactory.getInteger(64, true); break; }
      case CXType_UInt128  : { tipo = typeFactory.getInteger(128, true); break; }

      // Floats
      case CXType_Float16 :
      case CXType_BFloat16:
      case CXType_Half    : { tipo = typeFactory.getFloat( 16); break; }
      case CXType_Float   : { tipo = typeFactory.getFloat( 32); break; }
      case CXType_Double  : { tipo = typeFactory.getFloat( 64); break; }
      case CXType_Float128: { tipo = typeFactory.getFloat(128); break; }

      // Pointers
      case CXType_Pointer: {
        CXType apuntado = clang_getPointeeType(tipoClang);
        Dt dt_apuntado = getClangType(apuntado);
        tipo = typeFactory.getPointer(dt_apuntado.valor);
        break;
      }

      // Constant Arrays
      case CXType_ConstantArray: {
        CXType elem = clang_getArrayElementType(tipoClang);
        long long size = clang_getArraySize(tipoClang);
        Dt dt_elem = getClangType(elem);
        tipo = typeFactory.getArray(dt_elem.valor, static_cast<int>(size));
        break;
      }

      // Incomplete Arrays
      case CXType_IncompleteArray: {
        CXType elem = clang_getArrayElementType(tipoClang);
        Dt dt_elem = getClangType(elem);
        tipo = typeFactory.getArray(dt_elem.valor, -1);
        break;
      }

      // Typedefs & Structs
      case CXType_Typedef   :
      case CXType_Elaborated: {
        CXType real = clang_getCanonicalType(tipoClang);
        return getClangType(real);
      }

      case CXType_Record: {
        CXCursor decl_cursor = clang_getTypeDeclaration(tipoClang);
        CXString cx_name = clang_getCursorSpelling(decl_cursor);
        std::string name = clang_getCString(cx_name);
        clang_disposeString(cx_name);

        if (name.empty()) { name = "anon"; }
        tipo = typeFactory.getUnresolved(name);
        break;
      }

      // Enums
      case CXType_Enum: {
        tipo = typeFactory.getInteger(32, false);
        break;
      }

      // Unknown
      default: {
        tipo = typeFactory.getUnknown();
        break;
      }

    }

    Dt result(tipo);
    result.es_const = es_const;
    return result;

  }

  static CXChildVisitResult visitarClang(CXCursor cursor, CXCursor parent, CXClientData client) {

    Checker* checker = static_cast<Checker*>(client); // "this", but with extra steps

    CXCursorKind tipo_nodo = clang_getCursorKind(cursor);

    if (tipo_nodo == CXCursor_FunctionDecl) {
      CXString cx_name = clang_getCursorSpelling(cursor);
      std::string nombre_func = clang_getCString(cx_name);
      clang_disposeString(cx_name);

      if (nombre_func.empty()) { return CXChildVisit_Continue; }

      InfoFuncion info_func;
      info_func.nombre = nombre_func;
      CXType cx_ret_type = clang_getCursorResultType(cursor);
      info_func.tipo_retorno = checker->getClangType(cx_ret_type);

      int num_args = clang_Cursor_getNumArguments(cursor);
      for (int i = 0; i < num_args; ++i) {
        CXCursor arg_cursor = clang_Cursor_getArgument(cursor, i);

        CXString cx_arg_name = clang_getCursorSpelling(arg_cursor);
        std::string arg_name = clang_getCString(cx_arg_name);
        clang_disposeString(cx_arg_name);

        if (arg_name.empty()) { arg_name = "arg" + std::to_string(i); }

        CXType arg_type = clang_getCursorType(arg_cursor);
        InfoVariable info_var;
        info_var.tipo = checker->getClangType(arg_type);
        info_var.es_const = clang_isConstQualifiedType(arg_type);

        info_func.tipos_parametros.push_back({arg_name, info_var});

      }

      std::vector<Dt> clang_args;

      for (auto& param : info_func.tipos_parametros) {
        clang_args.push_back(param.second.tipo);
      }

      info_func.firma = generarFirma(nombre_func, clang_args);
      info_func.is_external = true;
      info_func.is_variadic = clang_Cursor_isVariadic(cursor);

      std::cout << "[474, Checker.hpp] info_func.firma: '" << info_func.firma << "'\n";

      checker->tablas.añadirFuncion(nombre_func, info_func);

      std::cout << '\n';

    } else if (tipo_nodo == CXCursor_StructDecl) {
      CXString cx_name = clang_getCursorSpelling(cursor);
      std::string nombre_struct = clang_getCString(cx_name);
      clang_disposeString(cx_name);

      if (nombre_struct.empty()) { return CXChildVisit_Continue; }

      std::cout << "[488, Checker.hpp] nombre_struct: '" << nombre_struct << "'\n";
      InfoStruct info_struct;
      info_struct.nombre = nombre_struct;

      checker->tablas.añadirStruct(nombre_struct, info_struct);

    } else if (tipo_nodo == CXCursor_TypedefDecl) {
      CXString cx_name = clang_getCursorSpelling(cursor);
      std::string nombre_typedef = clang_getCString(cx_name);
      clang_disposeString(cx_name);

      if (nombre_typedef.empty()) { return CXChildVisit_Continue; }

      CXType under_type = clang_getTypedefDeclUnderlyingType(cursor);
      CXString cx_under_name = clang_getTypeSpelling(under_type);
      std::string nombre_real = clang_getCString(cx_under_name);
      clang_disposeString(cx_under_name);

      const std::string prefix = "struct ";
      if (nombre_real.find(prefix) == 0) {
        nombre_real = nombre_real.substr(prefix.length());
      }

      checker->tablas.añadirTypedef(nombre_typedef, nombre_real);

    }

    return CXChildVisit_Continue;

  }

  void loadSymbolsLibclang(const std::string& path, bool isSystemHeader) {
    CXIndex index = clang_createIndex(0, 0);

    std::string code;
    if   (isSystemHeader) { code = "#include <"  + path +  ">\n"; }
    else                  { code = "#include \"" + path + "\"\n"; }

    CXUnsavedFile virtual_file;
    virtual_file.Filename =  "virtual_file.c";
    virtual_file.Contents = code.c_str();
    virtual_file.Length   = code.length();

    const char* args_clang[] = {
      "-x", "c",
      "-fsyntax-only"
    };

    int num_args = sizeof(args_clang) / sizeof(args_clang[0]);

    CXTranslationUnit unit = clang_parseTranslationUnit(
      index,
      "virtual_file.c",
      args_clang,
      num_args,
      &virtual_file,
      1,
      CXTranslationUnit_SkipFunctionBodies
    );

    if (unit == nullptr) {
      clang_disposeIndex(index);
      return ;
    }

    CXCursor cursor = clang_getTranslationUnitCursor(unit);
    clang_visitChildren(cursor, Checker::visitarClang, this);

    clang_disposeTranslationUnit(unit);
    clang_disposeIndex(index);

  }

  Dt instanciarTemplate(std::shared_ptr<TemplateInstanceStructType> tipoInstancia) {
    std::string name = tipoInstancia->name;
    std::cout << "[318, Checker.hpp] name: '" << name << "'\n";

    std::string firma = generarFirma(name, tipoInstancia->argumentos);

    std::cout << "[322, Checker.hpp] firma: '" << firma << "'\n";

    if (InfoStruct* info_struct = tablas.buscarStruct(firma)) {
      return Dt(typeFactory.getStruct(info_struct));
    }

    InfoTemplate* info_tmpl = tablas.buscarTemplate(name);
    if (!info_tmpl) {
      throw std::runtime_error("Error: Plantilla '" + name + "' no definida");
    }

    std::cout << "[335, Checker.hpp] mapa_reemplazos:\n\n";
    std::unordered_map<std::string, Dt> mapa_reemplazos;
    for (size_t i = 0; i < info_tmpl->args.size(); ++i) {
      mapa_reemplazos[info_tmpl->args[i].first] = tipoInstancia->argumentos[i];
      std::cout << "[339, Checker.hpp] info_tmpl->args[i].first: '" << info_tmpl->args[i].first << "'\n";
      std::cout << "[340, Checker.hpp] tipoInstancia->argumentos[i]: '" << tipoInstancia->argumentos[i].tipoString() << "'\n";

    }

    std::cout << '\n';

    auto ast_clon = info_tmpl->ast->clonar();

    if (auto* struct_ast = dynamic_cast<SentenciaStruct*>(ast_clon.get())) {
      struct_ast->name = firma;
    }

    TemplateHandler tmplHandler(mapa_reemplazos, typeFactory);
    ast_clon->accept(&tmplHandler);

    ModoChecker modo_prev = mode;
    Scope* scope_prev = tablas.getScopeActual();
    bool lectura_prev = tablas.getLectura();

    if (info_tmpl->scope_def != nullptr) {
      tablas.setScopeActual(info_tmpl->scope_def);
    }

    size_t nav_prev = tablas.getScopeActual()->hijo_actual;

    mode = ModoChecker::REGISTRO;
    if (tablas.getLectura()) { tablas.switchMode(); }

    ast_clon->accept(this);

    tablas.getScopeActual()->hijo_actual = tablas.getScopeActual()->hijos.size() - 1;

    mode = ModoChecker::VERIFICACION;
    if (!tablas.getLectura()) { tablas.switchMode(); }

    ast_clon->accept(this);

    tablas.getScopeActual()->hijo_actual = nav_prev;
    tablas.setScopeActual(scope_prev);

    mode = modo_prev;

    if (tablas.getLectura() != lectura_prev) { tablas.switchMode(); }

    if (InfoStruct* info_struct = tablas.buscarStruct(firma)) {
      std::cout << "[360, Checker.hpp]\n";
      templates.push_back(std::move(ast_clon));
      return Dt(typeFactory.getStruct(info_struct));
    }

    throw std::runtime_error("Error: No se pudo registrar la instancia de la plantilla '" + firma + "'");

  }

  std::shared_ptr<ArcanaType> aplicarTypedefs(std::shared_ptr<ArcanaType> tipo) {
    if (!tipo) { return nullptr; }

    switch (tipo->kind) {
      case TypeKind::UNRESOLVED: {
        auto unres = std::static_pointer_cast<UnresolvedType>(tipo);
        return typeFactory.getUnresolved(tablas.resolverTypedef(unres->pending_type));
      }

      case TypeKind::POINTER: {
        auto ptr = std::static_pointer_cast<PointerType>(tipo);
        return typeFactory.getPointer(aplicarTypedefs(ptr->tipo_apuntado));
      }

      case TypeKind::ARRAY: {
        auto arr = std::static_pointer_cast<ArrayType>(tipo);
        return typeFactory.getArray(aplicarTypedefs(arr->base), arr->size);
      }

      default: {
        return tipo;
      }

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

      [&](StringData& d) {
        nodo->tipo_resuelto = Dt(typeFactory.getString());
      },

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
    std::cout << "[768, Checker.hpp] ExprVariable\n";

    if (bloquesArcanoActivos.empty()) {
      std::cout << "[771, Checker.hpp] bloquesArcanoActivos vacío\n";

    } else {
      std::cout << "[774, Checker.hpp] bloquesArcanoActivos:\n";
      for (const auto& [key, val] : bloquesArcanoActivos) {
        std::cout << "[776, Checker.hpp] key: '" << key << "'\n";

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

      default: {
        std::cerr << "Error: Operador unario no implementado\n";
        exit(1);
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

    if (tipo_contenedor->kind != TypeKind::ARRAY   &&
        tipo_contenedor->kind != TypeKind::STRING  &&
        tipo_contenedor->kind != TypeKind::POINTER ) {
      std::cerr << "Error: El objeto no es indexable\n";
      return ;
    }

    if (tipo_contenedor->kind == TypeKind::ARRAY) {
      auto tipo_base = tipo_contenedor->getUnderlyingType();

      if (tipo_rango->kind == TypeKind::INTEGER) {
        nodo->tipo_resuelto.valor = tipo_base;

      } else if (tipo_rango->kind == TypeKind::RANGE) {
        nodo->tipo_resuelto.valor = tipo_contenedor;

      }

    } else if (tipo_contenedor->kind == TypeKind::STRING) {

      if (tipo_rango->kind == TypeKind::INTEGER) {
        nodo->tipo_resuelto.valor = typeFactory.getChar(8);

      } else if (tipo_rango->kind == TypeKind::RANGE) {
        nodo->tipo_resuelto.valor = tipo_contenedor;

      }

    } else if (tipo_contenedor->kind == TypeKind::POINTER) {

      if (tipo_rango->kind == TypeKind::INTEGER) {
        nodo->tipo_resuelto.valor = tipo_contenedor->getUnderlyingType();

      } else if (tipo_rango->kind == TypeKind::RANGE) {
        nodo->tipo_resuelto.valor = typeFactory.getArray(tipo_contenedor->getUnderlyingType(), -1);

      }
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

    if (tipo_izq->kind == TypeKind::TEMPLATE_INSTANCE) {
      std::cout << "[786, Checker.hpp] TypeKind::TEMPLATE_INSTANCE\n";
      auto instance_type = std::static_pointer_cast<TemplateInstanceStructType>(tipo_izq);
      InfoTemplate* info_tmpl = tablas.buscarTemplate(instance_type->name);
      std::cout << "[789, Checker.hpp] instance_type->name: '" << instance_type->name << "'\n";

      if (info_tmpl) {
        tipo_izq = typeFactory.getTemplateInstance(instance_type->name, instance_type->argumentos);
        nodo->izquierda->tipo_resuelto = Dt(tipo_izq);
        nodo->tipo_resuelto = Dt(typeFactory.getUnresolved(instance_type->name)); //...
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
    std::cout << "[1130, Checker.hp] SentenciaAsignarVar\n";

    if (auto* tipo_instancia = dynamic_cast<TemplateInstanceStructType*>(nodo->tipo_explicito.tipo.valor.get())) {
      std::cout << "[1133, Checker.hpp]\n";
      auto ptr_instancia = std::static_pointer_cast<TemplateInstanceStructType>(nodo->tipo_explicito.tipo.valor);
      nodo->tipo_explicito.tipo = instanciarTemplate(ptr_instancia);

    }

    if (auto* tipo_pendiente = dynamic_cast<UnresolvedType*>(nodo->tipo_explicito.tipo.valor.get())) {
      std::string nombre_tipo = tipo_pendiente->pending_type;

      std::cout << "[1142, Checker.hpp] nombre_tipo: '" << nombre_tipo << "'\n";
      InfoStruct* info_struct = tablas.buscarStruct(nombre_tipo);

      if (info_struct != nullptr) {
        nodo->tipo_explicito.tipo = Dt(typeFactory.getStruct(info_struct));

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

  void visitar(ExprFuncCall* nodo) override { //..
    std::cout << "[1157, Checker.hpp] ExprFuncCall\n";

    bool func_resuleta = false;

    for (const auto& a : nodo->argumentos) {
      a.second->accept(this);
      a.second->tipo_resuelto.valor = aplicarTypedefs(a.second->tipo_resuelto.valor);

    }

    if (auto* var_callee = dynamic_cast<ExprVariable*>(nodo->callee.get())) {

      std::vector<Dt> args_type;
      for (auto& a : nodo->argumentos) {
        args_type.push_back(a.second->tipo_resuelto.valor);
      }

      std::string firma = generarFirma(var_callee->nombre, args_type);
      std::cout << "[1230, Checker.hpp] firma: '" << firma << "'\n";

      if (InfoFuncion* info = tablas.buscarFuncionFirma(var_callee->nombre, firma)) {
        std::cout << "[1233, Checker.hpp]\n";
        var_callee->tipo_resuelto.valor = typeFactory.getUnknown();
        nodo->tipo_resuelto = info->tipo_retorno;
        nodo->info = info;
        func_resuleta = true;

      } else {
        std::cout << "[1240, Checker.hpp] var_callee->nombre: '" << var_callee->nombre << "'\n";
        std::vector<InfoFuncion>* candidatas = tablas.buscarFuncionName(var_callee->nombre);

        if (!candidatas) {
          std::cout << "[1244, Checker.hpp] !candidatas\n";
          nodo->callee->accept(this);
          return ;
        }

        std::cout << "[1249, Checker.hpp] candidatas:\n";
        for (auto& i : *candidatas) {
          std::cout << "[1251, Checker.hpp] i.nombre: '" << i.nombre << "'\n";
          std::cout << "[1252, Checker.hpp] i.firma: '" <<  i.firma << "'\n";
        }

        InfoFuncion* info_mejor = nullptr;
        int best_fit = -1;
        bool ambiguedad = false;

        for (auto& cand : *candidatas) {

          size_t min_args = cand.tipos_parametros.size();

          if (cand.is_variadic) {
            if (args_type.size() < min_args) { continue; }

          } else {
            if (args_type.size() != min_args) { continue; }

          }

          int current_fit = 0;
          bool viable = true;

          for (size_t i = 0; i < min_args; ++i) {

            Dt param = cand.tipos_parametros[i].second.tipo;
            Dt arg   = args_type[i];

            if (arg == param) { // This should be empty

            } else if (esCasteoValido(arg, param)) {
              current_fit++;

            } else {
              viable = false;
              break;

            }

            if (best_fit != -1 && current_fit > best_fit) {
              viable = false;
              break;

            }

          }

          if (!viable) { continue; }

          if (best_fit == -1 || current_fit < best_fit) {
            best_fit = current_fit;
            info_mejor = &cand;
            ambiguedad = false;

          } else if (current_fit == best_fit) {
            ambiguedad = true;

          }

        }

        if (ambiguedad) {
          std::cerr << "Error: LLamada ambigua a '" << var_callee->nombre << '\n';
          exit(1);

        } else if (!info_mejor) {
          std::cerr << "Error: Ninguna sobrecarga de '" << var_callee->nombre << "' coincide con los tipos proporcionados\n";
          exit(1);

        }

        for (size_t i = 0; i < nodo->argumentos.size(); ++i) {
          if (i < info_mejor->tipos_parametros.size()) {
            Dt param_type = info_mejor->tipos_parametros[i].second.tipo;
            nodo->argumentos[i].second = forzarTipo(std::move(nodo->argumentos[i].second), param_type);
          }
        }

        var_callee->accept(this);
        nodo->tipo_resuelto = info_mejor->tipo_retorno;
        nodo->info = info_mejor;
        func_resuleta = true;

      }

    } else if (auto* acceso = dynamic_cast<ExprAccesoPunto*>(nodo->callee.get())) {
      std::cout << "[1184, Checker.hpp] Es ExprAccesoPunto\n";

      acceso->izquierda->accept(this);
      auto tipo_izq = acceso->izquierda->tipo_resuelto.valor;

      if (tipo_izq->kind == TypeKind::STRUCT) {
        std::cout << "[1190, Checker.hpp] y es Struct\n";
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
          nodo->info = &it_metodo->second;

          auto instancia = std::move(acceso->izquierda);
          auto func_var = std::make_unique<ExprVariable>(firma);
          nodo->callee = std::move(func_var);

          auto ref_instancia = std::make_unique<ExprUnaria>(TipoOperador::PTR_REF, std::move(instancia), true);
          ref_instancia->accept(this);
          nodo->argumentos.insert(nodo->argumentos.begin(), {"", std::move(ref_instancia)});

          return ;

        } else {
          std::cerr << "Error: El struct '" << struct_type->info->nombre << "' no tiene un método que coincide con la firma '" << firma << "'\n";
          exit(1);

        }

      }

      nodo->callee->accept(this);

    } else {
      nodo->callee->accept(this);

    }

    if (!func_resuleta) {
      nodo->tipo_resuelto.valor = typeFactory.getInteger(32, false); //... Blindly assume it returns i32
    }
  }

  void visitar(ExprInitList* nodo) override { //...

    if (auto* n = dynamic_cast<UnresolvedType*>(nodo->tipo_resuelto.valor.get())) {
      std::cout << "[973, Checker.hpp] n->pending_type: '" << n->pending_type << "'\n";
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
      std::cout << "[1128, Checker.hpp] Bad Info"; //...
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
    std::cout << "[1047, Checker.hpp] firma: '" << firma << "'\n";

    if (mode == ModoChecker::REGISTRO) {
      InfoFuncion info_func;
      info_func.nombre = nodo->nombre_func;
      info_func.tipo_retorno = nodo->ret_type;
      info_func.tipos_parametros = nodo->args_type;
      info_func.firma = firma;
      info_func.is_external = false;

      if (!tablas.añadirFuncion(nodo->nombre_func, info_func)) {
        std::cout << "[1056, Checker.hpp] Error: Función redefinida\n";
        exit(1);

      }

      return ;

    }

    InfoFuncion* ptr_func = tablas.buscarFuncionFirma(nodo->nombre_func, firma);
    tablas.pushFunction(ptr_func);

    tablas.switchMode();
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
    tablas.switchMode();

    tablas.popFunction();

  }

  void visitar(SentenciaStruct* nodo) override {

    tablas.entrarScope();

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

          std::cout << "[1295, Checker.hpp] tablas.getScopeActual: '" << tablas.getScopeActual() << "'\n";
          m->accept(this);

          if (auto* func_decl = dynamic_cast<SentenciaFuncDecl*>(m.get())) {
            InfoFuncion* func_reg = tablas.buscarFuncionFirma(func_decl->nombre_func, func_decl->firma_mangled);
            if (func_reg) {
              info_ptr->metodos[func_decl->firma_mangled] = *func_reg;
            }
          }

        }

        structActual = "";

      }

    } else {

      for (const auto& p : nodo->propiedades) {
        p->accept(this);
      }

      if (!nodo->metodos.empty()) {

        structActual = nombre_real;

        for (const auto& m : nodo->metodos) {
          m->accept(this);
        }

        structActual = "";

      }
    }

    tablas.salirScope();

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

  void visitar(SentenciaTemplate* nodo) override {
    if (mode == ModoChecker::REGISTRO) {
      tablas.updateTemplate(nodo->name, tablas.getScopeActual());
    }
  }

  void visitar(SentenciaInclude* nodo) override {
    if (mode == ModoChecker::REGISTRO) {
      bool es_c_header = nodo->is_system_header || nodo->path.ends_with(".h");

      if (es_c_header) {
        loadSymbolsLibclang(nodo->path, nodo->is_system_header);

      }
    }
  }

};
