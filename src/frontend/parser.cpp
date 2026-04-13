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
  return resolverAlias(tokens[pos]);
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

  std::cerr << "Error: Se esperaba ";
  for (Tt tipo : tipos) {
    std::cerr << nombreTipo(tipo) << ' ';
  }
  std::cerr << '\n';
  exit(1);

}

// We expect the next token to be exactly this one, if its not, then error
Token Parser::check(Tt tipoEsperado) {
  if (peek().tipo == tipoEsperado) {
    return get();
  }

  //... We have to report this error and call the not implemented yet error recovery function
  std::cerr << "Error: Se esperaba " << nombreTipo(tipoEsperado)
            << " pero se encontró " << nombreTipo(peek().tipo)
            << " ('" << peek().lexema << "') en linea " << peek().linea << std::endl;

  std::cout << "Backtrace:\n";
  std::cout << std::stacktrace::current() << '\n';

  exit(1);
}

Parser::Parser(std::vector<Token> t, TypeFactory& tf)
  : tokens(std::move(t)), pos(0), typeFactory(tf) {}

int extraerBits(std::string lexema, int defaultBits) {
  if (lexema == "int" || lexema == "raw" || lexema == "float") { return defaultBits; }

  std::string num;

  for (char c : lexema) {
    if (std::isdigit(c)) { num += c; }
  }

  return (num.empty() ? defaultBits : std::stoi(num));

}

