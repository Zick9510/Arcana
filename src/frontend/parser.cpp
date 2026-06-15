// parser.cpp

#include "Parser.hpp"

#include "Common.hpp"

/* --- Parser --- */
Token Parser::resolverAlias(Token t) {
  if (t.tipo == Tt::IDENTIFICADOR && aliasLexicos.count(t.lexema)) {
    t.tipo = aliasLexicos[t.lexema];
  }
  return t;
}

// Look the next token and return it. If its the EOF, give me the last token
Token Parser::peek(size_t offset) {
  if (pos + offset >= tokens.size()) { return tokens.back(); } // EOF
  return resolverAlias(tokens[pos + offset]);
}

// Consume the next token and return it
Token Parser::get() {
  Token actual = peek();
  pos++;
  return actual;
}

// Same use as Token check(Tt) function from this file, but it expects any of the options
Token Parser::coincide(std::initializer_list<Tt> tipos) {
  for (Tt tipo : tipos) {
    if (peek().tipo == tipo) {
      return get();
    }
  }

  std::cerr << "Error: Se esperaba alguna ";
  for (Tt tipo : tipos) {
    std::cerr << nombreTipo(tipo) << ' ';
  }

  std::cerr << "pero se encontró "   << nombreTipo(peek().tipo)  << " (" << peek().lexema << ")\n";
  exit(1);

}

bool Parser::isStatement(Token t) {
  if (isKeyword(t.tipo)) { return true; }
  if (t.tipo == Tt::IDENTIFICADOR &&
      contextoArcanos.esKeywordArcano(t.lexema)) {
    return true;
  }
  return false;

}

bool Parser::sync(Tt target) {

  while (peek().tipo != Tt::EOF_TT) {
    Token t = peek();

    if (t.tipo == target)         { return true ; }
    if (isStructural(t.tipo) ||
        isStatement (t        ) ) { return false; }

    get();

  }

  return false;

}

// We expect the next token to be exactly this one, if its not, then error
Token Parser::check(Tt tipoEsperado, Pm parseMode) {

  if (parseMode == Pm::RELAXED && tipoEsperado == Tt::IDENTIFICADOR) {
    if (isKeyword(peek().tipo)) {
      return get();
    }
  }

  if (peek().tipo == tipoEsperado) {
    return get();
  }

  //... We have to report this error and call the not implemented yet error recovery function
  std::cerr << "Error: Se esperaba "  << nombreTipo(tipoEsperado)
            << " pero se encontró "   << nombreTipo(peek().tipo)  << " ('"
            << peek().lexema          << "')\n";

  std::cout << "Backtrace:\n";
  std::cout << std::stacktrace::current() << '\n';

  std::cout << "[94, parser.cpp]\n";
  std::cout << peek().lexema << ' ' << peek().pos.cur << ", " << peek().pos.len << '\n';

  errHandler.report(CE::E_EXPECTED_TOKEN, peek().pos, nombreTipo(tipoEsperado), nombreTipo(peek().tipo));

  bool flag = sync(tipoEsperado);

  if (flag) {
    return get();

  } else {
    Token ghost  = peek()      ;
    ghost.tipo   = tipoEsperado;
    ghost.lexema = "<recovery>";
    return ghost;

  }

}

bool Parser::isType(Token t) {

  if (esTipo(t.tipo)) { // Base types
    return true;
  }

  if (t.tipo == Tt::IDENTIFICADOR) { // User defined types
    if (tablas.buscarStruct       (t.lexema) != nullptr) { return true; }
    if (tablas.buscarTemplate     (t.lexema) != nullptr) { return true; }
    if (tablas.buscarTemplateParam(t.lexema) != nullptr) { return true; }

  }

  return false;

}

Parser::Parser(std::vector<Token> tok, GestorTablas& t, ErrorHandler& e, ContextoArcanos& ca, TypeFactory& tf)
  : tokens(std::move(tok)), tablas(t), pos(0), errHandler(e), contextoArcanos(ca), typeFactory(tf) {}

int extraerBits(std::string lexema, int defaultBits) {
  if (lexema == "int" || lexema == "raw" || lexema == "float") { return defaultBits; }

  std::string num;

  for (char c : lexema) {
    if (std::isdigit(c)) { num += c; }
  }

  return (num.empty() ? defaultBits : std::stoi(num));

}

InfoVariable Parser::parsearTipo() {
  std::cout << "[147, parser.cpp] parsearTipo\n";
  InfoVariable info;
  std::shared_ptr<ArcanaType> tipo_actual = nullptr;
  bool es_unsigned = false;

  // Modifiers
  while (peek().tipo == Tt::CONST || peek().tipo == Tt::UNSIGNED) {
    if (get().tipo == Tt::CONST) {
      if (info.es_const) {
        //... Error, dos veces const
      } else {
        info.es_const = true;
      }
    } else {
      if (es_unsigned) {
        //... Error, dos veces unsigned
      } else {
        es_unsigned = true;
      }
    }
  }

  Token t_base = peek();

  switch (t_base.tipo) {

    case Tt::VOID_TYPE: {
      get();
      tipo_actual = typeFactory.getVoid();
      break;
    }

    case Tt::SHORT_TYPE: {
      get();
      int bits = extraerBits(t_base.lexema, 16);
      if (bits > 16) {
        //... Error, los tipos short no pueden tener más de 16 bits asignados
      }
      tipo_actual = typeFactory.getInteger(bits, es_unsigned);
      break;
    }

    case Tt::INT_TYPE: {
      get();
      int bits = extraerBits(t_base.lexema, 32);
      tipo_actual = typeFactory.getInteger(bits, es_unsigned);
      break;
    }

    case Tt::UINT_TYPE: {
      get();
      int bits = extraerBits(t_base.lexema, 32);
      tipo_actual = typeFactory.getInteger(bits, true);
      break;
    }

    case Tt::FLOAT_TYPE: {
      get();
      int bits = extraerBits(t_base.lexema, 64);
      tipo_actual = typeFactory.getFloat(bits);
      break;
    }

    case Tt::CHAR_TYPE: {
      get();
      int bits = extraerBits(t_base.lexema, 8);
      tipo_actual = typeFactory.getChar(bits);
      break;
    }

    case Tt::BOOL_TYPE: {
      get();
      tipo_actual = typeFactory.getBoolean();
      break;
    }

    case Tt::IDENTIFICADOR: {
      std::string name = get().lexema;
      std::cout << "[224, parser.cpp] name: '" << name << "'\n";

      InfoStruct*        struct_info         = tablas.buscarStruct(name);
      InfoTemplateParam* template_param_info = tablas.buscarTemplateParam(name);
      InfoTemplate*      template_info       = tablas.buscarTemplate(name);

      if (struct_info != nullptr) {
        std::cout << "[233, parser.cpp] struct_info\n";
        tipo_actual = typeFactory.getStruct(struct_info);

      } else if (template_param_info != nullptr) {
        std::cout << "[237, parser.cpp] template_param_info\n";
        tipo_actual = typeFactory.getTemplateParam(name);

      } else if (template_info != nullptr) {
        std::cout << "[241, parser.cpp] template_info\n";
        tipo_actual = typeFactory.getUnresolved(name);

      } else {
        std::cout << "[245, parser.cpp] else\n";
        tipo_actual = typeFactory.getUnresolved(name);
      }


      //else {
      //  InfoVariable tipo_pendiente;
      //  tipo_pendiente.tipo = Dt(typeFactory.getUnresolved(t_base.lexema));
      //  while (peek().tipo == Tt::ASTERISCO || peek().tipo == Tt::POTENCIA) {
      //    get();
      //    if (peek().tipo == Tt::ASTERISCO) { get(); }
      //  }
      //  return tipo_pendiente;
      //}

      std::cout << "[252, parser.cpp] <>\n";

      if (peek().tipo == Tt::MENOR) {
        get();
        std::vector<Dt> template_args;

        while (peek().tipo != Tt::MAYOR && peek().tipo != Tt::EOF_TT) {
          template_args.push_back(parsearTipo().tipo);
          if (peek().tipo == Tt::COMA) { get(); }

        }

        check(Tt::MAYOR);

        std::cout << "[266, parser.cpp] getTemplateInstance name: '" << name << "'\n";
        tipo_actual = typeFactory.getTemplateInstance(name, template_args);

      }

      break;

    }

    case Tt::STRING_TYPE: {
      get();
      tipo_actual = typeFactory.getString();
      break;
    }

    default: { //...
      std::cout << "[271, parser.cpp] Type not implemented: " << peek().lexema << '\n';
      exit(1);
      break;
    }

  }

  while (peek().tipo == Tt::ASTERISCO || peek().tipo == Tt::POTENCIA) {

    if (peek().tipo == Tt::ASTERISCO) {
      tipo_actual = typeFactory.getPointer(tipo_actual);

    } else {
      tipo_actual = typeFactory.getPointer(typeFactory.getPointer(tipo_actual));

    }

    get();

  }

  std::cout << "[280, parser.cpp] Tipo: " << tipo_actual->toString() << '\n';
  info.tipo = Dt(tipo_actual);
  return info;

}

std::unique_ptr<Expresion> Parser::parsearCasteo() {
  std::cerr << "[287 parser.cpp] NO IMPLEMENTADO (parsearCasteo)\n";
  exit(1);
}

// --- Parseo de Literales ---

std::unique_ptr<Expresion> Parser::parsearRangoOArray() {

  if (peek().tipo == Tt::CORCH_R) { // "[]" An empty array
    get();
    std::vector<std::unique_ptr<Expresion>> elementos;
    return std::make_unique<ExprArray>(std::move(elementos));

  }

  std::unique_ptr<Expresion> primer_elemento = nullptr;

  if (peek().tipo != Tt::DOS_PUNTOS) {
    primer_elemento = parsearExpresion(Pr::MINIMA);

  }

  if (peek().tipo == Tt::CORCH_R) { // [a] Index
    get();

    return std::make_unique<ExprRango>(
      std::move(primer_elemento), std::move(nullptr), std::move(nullptr)

    );
  }

  if (peek().tipo == Tt::DOS_PUNTOS) { // A range
    std::unique_ptr<Expresion> fin  = nullptr;
    std::unique_ptr<Expresion> paso = nullptr;

    get(); // Consumir el primer :

    // Hay fin? [a:B]
    if (peek().tipo != Tt::DOS_PUNTOS && peek().tipo != Tt::CORCH_R) {
      fin = parsearExpresion(Pr::MINIMA);
    }
    if (peek().tipo == Tt::DOS_PUNTOS) {
      get();
    }

    // Hay paso? [a:b:C]
    if (peek().tipo != Tt::DOS_PUNTOS && peek().tipo != Tt::CORCH_R) {
      paso = parsearExpresion(Pr::MINIMA);
    }

    check(Tt::CORCH_R);

    return std::make_unique<ExprRango>(
      std::move(primer_elemento), std::move(fin), std::move(paso)
    );

  } else { // Un array
    std::vector<std::unique_ptr<Expresion>> contenido_array;
    contenido_array.push_back(std::move(primer_elemento));

    while (peek().tipo == Tt::COMA) {
      get();

      if (peek().tipo == Tt::CORCH_R) { // [... ,]
        break;
      }

      std::unique_ptr<Expresion> item = parsearExpresion(Pr::MINIMA);
      contenido_array.push_back(std::move(item));
    }

    check(Tt::CORCH_R);

    return std::make_unique<ExprArray>(std::move(contenido_array));

  }

}

std::unique_ptr<Expresion> Parser::parsearRango() {

  std::unique_ptr<Expresion> inicio = nullptr;
  std::unique_ptr<Expresion> fin    = nullptr;
  std::unique_ptr<Expresion> paso   = nullptr;

  // 1. Hay inicio? [A:]
  if (peek().tipo != Tt::DOS_PUNTOS) {
    inicio = parsearExpresion(Pr::MINIMA);

  }

  if (peek().tipo == Tt::DOS_PUNTOS) {
    get();

  }

  // 2. Hay fin? [a:B]
  if (peek().tipo != Tt::DOS_PUNTOS && peek().tipo != Tt::CORCH_R) {
    fin = parsearExpresion(Pr::MINIMA);

  }

  if (peek().tipo == Tt::DOS_PUNTOS) {
    get();

  }

  // 3. Hay paso? [a:b:C]
  if (peek().tipo != Tt::DOS_PUNTOS && peek().tipo != Tt::CORCH_R) {
    paso = parsearExpresion(Pr::MINIMA);

  }

  check(Tt::CORCH_R);
  return std::make_unique<ExprRango>(
    std::move(inicio), std::move(fin), std::move(paso)

  );

}

std::unique_ptr<Expresion> Parser::parsearAcceso(std::unique_ptr<Expresion> contenedor) {

  std::unique_ptr<Expresion> indice_o_rango = parsearRango();

  return std::make_unique<ExprAcceso>(
    std::move(contenedor),
    std::move(indice_o_rango)
  );

}

std::unique_ptr<Sentencia> Parser::parsearEscritura() {
  check(Tt::ESCRITURA);
  Token alias = check(Tt::IDENTIFICADOR);
  check(Tt::IGUAL_ASIG);
  Token original = get();
  check(Tt::PUNTO_COMA);
 
  aliasLexicos[alias.lexema] = original.tipo;
 
  return std::make_unique<SentenciaEscritura>(alias.lexema, original.tipo);

}

std::pair<std::string, std::string> Parser::partirLexemaNum(std::string lexema) {
  std::string digitosValidos = "0123456789_.e";

  if (lexema.size() > 2 && lexema[0] == '0') {
    if        (lexema[1] == 'x') {
      digitosValidos += "xABCDEF";
    } else if (lexema[1] == 'o') {
      digitosValidos += 'o';
    } else if (lexema[1] == 'b') {
      digitosValidos += 'b';
    }
  }

  size_t corte = lexema.find_first_not_of(digitosValidos);

  if (corte == std::string::npos) { return {lexema, ""}; }

  return {lexema.substr(0, corte), lexema.substr(corte)};

}