InfoVariable Parser::parsearTipo() {
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
        // Error, dos veces unsigned
      } else {
        es_unsigned = true;
      }
    }
  }

  Token t_base = peek();
  std::cout << "[97 parser.cpp] " << t_base.lexema << '\n';

  switch (t_base.tipo) {

    // --- Tipos Compuestos (ADTs) ---

    case Tt::CORCH_L: { // Morphs
      std::cout << "[104, parser.cpp]\n";
      get();

      std::vector<std::shared_ptr<ArcanaType>> subtipos;

      while (peek().tipo != Tt::CORCH_R) {
        InfoVariable sub_info = parsearTipo();
        subtipos.push_back(sub_info.tipo.valor);

        if (peek().tipo == Tt::COMA) { get(); }

      }

      get();

      std::cout << "[119, parser.cpp]\n";
      tipo_actual = typeFactory.getMorph(subtipos);
      std::cout << "[121, parser.cpp]\n";
      break;

    }

    case Tt::LLAVE_L: { // Shapes
      std::cout << "[127, parser.cpp]\n";
      get();

      std::vector<CampoShape> campos;

      while (peek().tipo != Tt::LLAVE_R) {
        CampoShape campo;

        InfoVariable sub_info = parsearTipo();
        campo.tipo = sub_info.tipo.valor;

        if (peek().tipo == Tt::IDENTIFICADOR) {
          campo.nombre = get().lexema;
        }

        campos.push_back(campo);
        if (peek().tipo == Tt::COMA) { get(); }
      }

      get();

      tipo_actual = typeFactory.getShape(campos);
      break;

    }

    //... What about T1<T2> ?

    // --- Tipos Primitivos ---

    case Tt::VOID: {
      std::cout << "[158, parser.cpp]\n";
      get();
      tipo_actual = typeFactory.getVoid();
      break;
    }

    case Tt::INT: {
      std::cout << "[165, parser.cpp]\n";
      get();
      int bits = extraerBits(t_base.lexema, 32);
      tipo_actual = typeFactory.getInteger(bits, es_unsigned);
      break;
    }

    case Tt::UINT: {
      std::cout << "[173, parser.cpp]\n";
      get();
      int bits = extraerBits(t_base.lexema, 32);
      tipo_actual = typeFactory.getInteger(bits, true);
      break;
    }

    case Tt::FLOAT: {
      std::cout << "[180, parser.cpp]\n";
      get();
      int bits = extraerBits(t_base.lexema, 64);
      tipo_actual = typeFactory.getFloat(bits);
      break;
    }

    default: {
      std::cout << "[189, parser.cpp]\n";
      std::cout << peek().lexema << '\n';
      get(); //...
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

  info.tipo = Dt(tipo_actual);
  return info;

}

std::unique_ptr<Expresion> Parser::parsearCasteo() {
  std::cerr << "[140 parser.cpp] NO IMPLEMENTADO (parsearCasteo)\n";
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

std::unique_ptr<Expresion> Parser::parsearPrefijo() {
  Token t = get();

  switch (t.tipo) { //...
    case Tt::NUMERO: {
      auto [num, suf] = partirLexemaNum(t.lexema);
      return std::make_unique<ExprNumero>(num, suf);
    }

    case Tt::IDENTIFICADOR: {
      return std::make_unique<ExprVariable>(t.lexema);
    }

    case Tt::PAREN_L: {
      auto expr = parsearExpresion(Pr::MINIMA);
      check(Tt::PAREN_R);
      return expr;
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
    case Tt::MAS        : {op = TipoOperador::A_SUMA    ; break; }
    case Tt::MENOS      : {op = TipoOperador::A_RESTA   ; break; }
    case Tt::NO_LOGICO  : {op = TipoOperador::LOGICO_NO ; break; }
    case Tt::NO_BITWISE : {op = TipoOperador::BITWISE_NO; break; }
    case Tt::INCREMENTAR: {op = TipoOperador::INC_PREF  ; break; }
    case Tt::DECREMENTAR: {op = TipoOperador::DEC_PREF  ; break; }
    case Tt::ASTERISCO  : {op = TipoOperador::PTR_DEREF ; break; }
    case Tt::AMPERSAND  : {op = TipoOperador::PTR_REF   ; break; }

    default: {
        std::cerr << "Línea " << t.linea << ": No se esperaba el prefijo '" << t.lexema << "'\n";
        exit(1);
    }
  }

  auto operando = parsearExpresion(Pr::PREFIJO);
  return std::make_unique<ExprUnaria>(op, std::move(operando), true);

}


std::unique_ptr<Sentencia> Parser::parsearDeclaracionVar() {

  InfoVariable tipo = parsearTipo();

  Token nombre = check(Tt::IDENTIFICADOR);

  std::unique_ptr<Expresion> valor = nullptr;

  if (peek().tipo == Tt::IGUAL_ASIG) { // [TIPO] [ID] = [EXPR];
    get();
    valor = parsearExpresion(Pr::MINIMA);

  }
  // else: [TIPO] [ID];

  check(Tt::PUNTO_COMA);

  return std::make_unique<SentenciaVar>(nombre.lexema, tipo, std::move(valor));

}

std::unique_ptr<Sentencia> Parser::parsearSentenciaExpresion() {
  std::unique_ptr<Expresion> izquierda = parsearExpresion(Pr::MINIMA);

  if (peek().tipo == Tt::IGUAL_ASIG) { // ... = ...;
    get();

    std::unique_ptr<Expresion> derecha = parsearExpresion(Pr::MINIMA);
    check(Tt::PUNTO_COMA);

    return std::make_unique<SentenciaAsignacion>(std::move(izquierda), std::move(derecha));

  }

  // If you get a "Expected ';', got [something else]"
  // Please check this line and inc the counter
  // 15. Yikes.
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

std::unique_ptr<Sentencia> Parser::parsearBloque() {
  check(Tt::LLAVE_L);

  auto bloque = std::make_unique<Bloque>();

  while (peek().tipo != Tt::LLAVE_R && peek().tipo != Tt::FIN_ARCHIVO) {
    bloque->agregarSentencia(parsearSentencia());

  }

  check(Tt::LLAVE_R);
  return bloque;
}

// Parsear Sentencias
std::unique_ptr<Sentencia> Parser::parsearSentencia() {
  Tt actual = peek().tipo;

  if (actual == Tt::IF       ) { return parsearSi()       ; }
  if (actual == Tt::ELSE     ) { return parsearSino()     ; }
  if (actual == Tt::WHILE    ) { return parsearMientras() ; }
  if (actual == Tt::BREAK    ) { return parsearBreak()    ; }
  if (actual == Tt::CONTINUE ) { return parsearContinue() ; }
  if (actual == Tt::LLAVE_L  ) { return parsearBloque()   ; }
  if (actual == Tt::FUNC     ) { return parsearFuncDecl() ; }
  if (actual == Tt::PURE     ) { return parsearFuncDecl() ; }
  if (actual == Tt::RETURN   ) { return parsearReturn()   ; }
  if (actual == Tt::ESCRITURA) { return parsearEscritura(); }
  if (actual == Tt::ARCANO   ) { return parsearArcano()   ; }

  // Si empieza con un tipo de dato, es una delaración
  if (esTipo(actual) || esInfiere(actual)) {
    return parsearDeclaracionVar();

  }
 
  // Manejo de arcanos
  if (actual == Tt::IDENTIFICADOR) {
    if (arcanosActivos.count(peek().lexema)) {
      return parsearLlamadaArcano();

    }
  }
 
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
  int linea = peek().linea;
  check(Tt::BREAK);
  check(Tt::PUNTO_COMA);
  return std::make_unique<SentenciaBreak>(linea);
}

std::unique_ptr<Sentencia> Parser::parsearContinue() {
  int linea = peek().linea;
  check(Tt::CONTINUE);
  check(Tt::PUNTO_COMA);
  return std::make_unique<SentenciaContinue>(linea);
}

std::unique_ptr<Sentencia> Parser::parsearReturn() {
  check(Tt::RETURN);
  std::unique_ptr<Expresion> ret_value = parsearExpresion(Pr::MINIMA);
  check(Tt::PUNTO_COMA);

  return std::make_unique<SentenciaReturn>(ret_value->tipo_resuelto, std::move(ret_value));
}

std::map<std::string, InfoVariable> Parser::parsearFuncArgs() {

  std::map<std::string, InfoVariable> args;

  while (peek().tipo != Tt::PAREN_R && peek().tipo != Tt::FIN_ARCHIVO) {

    InfoVariable tipo  = parsearTipo();
    Token nombre = check(Tt::IDENTIFICADOR);


    if (args.count(nombre.lexema)) {
      //... Error, dos argumentos se llaman igual

    } else {
      args[nombre.lexema] = tipo;

    }

    if (peek().tipo == Tt::COMA) { get(); }

  }

  return args;

}

std::unique_ptr<Sentencia> Parser::parsearFuncDecl() {

  Token t = coincide({Tt::FUNC, Tt::PURE});
  Token i = check(Tt::IDENTIFICADOR);

  check(Tt::PAREN_L);

  std::map<std::string, InfoVariable> args = parsearFuncArgs();

  check(Tt::PAREN_R);

  check(Tt::FLECHA);

  InfoVariable ret_type = parsearTipo();

  bool firma;
  if        (peek().tipo == Tt::PUNTO_COMA) {
    firma = true;

  } else if (peek().tipo == Tt::LLAVE_L)    {
    firma = false;

  } else                                    {
    //... Error, peek().tipo was not expected

  }

  std::unique_ptr<Sentencia> cuerpo_func = nullptr;

  std::cout << "[670, parser.cpp] " << firma << '\n';

  if (firma) { // Firma
    get();

  } else { // Implementación
    cuerpo_func = parsearBloque();

  }

  return std::make_unique<SentenciaFuncDecl>(
    i.lexema,
    t.tipo == Tt::PURE,
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
        get(); // Get '='

      }

      args.push_back({nombre_arg, parsearExpresion(Pr::MINIMA)});

      if (peek().tipo == Tt::COMA) {
        get();

      } else {
        break; // No hay más comas, no hay más argumentos

      }
    } while (peek().tipo != Tt::PAREN_R && peek().tipo != Tt::FIN_ARCHIVO);

  }

  check(Tt::PAREN_R);

  return std::make_unique<ExprFuncCall>(std::move(callee), std::move(args));

}

std::pair<std::string, Regla> Parser::parsearReglaArcano() {
  std::pair<std::string, Regla> par;

  Token key = check(Tt::IDENTIFICADOR);
  check(Tt::CORCH_L);

  Token t;

  size_t cuenta_corchetes = 1;
  while (cuenta_corchetes > 0) {
    t = peek();
    if (t.tipo == Tt::CORCH_L) { cuenta_corchetes++; }
    if (t.tipo == Tt::CORCH_R) { cuenta_corchetes--; }
    if (cuenta_corchetes != 0) { par.second.componentes.push_back(get()); }

  }

  check(Tt::CORCH_R);

  t = peek();

  if        (t.tipo == Tt::ASTERISCO) { // 0 o más
    par.second.propiedades = 0;

  } else if (t.tipo == Tt::MAS)       { // 1 o más
    par.second.propiedades = 1;

  } else if (t.tipo == Tt::PREGUNTA)  { // 1 o 0
    par.second.propiedades = 2;

  } else                              { // Exactamente una vez
    par.second.propiedades = 3;

  }


  if ((par.second.propiedades & 0b11) != 3) {
    get();

  }

  return par;

}

std::map<std::string, Regla> Parser::parsearReglasArcano() {
  check(Tt::REGLAS);  // reglas
  check(Tt::LLAVE_L); // {

  std::map <std::string, Regla> reglas;
  std::pair<std::string, Regla> par;

  Token t = peek();

  while (t.tipo != Tt::LLAVE_R) { // Parsear cada regla por separado
    par = parsearReglaArcano();
    t = peek();

  }

  reglas[par.first] = par.second; // Añadimos la regla al diccionario

  check(Tt::LLAVE_R);    // }
  check(Tt::PUNTO_COMA); // ;

  return reglas;

}

std::map<std::string, std::unique_ptr<Sentencia>> Parser::parsearCuerpoArcano() {
  std::map<std::string, std::unique_ptr<Sentencia>> ramas;

  while (peek().tipo != Tt::LLAVE_R && peek().tipo != Tt::FIN_ARCHIVO) {
    Token t_keyword = check(Tt::IDENTIFICADOR); // algo
    check(Tt::ASIG_BLOQUE);                     // <=>
    std::unique_ptr<Sentencia> cuerpo = parsearBloque();      // { ... }
    check(Tt::PUNTO_COMA);                      // ;
    ramas[t_keyword.lexema] = std::move(cuerpo);
  }

  return ramas;
}

std::unique_ptr<Sentencia> Parser::parsearArcano() {
  check(Tt::ARCANO);
  Token nombre_arcano = check(Tt::IDENTIFICADOR);
  check(Tt::PAREN_L);

  DefinicionArcano def;

  while (peek().tipo != Tt::PAREN_R && peek().tipo != Tt::FIN_ARCHIVO) { // Argumentos
    Token nombre_param = check(Tt::IDENTIFICADOR);
    check(Tt::DOS_PUNTOS);
    Token t_tipo = get();  // code, expr, key

    TPA tipo = TPA::NULO;
    if      (t_tipo.lexema == "code") { tipo = TPA::CODE; }
    else if (t_tipo.lexema == "expr") { tipo = TPA::EXPR; }
    else if (t_tipo.lexema == "key" ) { tipo = TPA::KEY ; }

    def.esqueleto.push_back({nombre_param.lexema,
                                tipo,
                                });

    if (peek().tipo == Tt::COMA) { get(); }
  }
  check(Tt::PAREN_R);
  check(Tt::LLAVE_L);

  // Reglas
  std::map<std::string, Regla> relgas = parsearReglasArcano();

  // Cuerpo
  std::map<std::string, std::unique_ptr<Sentencia>> ramas = parsearCuerpoArcano();

  check(Tt::LLAVE_R);

  arcanosActivos[nombre_arcano.lexema] = def;

  return std::make_unique<SentenciaArcano>(
    nombre_arcano.lexema, std::move(def), std::move(ramas)
  );

}

std::unique_ptr<Sentencia> Parser::parsearLlamadaArcano() {
  Token nombre_arcano = get();
  DefinicionArcano& def = arcanosActivos[nombre_arcano.lexema];
  std::map<std::string, std::unique_ptr<Sentencia>> argumentos;

  // Recorremos el esqueleto para saber qué esperar
  for (const auto& parte : def.esqueleto) {
    if (parte.tipo_dato == TPA::CODE) {
        argumentos[parte.contenido] = parsearBloqSent();

    } else if (parte.tipo_dato == TPA::EXPR) {
      auto expr = parsearExpresion(Pr::MINIMA);
      argumentos[parte.contenido] = std::make_unique<SentenciaExpr>(std::move(expr));

    } else if (parte.tipo_dato == TPA::KEY) {
      Token t = peek();
      if (t.tipo == Tt::IDENTIFICADOR && t.lexema == parte.contenido) {
        get();

      }
    }
  }
  return std::make_unique<SentenciaLlamadaArcano>(nombre_arcano.lexema, std::move(argumentos));
}

// Precedencias de las operaciones
Pr Parser::obtenerPrecedencia(Tt tipo) {
  switch (tipo) {

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

    // --- Extra ---
    case Tt::CORCH_L   :
      return Pr::ACCESO;

    case Tt::PAREN_L    :
      return Pr::LLAMADA;

    default            :
      return Pr::MINIMA;
  }
}

// Algoritmo de Pratt
std::unique_ptr<Expresion> Parser::parsearExpresion(Pr precedenciaMinima) {

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

    if (op.tipo == Tt::CORCH_L) {
      izquierda = parsearAcceso(std::move(izquierda));
      continue;
    }

    // Un casteo
    if (op.tipo == Tt::PAREN_L && esTipo(peek().tipo)) {
      izquierda = parsearCasteo();
      continue;
    }

    // Function call
    if (op.tipo == Tt::PAREN_L) {
      izquierda = parsearFunctionCall(std::move(izquierda));
      continue;
    }


    // Casos Binarios
    Pr prec_propia = obtenerPrecedencia(op.tipo);
    Pr prec_derecha = (op.tipo == Tt::POTENCIA) ?  static_cast<Pr>(prec_propia - 1) : prec_propia;
 
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
  while (peek().tipo != Tt::FIN_ARCHIVO) {
    programa.push_back(parsearSentencia());
  }
  return programa;
}