std::unique_ptr<Sentencia> Parser::parsearDeclaracionVar() {
  std::cout << "[459, parser.cpp] parsearDeclaracionVar\n";

  InfoVariable tipo = parsearTipo();

  std::cout << "[488, parser.cpp] tipo.tipo.tipoString(): '" << tipo.tipo.tipoString() << "'\n";

  Token nombre = check(Tt::IDENTIFICADOR); // Type Var
  std::unique_ptr<Expresion> size = nullptr;

  if (peek().tipo == Tt::CORCH_L) { // [ Expr ]
    get();
    size = parsearExpresion(Pr::MINIMA);
    check(Tt::CORCH_R);

  }

  std::unique_ptr<Expresion> valor = nullptr;

  if (peek().tipo == Tt::IGUAL_ASIG) { // = Expr ;
    get();
    valor = parsearExpresion(Pr::MINIMA);

  }

  // else: Type Var ;

  check(Tt::PUNTO_COMA);

  return std::make_unique<SentenciaAsignarVar>(nombre.lexema, tipo, std::move(valor), std::move(size));

}

std::unique_ptr<Sentencia> Parser::parsearSentenciaExpresion() {
  std::cout << "[490, parser.cpp] parsearSentenciaExpresion\n";
  std::unique_ptr<Expresion> izquierda = parsearExpresion(Pr::MINIMA);
  std::cout << "[492, parser.cpp]\n";

  if (peek().tipo == Tt::IGUAL_ASIG) { // ... = ...;
    get();

    std::unique_ptr<Expresion> derecha = parsearExpresion(Pr::MINIMA);
    check(Tt::PUNTO_COMA);

    return std::make_unique<SentenciaReasignacionVar>(std::move(izquierda), std::move(derecha));

  }

  // If you get a "Expected ';', got [something else]"
  // Please check this line and inc the counter
  // 27. Yikes.
  // Also, the bug you are looking for is most certainly not in this function.
  // Cheers

  check(Tt::PUNTO_COMA); // ...;

  return std::make_unique<SentenciaExpr>(std::move(izquierda));

}

std::unique_ptr<Sentencia> Parser::parsearBloqSent() {
  std::unique_ptr<Sentencia> rama;
  if (peek().tipo == Tt::LLAVE_L) { // Es un bloque
    rama = parsearBloque();

  } else {
    rama = parsearSentencia();

  }
  return rama;

}

std::vector<BT> Parser::parsearTraits() {
  std::vector<BT> traits;

  while (peek().tipo == Tt::TATETI) {
    get();
    traits.push_back(stringToTrait(get().lexema));

  }

  return traits;

}

std::unique_ptr<Sentencia> Parser::parsearBloque() {

  std::vector<BT> traits = parsearTraits();

  check(Tt::LLAVE_L);

  auto bloque = std::make_unique<Bloque>();

  while (peek().tipo != Tt::LLAVE_R && peek().tipo != Tt::EOF_TT) {
    bloque->agregarSentencia(parsearSentencia());

  }

  bloque->traits = traits;

  check(Tt::LLAVE_R);
  return bloque;

}

bool Parser::isVarDecl() {
  size_t offset = 1;

  while (true) {
    if      (peek(offset).tipo == Tt::IDENTIFICADOR)                                      { return true ; }
    else if (peek(offset).tipo == Tt::ASTERISCO    || peek(offset).tipo == Tt::AMPERSAND) { offset++    ; }
    else if (peek(offset).tipo == Tt::POTENCIA     || peek(offset).tipo == Tt::Y_LOGICO ) { offset += 2 ; }
    else                                                                                  { return false; }
  }

}

std::unique_ptr<Sentencia> Parser::parsearSentencia() {
  std::cout << "[574, parser.cpp] parsearSentencia\n";

  while (isStructural(peek().tipo) && peek().tipo != Tt::EOF_TT && peek().tipo != Tt::PUNTO_COMA) {
    Token t = get();
    errHandler.report(CE::E_UNEXPECTED_TOKEN, t.pos, t.lexema);
    std::cout << "[579, parser.cpp]\n";

  }

  Tt actual = peek().tipo;
  std::cout << "[584, parser.cpp] peek().lexema: '" << peek().lexema << "'\n";

  if (actual == Tt::IF       ) { return parsearSi           (); }
  if (actual == Tt::ELSE     ) { return parsearSino         (); }
  if (actual == Tt::WHILE    ) { return parsearMientras     (); }

  if (actual == Tt::BREAK    ) { return parsearBreak        (); }
  if (actual == Tt::CONTINUE ) { return parsearContinue     (); }
  if (actual == Tt::REDO     ) { return parsearRedo         (); }

  if (actual == Tt::LLAVE_L  ) { return parsearBloque       (); }
  if (actual == Tt::TATETI   ) { return parsearBloque       (); }

  if (actual == Tt::FUNC     ) { return parsearFuncDecl     (); }
  if (actual == Tt::PURE     ) { return parsearFuncDecl     (); }
  if (actual == Tt::RETURN   ) { return parsearReturn       (); }

  if (actual == Tt::STRUCT   ) { return parsearStruct       (); }

  if (actual == Tt::ESCRITURA) { return parsearEscritura    (); }
  if (actual == Tt::ARCANE   ) { return parsearArcano       (); }
  if (actual == Tt::PREGUNTA ) { return parsearMetaDirectiva(); }

  if (actual == Tt::TEMPLATE ) { return parsearTemplate     (); }

  if (actual == Tt::INCLUDE  ) { return parsearInclude      (); }

  // Si empieza con un tipo de dato, es una delaración
  if (isType(peek()) || esInfiere(actual)) {
    std::cout << "[611, parser.cpp]\n";
    return parsearDeclaracionVar();

  }
 
  if (actual == Tt::IDENTIFICADOR) {
    std::cout << "[617, parser.cpp]\n";
    // Arcanos
    if (contextoArcanos.esKeywordArcano(peek().lexema)) {
      std::cout << "[620, parser.cpp]\n";
      return parsearLlamadaArcano();
    }

    // Tipos
    if (isVarDecl()) {
      std::cout << "[640, parser.cpp]\n";
      return parsearDeclaracionVar();
    }

    //if (peek(1).tipo == Tt::IDENTIFICADOR) {
    //  std::cout << "[626, parser.cpp]\n";
    //  return parsearDeclaracionVar();
    //}

  }
 
  std::cout << "[631, parser.cpp]\n";
  // Por defecto, es una expresión
  return parsearSentenciaExpresion();

}

std::unique_ptr<Sentencia> Parser::parsearSi() {
  check(Tt::IF);
  check(Tt::PAREN_L);

  auto condicion = parsearExpresion(Pr::MINIMA);

  check(Tt::PAREN_R);

  // La rama puede ser un bloque {...} o una sentencia

  std::unique_ptr<Sentencia> rama_si;
  rama_si = parsearBloqSent();
 
  std::unique_ptr<Sentencia> rama_sino = nullptr;

  if (peek().tipo == Tt::ELSE) {
    rama_sino = parsearSino();
  }

  return std::make_unique<SentenciaSi>(std::move(condicion), std::move(rama_si), std::move(rama_sino));

}

std::unique_ptr<Sentencia> Parser::parsearSino() {
  check(Tt::ELSE);
  return std::make_unique<SentenciaSino>(parsearBloqSent());

}

std::unique_ptr<Sentencia> Parser::parsearMientras() {
  check(Tt::WHILE);
  check(Tt::PAREN_L);

  auto condicion = parsearExpresion(Pr::MINIMA);

  check(Tt::PAREN_R);

  std::unique_ptr<Sentencia> rama_while;
  rama_while = parsearBloqSent();

  std::unique_ptr<Sentencia> rama_sino = nullptr;

  if (peek().tipo == Tt::ELSE) {
    get();
    rama_sino = parsearBloqSent();

  }

  return std::make_unique<SentenciaMientras>(
       std::move(condicion),
    std::move(rama_while),
     std::move(rama_sino)
  );

}

std::unique_ptr<Sentencia> Parser::parsearBreak() {
  check(Tt::BREAK);
  check(Tt::PUNTO_COMA);

  return std::make_unique<SentenciaBreak>();
}

std::unique_ptr<Sentencia> Parser::parsearContinue() {
  check(Tt::CONTINUE);
  check(Tt::PUNTO_COMA);
  return std::make_unique<SentenciaContinue>();

}

std::unique_ptr<Sentencia> Parser::parsearRedo() {
  check(Tt::REDO);
  check(Tt::PUNTO_COMA);
  return std::make_unique<SentenciaRedo>();

}

std::unique_ptr<Sentencia> Parser::parsearReturn() {
  check(Tt::RETURN);
  std::unique_ptr<Expresion> ret_value = parsearExpresion(Pr::MINIMA);
  check(Tt::PUNTO_COMA);

  return std::make_unique<SentenciaReturn>(ret_value->tipo_resuelto, std::move(ret_value));

}

std::vector<std::pair<std::string, InfoVariable>> Parser::parsearFuncArgs(Tt tEnd) {

  std::vector<std::pair<std::string, InfoVariable>> args;
  std::set<std::string> nombre_args;

  while (peek().tipo != tEnd && peek().tipo != Tt::EOF_TT) {

    InfoVariable tipo = parsearTipo();
    Token nombre = check(Tt::IDENTIFICADOR);

    if (nombre_args.count(nombre.lexema)) {
      //... Error, dos argumentos se llaman igual

    } else {
      nombre_args.insert(nombre.lexema);
      args.push_back({nombre.lexema, tipo});

    }

    if (peek().tipo == Tt::COMA) { get(); }

  }

  return args;

}

std::unique_ptr<Sentencia> Parser::parsearFuncDecl() {

  bool es_pure = (coincide({ Tt::FUNC, Tt::PURE }).tipo == Tt::PURE);
  Token i = check(Tt::IDENTIFICADOR);

  check(Tt::PAREN_L);

  std::vector<std::pair<std::string, InfoVariable>> args = parsearFuncArgs();

  check(Tt::PAREN_R);

  check(Tt::FLECHA);

  Dt ret_type = parsearTipo().tipo;

  bool firma;
  if        (peek().tipo == Tt::PUNTO_COMA) {
    firma = true;

  } else if (peek().tipo == Tt::LLAVE_L)    {
    firma = false;

  } else                                    {
    //... Error, peek().tipo was not expected

  }

  std::vector<std::unique_ptr<Sentencia>> cuerpo_func;

  if (firma) { // Firma
    get();

  } else     { // Implementación
    auto bloque = std::unique_ptr<Bloque>(
      static_cast<Bloque*>(parsearBloque().release())
    );

    cuerpo_func = std::move(bloque->instrucciones);

  }

  return std::make_unique<SentenciaFuncDecl>(
    i.lexema,
    es_pure,
    args,
    std::move(cuerpo_func),
    ret_type
  );

}

std::unique_ptr<Expresion> Parser::parsearFunctionCall(std::unique_ptr<Expresion> callee) {
  std::vector<std::pair<std::string, std::unique_ptr<Expresion>>> args;

  if (peek().tipo != Tt::PAREN_R) {
    do {
      std::string nombre_arg = "";
      if (peek().tipo == Tt::IDENTIFICADOR && peek(1).tipo == Tt::IGUAL_ASIG) {
        nombre_arg = get().lexema;
        get(); // =

      }

      args.push_back({nombre_arg, parsearExpresion(Pr::MINIMA)});

      if (peek().tipo == Tt::COMA) {
        get();

      } else {
        break; // No hay más comas, no hay más argumentos

      }
    } while (peek().tipo != Tt::PAREN_R && peek().tipo != Tt::EOF_TT);

  }

  check(Tt::PAREN_R);

  return std::make_unique<ExprFuncCall>(std::move(callee), std::move(args));

}

std::pair<std::string, ReglaArcano> Parser::parsearReglaArcano() { //...
  std::pair<std::string, ReglaArcano> par;

  check(Tt::ARROBA); // @

  par.first = "@" + check(Tt::IDENTIFICADOR).lexema; // @rule_tag

  check(Tt::DOS_PUNTOS); // :

  par.second.keyword = check(Tt::IDENTIFICADOR, Pm::RELAXED).lexema; // Trigger keyword

  check(Tt::CORCH_L);
 
  Token t;

  while (peek().tipo != Tt::CORCH_R && peek().tipo != Tt::EOF_TT) {
    t = get();

    Token comp = t;

    par.second.componentes.push_back(comp);

  }

  check(Tt::CORCH_R);
  check(Tt::PUNTO_COMA);

  return par;

}

std::vector<std::pair<std::string, ReglaArcano>> Parser::parsearReglasArcano() {
  check(Tt::RULES);   // rules
  check(Tt::CORCH_L); // [

  std::vector<std::pair<std::string, ReglaArcano>> rules;
  std::pair<std::string, ReglaArcano> par;

  while (peek().tipo != Tt::CORCH_R && peek().tipo != Tt::EOF_TT) {
    par = parsearReglaArcano();
    rules.push_back(par);
  }

  check(Tt::CORCH_R);    // ]
  check(Tt::PUNTO_COMA); // ;

  return rules;

}

std::unordered_map<std::string, std::vector<EnlaceCadena>> Parser::parsearCadenasArcano(
  const std::vector<std::pair<std::string, ReglaArcano>>& reglasDeclaradas
) {
  auto existeRegla = [&](const std::string& etiqueta) {
    for (const auto& r : reglasDeclaradas) {
      if (r.first == etiqueta) { return true; }
    }
    return false;
  };

  std::unordered_map<std::string, std::vector<EnlaceCadena>> chains;

  check(Tt::CHAINS);
  check(Tt::CORCH_L);

  while (peek().tipo != Tt::CORCH_R && peek().tipo != Tt::EOF_TT) {
    check(Tt::ARROBA);
    std::string parent_rule = "@" + check(Tt::IDENTIFICADOR).lexema;

    if (!existeRegla(parent_rule)) {
      throw std::runtime_error("Error: La regla base '" + parent_rule + "' no existe en rules.");

    }

    check(Tt::FLECHA);

    std::vector<EnlaceCadena> enlaces;

    while (peek().tipo != Tt::PUNTO_COMA && peek().tipo != Tt::EOF_TT) {
      check(Tt::ARROBA);
      std::string target_rule = "@" + check(Tt::IDENTIFICADOR).lexema;

      if (!existeRegla(target_rule)) {
        throw std::runtime_error("Error: La regla encadenada '" + target_rule + "' no existe.");

      }

      bool optional = false;
      if (peek().tipo == Tt::PREGUNTA) {
        get();
        optional = true;
      }

      enlaces.push_back({target_rule, optional});

      if (peek().tipo == Tt::FLECHA) {
        get();
      }

    }

    check(Tt::PUNTO_COMA);
    chains[parent_rule] = std::move(enlaces);

  }

  check(Tt::CORCH_R);
  check(Tt::PUNTO_COMA);

  return chains;

}

std::vector<ArcaneBranch> Parser::parsearCuerpoArcano(
  const std::vector<std::pair<std::string, ReglaArcano>>& reglasDeclaradas
  ) {

  std::vector<ArcaneBranch> ramas_totales;

  auto existe_regla = [&](const std::string& etiqueta) {
    for (const auto& r : reglasDeclaradas) {
      if (r.first == etiqueta) { return true; }
    }
    return false;
  };

  while (peek().tipo == Tt::ARROBA) {
    get(); // @
    Token t_label = check(Tt::IDENTIFICADOR); //rule_tag

    std::string tag_name = "@" + t_label.lexema; // @rule
    if (!existe_regla(tag_name)) {
      throw std::runtime_error("Error: La regla '" + tag_name + "' no ha sido declarada en el bloque rules.");

    }

    check(Tt::LLAVE_L); // {

    while (peek().tipo !=  Tt::LLAVE_R && peek().tipo != Tt::EOF_TT) {
      ArcaneBranch rama_actual;
      rama_actual.rule_tag = tag_name;

      bool es_primer_segmento = true;

      while (peek().tipo != Tt::PUNTO_COMA && peek().tipo != Tt::EOF_TT) {
        ArcaneSegment segmento;
        segmento.br_key = check(Tt::IDENTIFICADOR, Pm::RELAXED).lexema;

        if (peek().tipo == Tt::CORCH_L) { // Argument parsing
          get();
          segmento.br_args = parsearFuncArgs(Tt::CORCH_R);
          check(Tt::CORCH_R);

        }

        if (peek().tipo == Tt::PAREN_L) { // Expressions parsing
          get();

          while (peek().tipo != Tt::PAREN_R && peek().tipo != Tt::EOF_TT) {
            segmento.br_expr.push_back(get().lexema);

            if (peek().tipo == Tt::COMA) { get(); }

          }

          check(Tt::PAREN_R);

        }

        check(Tt::ASIG_BLOQUE);

        segmento.br_cont = parsearBloque(); // { ... }
        rama_actual.segmentos.push_back(std::move(segmento));

        if (peek().tipo != Tt::IDENTIFICADOR && peek().tipo != Tt::PUNTO_COMA) {
          break;
        }

      }

      check(Tt::PUNTO_COMA);

      ramas_totales.push_back(std::move(rama_actual));
    }

    check(Tt::LLAVE_R);

  }

  return ramas_totales;

}

std::unique_ptr<Sentencia> Parser::parsearArcano() {
  check(Tt::ARCANE);
  Token nombre_arcano = check(Tt::IDENTIFICADOR);
  check(Tt::PAREN_L);

  ArcaneDef def;
  def.name = nombre_arcano.lexema;

  while (peek().tipo != Tt::PAREN_R && peek().tipo != Tt::EOF_TT) { // Argumentos
    Token nombre_param = check(Tt::IDENTIFICADOR, Pm::RELAXED);
    check(Tt::DOS_PUNTOS);
    Token t_tipo = get();  // key, expr, code

    TPA tipo = TPA::NULO;
    if      (t_tipo.lexema == "code") { tipo = TPA::CODE; }
    else if (t_tipo.lexema == "expr") { tipo = TPA::EXPR; }
    else if (t_tipo.lexema == "key" ) { tipo = TPA::KEY ; }
    else if (t_tipo.lexema == "var" ) { tipo = TPA::VAR ; }

    def.args.push_back({nombre_param.lexema, tipo});

    if (peek().tipo == Tt::COMA) { get(); }

  }

  check(Tt::PAREN_R);
  check(Tt::LLAVE_L);

  // Reglas
  std::vector<std::pair<std::string, ReglaArcano>> rules = parsearReglasArcano();

  //std::cout << "[896, parser.cpp]\n";

  for (const auto& rule : rules) {
    contextoArcanos.registrarRegla(rule.first, rule.second);
    //std::cout << rule.first << ' ';
    def.rules.push_back(rule.second);

  }

  //std::cout << '\n';

  if (peek().tipo == Tt::CHAINS) {
    def.chains = parsearCadenasArcano(rules);
  }

  // Cuerpo
  def.branches = parsearCuerpoArcano(rules);

  check(Tt::LLAVE_R);

  contextoArcanos.registrarDefinicion(nombre_arcano.lexema, def);

  return std::make_unique<SentenciaArcano>(std::move(def));

}

std::unique_ptr<Sentencia> Parser::parsearLlamadaArcano(bool checkSc) {
  Token trigger = check(Tt::IDENTIFICADOR, Pm::RELAXED);
  std::string key = trigger.lexema;

  ArcaneDef& def = contextoArcanos.buscarDefinicionPorKeyword(key);

  std::vector<std::unique_ptr<Expresion>> local_args;

  if (peek().tipo == Tt::CORCH_L) {

    get();

    while (peek().tipo != Tt::CORCH_R && peek().tipo != Tt::EOF_TT) {

      local_args.push_back(parsearExpresion(Pr::MINIMA));

      if (peek().tipo == Tt::COMA) { get(); }

    }

    check(Tt::CORCH_R);

  }

  std::vector<std::pair<size_t, ReglaArcano>> posibles_reglas;

  for (size_t i = 0; i < def.branches.size(); ++i) {

    auto& branch = def.branches[i];
    ReglaArcano rule = contextoArcanos.obtenerRegla(branch.rule_tag);
    auto& primer_seg = branch.segmentos[0];

    if (rule.keyword == key && primer_seg.br_args.size() == local_args.size()) {
      posibles_reglas.push_back({i, rule});

    }

  }

  if (posibles_reglas.empty()) {

    throw std::runtime_error("Firma no encontrada para '" + key + "'"          +
                                  " con [" + std::to_string(local_args.size()) +
                                  "] argumentos."
    );

  }

  std::unordered_map<std::string, std::unique_ptr<Sentencia>> mapa_args;
  std::unordered_map<std::string, std::unique_ptr<Sentencia>> mapa_code;
  std::unordered_map<std::string, std::unique_ptr<Sentencia>> mapa_expr;
  std::unordered_map<std::string, std::unique_ptr<Expresion>> mapa_vars;

  for (size_t i = 0; i < local_args.size(); ++i) {
    std::string nombre_local = def.branches[posibles_reglas[0].first].segmentos[0].br_args[i].first;
    mapa_args[nombre_local]  = std::make_unique<SentenciaExpr>(std::move(local_args[i]));
  }

  auto obtenerTipo = [&](const std::string& lex) -> std::pair<TPA, std::string> {
    for (const auto& arg : def.args) {
      if (arg.contenido == lex) { return { arg.tipo_dato, arg.contenido}; }

    }

    return {TPA::NULO, lex};

  };

  size_t comp_idx = 0;
  std::pair<size_t, ReglaArcano> rule;
  bool done = false;

  bool grupo_expr = false;

  while (!posibles_reglas.empty()) {

    std::vector<std::pair<size_t, ReglaArcano>> reglas_terminadas;
    std::vector<std::pair<size_t, ReglaArcano>> reglas_activas   ;

    for (auto& r : posibles_reglas) {
      if (comp_idx == r.second.componentes.size()) {
        reglas_terminadas.push_back(r);

      } else {
        reglas_activas   .push_back(r);

      }

    }

    Token sig = peek();

    if (!reglas_terminadas.empty()) {
      if (reglas_activas.empty() || sig.tipo == Tt::PUNTO_COMA) {
        rule = reglas_terminadas[0];
        done = true;
        break;

      }

    }

    if (reglas_activas.empty()) { break; }

    std::vector<std::pair<size_t, ReglaArcano>> restantes;

    for (auto& r : reglas_activas) {
      auto [tipo, nombre] = obtenerTipo(r.second.componentes[comp_idx].lexema);
      bool queda = false;

      if      (tipo == TPA::CODE && sig.tipo    == Tt::LLAVE_L)              { queda = true; }
      else if (tipo == TPA::EXPR && (grupo_expr || sig.tipo == Tt::PAREN_L)) { queda = true; }
      else if (tipo == TPA::KEY  && sig.lexema  == nombre     )              { queda = true; }
      else if (tipo == TPA::VAR  && sig.tipo    == Tt::IDENTIFICADOR)        { queda = true; }
      else if (tipo == TPA::NULO && (sig.lexema == nombre ||
              sig.tipo == r.second.componentes[comp_idx].tipo))              { queda = true; }

      if (queda) { restantes.push_back(r); }

    }

    if (restantes.empty()) {
      if (!reglas_terminadas.empty()) {
        // The longer ones dont fit, but a shorter one does
        rule = reglas_terminadas[0];
        done = true;
        break;

      } else {
        throw std::runtime_error("Error de sintaxis: Las estructuras provistas no coinciden con ninguna regla para '" + key + "'.");

      }

    }

    posibles_reglas = restantes;

    auto [tipo, nombre] = obtenerTipo(posibles_reglas[0].second.componentes[comp_idx].lexema);

    if        (tipo == TPA::CODE) {
      mapa_code[nombre] = parsearBloque();

    } else if (tipo == TPA::EXPR) {

      if (!grupo_expr) {
        check(Tt::PAREN_L);
      }

      mapa_expr[nombre] = std::make_unique<SentenciaExpr>(parsearExpresion(Pr::MINIMA));

      if (peek().tipo == Tt::PUNTO_COMA) {
        get();
        grupo_expr = true;

      } else if (peek().tipo == Tt::PAREN_R) {
        get();
        grupo_expr = false;

      }

    } else if (tipo == TPA::KEY) {
      get();

    } else if (tipo == TPA::VAR) {
      mapa_vars[nombre] = std::make_unique<ExprVariable>(check(Tt::IDENTIFICADOR).lexema);

    }

    comp_idx++;

  }

  if (!done && posibles_reglas.empty()) {
    throw std::runtime_error("Firma incompleta o no encontrada para '" + key + "'.");

  }

  const auto& matched_branch = def.branches[rule.first];

  bool necesita_sc = true;

  if (!rule.second.componentes.empty()) {
    auto [tipo, nombre] = obtenerTipo(rule.second.componentes.back().lexema);
    if (tipo == TPA::CODE) { necesita_sc = false; }
  }

  std::vector<std::unique_ptr<SentenciaLlamadaArcano>> chains;

  auto it = def.chains.find(matched_branch.rule_tag);
  if (it != def.chains.end()) {
    for (const auto& enlace : it->second) {
      ReglaArcano targete_info = contextoArcanos.obtenerRegla(enlace.target_rule);
      if (peek().lexema == targete_info.keyword) {
        auto sub = parsearLlamadaArcano(false);
        auto sub_arcano = std::unique_ptr<SentenciaLlamadaArcano>(static_cast<SentenciaLlamadaArcano*>(sub.release()));

        ReglaArcano last_rule = contextoArcanos.obtenerRegla(sub_arcano->rule_tag);
        necesita_sc = true;
        if (!last_rule.componentes.empty()) {
          auto [t, _] = obtenerTipo(last_rule.componentes.back().lexema);
          if (t == TPA::CODE) { necesita_sc = false; }
        }
        chains.push_back(std::move(sub_arcano));
      }
    }
  }

  if (necesita_sc && checkSc) {
    check(Tt::PUNTO_COMA);
  }

  return std::make_unique<SentenciaLlamadaArcano>(
    key,
    matched_branch.rule_tag,
    std::move(mapa_args),
    std::move(mapa_code),
    std::move(mapa_expr),
    std::move(mapa_vars),
    std::move(chains)   ,
    rule.first
  );

}

std::unique_ptr<Sentencia> Parser::parsearMetaDirectiva() {
  check(Tt::PREGUNTA);
  std::string name = check(Tt::IDENTIFICADOR).lexema;
  MetaID id = metaStringToID(name);

  check(Tt::PAREN_L);
  std::vector<std::unique_ptr<Expresion>> args;

  while (peek().tipo != Tt::PAREN_R && peek().tipo != Tt::EOF_TT) {
    std::unique_ptr<Expresion> arg = parsearExpresion(Pr::MINIMA);
    args.push_back(std::move(arg));
    if (peek().tipo == Tt::COMA) { get(); }

  }

  check(Tt::PAREN_R);
  auto bloque = parsearBloque();

  return std::make_unique<SentenciaMetaDirective>(
    id,
    std::move(args),
    std::move(bloque)
  );

}

std::unique_ptr<Sentencia> Parser::parsearStruct() {
  check(Tt::STRUCT);
  std::string name = check(Tt::IDENTIFICADOR).lexema;

  InfoStruct info_temp;
  info_temp.nombre = name;

  check(Tt::LLAVE_L);

  std::vector<std::unique_ptr<Sentencia>> propiedades;
  std::vector<std::unique_ptr<Sentencia>> metodos    ;

  InfoStruct info;
  info.nombre = name;

  while (peek().tipo != Tt::LLAVE_R && peek().tipo != Tt::EOF_TT) {

    if (isType(peek())) {
      auto nodo_var = parsearDeclaracionVar();

      auto* nodo_decl = static_cast<SentenciaAsignarVar*>(nodo_var.get());

      info.propiedades[nodo_decl->nombre] = nodo_decl->tipo_explicito;
      info.orden_props.push_back(nodo_decl->nombre);

      propiedades.push_back(std::move(nodo_var));

    } else if (peek().tipo == Tt::FUNC) {
      auto nodo_func = parsearFuncDecl();
      auto* nodo_decl = static_cast<SentenciaFuncDecl*>(nodo_func.get());

      Dt tipo_this;

      InfoFuncion info_func;
      info_func.nombre = nodo_decl->nombre_func;
      info_func.tipo_retorno = nodo_decl->ret_type;
      info_func.tipos_parametros = nodo_decl->args_type;
      info_func.is_external = false;

      std::vector<Dt> tipos_args;
      for (const auto& a : nodo_decl->args_type) {
        tipos_args.push_back(a.second.tipo);
      }

      //std::string firma = name + "_" + generarFirma(nodo_decl->nombre_func, tipos_args);
      //std::cout << "[1362, parser.cpp] firma: '" << firma << "'\n";

      //nodo_decl->nombre_func = firma;
      //info.metodos[firma] = std::move(info_func);
      metodos.push_back(std::move(nodo_func));

    } else if (peek().tipo == Tt::STRUCT) {
      propiedades.push_back(parsearStruct());

    } else if (peek().tipo == Tt::IDENTIFICADOR) {
      Token t = get();
      check(Tt::PUNTO_COMA);

      auto placeholder = std::make_unique<SentenciaExpr>(
        std::make_unique<ExprVariable>(t.lexema)
      );

      propiedades.push_back(std::move(placeholder));

    } else { // Error
      std::cerr << "[1374, parser.cpp] Error, lexema: '" << peek().lexema << "'\n";
      std::cerr << nombreTipo(peek().tipo) << '\n';
      throw std::runtime_error("[1374, parser.cpp] Error: structs");

    }

  }

  check(Tt::LLAVE_R);
  check(Tt::PUNTO_COMA);

  return std::make_unique<SentenciaStruct>(name, std::move(propiedades), std::move(metodos));

}

std::unique_ptr<Expresion> Parser::parsearInitList() {
  std::vector<ArgumentoInit> args;
  bool seen_named = false;

  while (peek().tipo != Tt::LLAVE_R && peek().tipo != Tt::EOF_TT) {
    std::optional<std::string> name = std::nullopt;

    if (peek().tipo == Tt::IDENTIFICADOR && peek(1).tipo == Tt::DOS_PUNTOS) {
      seen_named = true;
      name = get().lexema;
      get(); // :

    } else if (seen_named) {
      errHandler.report(CE::E_STRUCT_POS_AFTER_NAMED, peek().pos);
      return std::make_unique<ErrorNode>();

    }

    auto valor = parsearExpresion(Pr::MINIMA);
    args.push_back(ArgumentoInit{name, std::move(valor)});

    if (peek().tipo != Tt::LLAVE_R) {
      check(Tt::COMA);

    }

  }

  check(Tt::LLAVE_R);

  return std::make_unique<ExprInitList>(std::move(args));

}

std::unique_ptr<Sentencia> Parser::parsearTemplate() {
  std::cout << "[1448, parser.cpp] parsearTemplate\n";
  check(Tt::TEMPLATE);
  check(Tt::MENOR);

  tablas.entrarScopeTemplate();

  std::vector<std::pair<std::string, std::variant<InfoTemplateParam, InfoVariable>>> args;

  while (peek().tipo != Tt::MAYOR && peek().tipo != Tt::EOF_TT) {
    std::cout << "[1457, parser.cpp] peek().lexema: '" << peek().lexema << "'\n";
    if (peek().tipo == Tt::TYPE) {
      get();
      InfoTemplateParam info_pt;

      info_pt.nombre = check(Tt::IDENTIFICADOR).lexema;

      if (peek().tipo == Tt::IGUAL_ASIG) {
        get();
        info_pt.default_type = parsearTipo().tipo;

      }

      args.push_back({ info_pt.nombre, info_pt });
      tablas.añadirTemplateParam(info_pt.nombre, info_pt);

    } else if (isType(peek())) {
      InfoVariable type = parsearTipo();
      args.push_back({ check(Tt::IDENTIFICADOR).lexema, type });

    }

    if (peek().tipo == Tt::COMA) {
      get();
    }

  }

  check(Tt::MAYOR);

  auto statement = parsearSentencia();
  std::string template_name;

  if (auto* struct_node = dynamic_cast<SentenciaStruct*>(statement.get())) {
    template_name = struct_node->name;

  } else if (auto* func_node = dynamic_cast<SentenciaFuncDecl*>(statement.get())) {
    template_name = func_node->nombre_func;

  } else {
    throw std::runtime_error("Error: Templates solo con structs o funciones");

  }

  InfoTemplate info_tmpl;
  info_tmpl.nombre = template_name;
  info_tmpl.args   = args;
  info_tmpl.ast    = statement->clonar();

  tablas.añadirTemplate(template_name, std::move(info_tmpl));
  tablas.salirScopeTemplate();

  return std::make_unique<SentenciaTemplate>(template_name, std::move(args), std::move(statement));

}

std::unique_ptr<Sentencia> Parser::parsearInclude() {

  check(Tt::INCLUDE);

  std::string path = "";
  bool is_system_header = false;

  if (peek().tipo == Tt::MENOR) { // include <...>
    get();
    is_system_header = true;
    path = check(Tt::IDENTIFICADOR, Pm::RELAXED).lexema;
    path += check(Tt::PUNTO).lexema;
    path += check(Tt::IDENTIFICADOR, Pm::RELAXED).lexema;
    check(Tt::MAYOR);

  } //...

  return std::make_unique<SentenciaInclude>(std::move(path), is_system_header);

}

// Precedencias de las operaciones
Pr Parser::obtenerPrecedencia(Tt tipo) {
  switch (tipo) {

    // --- Ternario ---
    case Tt::PREGUNTA      :
      return Pr::TERNARIO  ;

    // --- Lógicos ---
    case Tt::O_LOGICO      :
      return Pr::LOGICA_O  ;

    case Tt::XO_LOGICO     :
      return Pr::LOGICA_XOR;

    case Tt::Y_LOGICO      :
      return Pr::LOGICA_Y  ;

    // --- Bitwise ---
    case Tt::O_BITWISE  :
      return Pr::BIT_O  ;

    case Tt::XO_BITWISE :
      return Pr::BIT_XOR;

    case Tt::AMPERSAND  :
    case Tt::Y_BITWISE  :
      return Pr::BIT_Y  ;

    // --- Igualdad --- 
    case Tt::IGUAL_CMP   :
    case Tt::DISTINTO    :
      return Pr::IGUALDAD;

    // --- Relacionales ---
    case Tt::MENOR         :
    case Tt::MAYOR         :
    case Tt::MAYOR_IGUAL   :
    case Tt::MENOR_IGUAL   :
      return Pr::RELACIONAL;

    // --- Shift ---
    case Tt::BITWISE_L:
    case Tt::BITWISE_R:
      return Pr::SHIFT;

    // --- Aritméticos ---
    case Tt::MAS     :
    case Tt::MENOS   :
      return Pr::SUMA;

    case Tt::ASTERISCO:
    case Tt::DIV      :
    case Tt::MODULO   :
      return Pr::MULT ;

    case Tt::POTENCIA    :
    case Tt::RAIZ        :
      return Pr::POTENCIA;

    case Tt::INCREMENTAR:
    case Tt::DECREMENTAR:
      return Pr::SUFIJO ;

    case Tt::SWAP    :
      return Pr::SWAP;

    // --- Extra ---
    case Tt::PUNTO     :
    case Tt::CORCH_L   :
      return Pr::ACCESO;

    case Tt::PAREN_L    :
      return Pr::LLAMADA;

    default            :
      return Pr::MINIMA;
  }

}

std::unique_ptr<Expresion> Parser::parsearPrefijo() {
  Token t = peek();

  std::cout << "[1561, parser.cpp]\n";
  std::cout << t.lexema << '\n';
  std::cout << nombreTipo(t.tipo) << '\n';

  // Error cases
  switch (t.tipo) { //...
    case Tt::PAREN_R:
    case Tt::LLAVE_R:
    case Tt::PUNTO_COMA: {
      errHandler.report(CE::E_EXPECTED_EXPRESSION, peek().pos, nombreTipo(t.tipo));
      return std::make_unique<ErrorNode>();
    }

    default: { break; }

  }

  //if (isType(t)) {
  //  InfoVariable info = parsearTipo();
  //  check(Tt::LLAVE_L);
  //  auto nodo_init_list = parsearInitList();
  //  nodo_init_list->tipo_resuelto = info.tipo;
  //  return nodo_init_list;
  //}

  get();

  // Atom cases
  switch (t.tipo) { //...
    case Tt::NUMERO: {
      auto [num, suf] = partirLexemaNum(t.lexema);
      return std::make_unique<ExprLiteral>(NumberData{num, suf});
    }

    case Tt::CHAR: {
      return std::make_unique<ExprLiteral>(CharData{&t.lexema[0]});
    }

    case Tt::TRUE :
    case Tt::FALSE: {
      return std::make_unique<ExprLiteral>(BooleanData{t.lexema});
    }

    case Tt::ARROBA: {
      Token t = check(Tt::IDENTIFICADOR);
      return std::make_unique<ExprLiteral>(RuleData{"@" + t.lexema});
    }

    case Tt::STRING: {
      return std::make_unique<ExprLiteral>(StringData{t.lexema});
    }

    case Tt::IDENTIFICADOR: {

      if (peek().tipo == Tt::LLAVE_L) { //... This dont belong here
        Dt tipo_struct = Dt(typeFactory.getUnresolved(t.lexema));
        get();

        auto nodo_init_list = parsearInitList();
        nodo_init_list->tipo_resuelto = tipo_struct;
        return nodo_init_list;

      }

      return std::make_unique<ExprVariable>(t.lexema);

    }

    case Tt::PAREN_L: {
      auto expr = parsearExpresion(Pr::MINIMA);
      check(Tt::PAREN_R);
      return expr;
    }

    case Tt::LLAVE_L: {
      return parsearInitList();
    }

    case Tt::CORCH_L: {
      return parsearRangoOArray();
    }

    default: {
      break; // No es un átomo
    }

  }

  if (t.tipo == Tt::POTENCIA) { // **
    auto inner = parsearExpresion(Pr::PREFIJO);
    auto primera_deref = std::make_unique<ExprUnaria>(TipoOperador::PTR_DEREF, std::move(inner), true);
    return std::make_unique<ExprUnaria>(TipoOperador::PTR_DEREF, std::move(primera_deref), true);

  }

  if (t.tipo == Tt::Y_LOGICO) { // &&
    auto inner = parsearExpresion(Pr::PREFIJO);
    auto primera_ref = std::make_unique<ExprUnaria>(TipoOperador::PTR_REF, std::move(inner), true);
    return std::make_unique<ExprUnaria>(TipoOperador::PTR_REF, std::move(primera_ref), true);

  }

  TipoOperador op;

  switch (t.tipo) {
    case Tt::MAS        : {op = TipoOperador::SUMA      ; break; }
    case Tt::MENOS      : {op = TipoOperador::RESTA     ; break; }
    case Tt::NO_LOGICO  : {op = TipoOperador::LOGICO_NO ; break; }
    case Tt::NO_BITWISE : {op = TipoOperador::BITWISE_NO; break; }
    case Tt::INCREMENTAR: {op = TipoOperador::INC_PREF  ; break; }
    case Tt::DECREMENTAR: {op = TipoOperador::DEC_PREF  ; break; }
    case Tt::ASTERISCO  : {op = TipoOperador::PTR_DEREF ; break; }
    case Tt::AMPERSAND  : {op = TipoOperador::PTR_REF   ; break; }

    default: {
        std::cerr << "[1703, parser.cpp]\n";
        std::cerr << "No se esperaba el prefijo '" << t.lexema << "'\n";
        exit(1);
    }
  }

  auto operando = parsearExpresion(Pr::PREFIJO);
  return std::make_unique<ExprUnaria>(op, std::move(operando), true);

}

// Algoritmo de Pratt
std::unique_ptr<Expresion> Parser::parsearExpresion(Pr precedenciaMinima) {
  std::cout << "[1716, parser.cpp] parsearExpresion\n";

  // 1. Empezamos con un átomo (número o id)
  std::unique_ptr<Expresion> izquierda = parsearPrefijo();

  // 2. Comprobamos precedencias
  while (precedenciaMinima < obtenerPrecedencia(peek().tipo)) {
    Token op = get();

    // Casos de Sufijos
    if (op.tipo == Tt::INCREMENTAR || op.tipo == Tt::DECREMENTAR) {
      TipoOperador operador = (op.tipo == Tt::INCREMENTAR) ? TipoOperador::INC_SUF: TipoOperador::DEC_SUF;
      izquierda = std::make_unique<ExprUnaria>(
          operador, std::move(izquierda), false);
      continue;
    }

    // Casting
    /*
      (type) C-style

      {type} Static

      [type] Reinterpret

      |type| Bit-Cast

    */

    // C-Style
    if (op.tipo == Tt::PAREN_L && isType(peek())) {
      izquierda = parsearCasteo();
      continue;
    }

    if (op.tipo == Tt::CORCH_L) {
      izquierda = parsearAcceso(std::move(izquierda));
      continue;
    }

    // Function call
    if (op.tipo == Tt::PAREN_L) {
      izquierda = parsearFunctionCall(std::move(izquierda));
      continue;
    }

    if (op.tipo == Tt::PUNTO) {
      auto propiedad = check(Tt::IDENTIFICADOR).lexema;
      izquierda = std::make_unique<ExprAccesoPunto>(std::move(izquierda), std::move(propiedad));
      continue;
    }

    if (op.tipo == Tt::PREGUNTA) {
      auto rama_verdadera = parsearExpresion(Pr::MINIMA);
      check(Tt::DOS_PUNTOS);
      auto rama_falsa = parsearExpresion(Pr::MINIMA);
      izquierda = std::make_unique<ExprTernaria>(
        std::move(izquierda),
        std::move(rama_verdadera),
        std::move(rama_falsa)
      );
      continue;
    }

    // Casos Binarios
    Pr prec_propia = obtenerPrecedencia(op.tipo);
    Pr prec_derecha = (op.tipo == Tt::POTENCIA) ? static_cast<Pr>(prec_propia - 1) : prec_propia;
 
    auto derecha = parsearExpresion(prec_derecha);
 
    // Creamos el nodo binario y lo hacemos la nueva izquierda
    izquierda = std::make_unique<ExprBinaria>(
      convertirEnTipoOperador(op.tipo), std::move(izquierda), std::move(derecha)
    );
  }

  return izquierda;

}

std::vector<std::unique_ptr<Sentencia>> Parser::parsearPrograma() {
  std::vector<std::unique_ptr<Sentencia>> programa;
  while (peek().tipo != Tt::EOF_TT) {
    programa.push_back(parsearSentencia());
  }
  return programa;

}
