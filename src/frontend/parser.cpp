// parser.cpp

#include "Common.hpp"

#include "Parser.hpp"

/* --- Parser --- */
Token Parser::resolverAlias(Token t) {
  if (t.tipo == Tt::IDENTIFICADOR && alias_lexicos.count(t.lexema)) {
    t.tipo = alias_lexicos[t.lexema];
  }
  return t;
}

// Mira el token actual sin consumirlo
Token Parser::peek() {
  if (pos >= tokens.size()) { return tokens.back(); } // EOF
  return resolverAlias(tokens[pos]);
}

// Consume el token actual y avanza
Token Parser::get() {
  Token actual = peek();
  pos++;
  return actual;
}

// Chequea si el token actual es de ciertos tipos y avanza, sino, Error
bool Parser::coincide(std::initializer_list<Tt> tipos) {
  for (Tt tipo : tipos) {
    if (peek().tipo == tipo) {
      get();
      return true;
    }
  }
  return false;
}

Token Parser::check(Tt tipoEsperado) {
  if (peek().tipo == tipoEsperado) {
    return get();
  }
  std::cerr << "Error: Se esperaba " << nombreTipo(tipoEsperado)
            << " pero se encontró " << nombreTipo(peek().tipo)
            << " ('" << peek().lexema << "') en linea " << peek().linea << std::endl;
  exit(1);
}

Parser::Parser(std::vector<Token> t)
  : tokens(std::move(t)), pos(0) {}

InfoTipo Parser::parsearTipo() {
  InfoTipo info;
  Token t = peek();
  if (esInfiere(t.tipo) || esTipo(t.tipo)) {
    info.base_tipo = get().tipo;
  } else { //...
    std::cerr << "Error: Se esperaba un tipo de dato, pero se encontró '"
              << t.lexema << "'\n";
    exit(1);
  }

  if (esTipoComp(info.base_tipo)) {
    check(Tt::MENOR); // <
    info.subtipos.push_back(parsearTipo());
    while (peek().tipo == Tt::COMA) { // ,
      get();
      info.subtipos.push_back(parsearTipo()); // [TIPO]
    }
    check(Tt::MAYOR); // >
  }

  while (esModificador(peek().tipo)) {
    Tt mod = get().tipo;
    switch (mod) {
      case Tt::NAT: { info.es_nat = true; break; }
      case Tt::ETERNO: { info.es_eterno = true; break; }
      case Tt::EXO: { info.multiplicador *= 2; break; }
      case Tt::MAGNO: { info.multiplicador *= 4; break; }
      case Tt::ILUSTRE: { info.multiplicador *= 8; break; }
      case Tt::COMPLEJO: { info.es_complejo = true; break; }
      default: { break; }
    }
  }

  // Reglas de Tipos:
  //  Var no puede tener modificadores de tamaño o signo
  //  Lo mismo para real y wyn
  //  Vasto no puede tener modificadores de signo
  if (((info.base_tipo == Tt::VAR  ||
    info.base_tipo == Tt::REAL ||
    info.base_tipo == Tt::WYN) &&
    (info.multiplicador > 1 || info.es_nat)) ||
    ((info.base_tipo == Tt::VASTO) && (info.es_nat))) {
    std::cerr //...
    << "Error: No podés usar modificadores de tamaño o signo con inferencia de tipos\n";
    exit(1);
    }
    return info;
}

std::unique_ptr<Expresion> Parser::parsearRango() {
  std::unique_ptr<Expresion> inicio = nullptr;
  std::unique_ptr<Expresion> fin = nullptr;
  std::unique_ptr<Expresion> paso = nullptr;
  bool obtener_valores = false;
 
  // 1. Hay inicio? [A:]
  if (peek().tipo != Tt::DOS_PUNTOS) {
    inicio = parsearExpresion(MINIMA);
  }
  if (peek().tipo == Tt::DOS_PUNTOS) {
    get();
  }
  // 2. Hay fin? [a:B]
  if (peek().tipo != Tt::DOS_PUNTOS && peek().tipo != Tt::CORCH_R) {
    fin = parsearExpresion(MINIMA);
  }
  if (peek().tipo == Tt::DOS_PUNTOS) {
    get();
  }
  // 3. Hay paso? [a:b:C]
  if (peek().tipo != Tt::DOS_PUNTOS && peek().tipo != Tt::CORCH_R) {
    paso = parsearExpresion(MINIMA);
  }
  // 4. Pide los valores? [a:b:c .]
  if (peek().tipo == Tt::PUNTO) {
    get();
    obtener_valores = true;
  }

  check(Tt::CORCH_R);
  return std::make_unique<ExprRango>(
    std::move(inicio), std::move(fin),
    std::move(paso), obtener_valores
  );
}

std::unique_ptr<Expresion> Parser::parsearAcceso(std::unique_ptr<Expresion> contenedor) {
  auto indice_o_rango = parsearRango();
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
 
  alias_lexicos[alias.lexema] = original.tipo;
 
  return std::make_unique<SentenciaEscritura>(alias.lexema, original.tipo);
}

std::unique_ptr<Expresion> Parser::parsearPrefijo() {
  Token t = get();
  switch (t.tipo) {
    case Tt::NUMERO: return std::make_unique<ExprNumero>(t.lexema);
    case Tt::IDENTIFICADOR: return std::make_unique<ExprVariable>(t.lexema);
    case Tt::PAREN_L: {
      auto expr = parsearExpresion(MINIMA);
      check(Tt::PAREN_R);
      return expr;
    }
    // Casos de Prefijos
    case Tt::MENOS:
    case Tt::NO_LOGICO:
    case Tt::INCREMENTAR:
    case Tt::DECREMENTAR:
    case Tt::ASTERISCO:
    case Tt::AMPERSAND: {
      auto operando = parsearExpresion(PREFIJO);
     return std::make_unique<ExprUnaria>(t.lexema, std::move(operando), true);
    }
    case Tt::CORCH_L: {
      return parsearRango();
    }
    default: {
     std::cerr << "Error: No se esperaba el prefijo '" << t.lexema << "'\n";
     exit(1);
    }
  }
}

std::unique_ptr<Sentencia> Parser::parsearDeclaracionVar() {
  InfoTipo tipo = parsearTipo();
  Token nombre = check(Tt::IDENTIFICADOR);
  std::unique_ptr<Expresion> valor = nullptr;
  if (peek().tipo == Tt::IGUAL_ASIG) { // [TIPO] [ID] = [EXPR];
    get();
    valor = parsearExpresion(MINIMA);
  }
  // else: [TIPO] [ID];
  check(Tt::PUNTO_COMA);
  return std::make_unique<SentenciaVar>(nombre.lexema, "", std::move(valor));
}

std::unique_ptr<Sentencia> Parser::parsearSentenciaExpresion() {
  std::unique_ptr<Expresion> izquierda = parsearExpresion(MINIMA);
  if (peek().tipo == Tt::IGUAL_ASIG) {
    get();
    std::unique_ptr<Expresion> derecha = parsearExpresion(MINIMA);
    check(Tt::PUNTO_COMA);
    return std::make_unique<SentenciaAsignacion>(std::move(izquierda), std::move(derecha));
  }
  check(Tt::PUNTO_COMA);
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
  if (actual == Tt::SI)        { return parsearSi();        }
  if (actual == Tt::SINO)      { return parsearSino();      }
  if (actual == Tt::MIENTRAS)  { return parsearMientras();  }
  if (actual == Tt::LLAVE_L)   { return parsearBloque();    }
  if (actual == Tt::ESCRITURA) { return parsearEscritura(); }
  if (actual == Tt::ARCANO)    { return parsearArcano();    }
 
  // Si empieza con un tipo de dato, es una delaración
  if (esTipo(actual) || esInfiere(actual)) {
    return parsearDeclaracionVar();
  }
 
  // Manejo de arcanos
  if (actual == Tt::IDENTIFICADOR) {
    if (arcanos_activos.count(peek().lexema)) {
      return parsearLlamadaArcano();
    }
  }
 
  // Por defecto, es una expresión
  return parsearSentenciaExpresion();
}

std::unique_ptr<Sentencia> Parser::parsearSi() {
  check(Tt::SI);
  check(Tt::PAREN_L);
  auto condicion = parsearExpresion(MINIMA);
  check(Tt::PAREN_R);
  // La rama puede ser un bloque {...} o una sentencia
  std::unique_ptr<Sentencia> rama_si;
  rama_si = parsearBloqSent();
 
  return std::make_unique<SentenciaSi>(std::move(condicion), std::move(rama_si));
}

std::unique_ptr<Sentencia> Parser::parsearSino() {
  check(Tt::SINO);
  return std::make_unique<SentenciaSino>(parsearBloqSent());
}

std::unique_ptr<Sentencia> Parser::parsearMientras() {
  check(Tt::MIENTRAS);
  check(Tt::PAREN_L);
  auto condicion = parsearExpresion(MINIMA);
  check(Tt::PAREN_R);
  std::unique_ptr<Sentencia> rama_while;
  rama_while = parsearBloqSent();
  std::unique_ptr<Sentencia> rama_sino = nullptr;
  if (peek().tipo == Tt::SINO) {
    get();
    rama_sino = parsearBloqSent();
  }
  return std::make_unique<SentenciaMientras>(std::move(condicion), std::move(rama_while), std::move(rama_sino));
}

std::unique_ptr<Sentencia> Parser::parsearArcano() {
  check(Tt::ARCANO);
  Token nombre_arcano = check(Tt::IDENTIFICADOR);
  check(Tt::PAREN_L);

  DefinicionArcano def;

  while (peek().tipo != Tt::PAREN_R && peek().tipo != Tt::FIN_ARCHIVO) { // Argumentos
    if (coincide({Tt::REQ, Tt::OP})) { // Parámetro
      bool es_opcional = (tokens[pos - 1].tipo == Tt::OP);
      Token nombre_param = check(Tt::IDENTIFICADOR);
      check(Tt::DOS_PUNTOS);
      Token t_tipo = get();  // cod, expr, key

      TPA tipo = TPA::NULO;
      if (t_tipo.lexema == "cod") { tipo = TPA::COD; }
      else if (t_tipo.lexema == "expr") { tipo = TPA::EXPR; }
      else if (t_tipo.lexema == "key") { tipo = TPA::KEY; }

      def.esqueleto.push_back({TipoParte::PARAMETRO,
                                  nombre_param.lexema,
                                  tipo,
                                  es_opcional});

    } else { // Separador
      Token separador = check(Tt::IDENTIFICADOR);
      def.esqueleto.push_back({TipoParte::SEPARADOR,
                                  separador.lexema,
                                  TPA::NULO,
                                  false});
    }
    if (peek().tipo == Tt::COMA) { get(); }
  }
  check(Tt::PAREN_R);
  check(Tt::FLECHA);
  check(Tt::LLAVE_L);

  // Cuerpo
  std::map<std::string, std::unique_ptr<Sentencia>> ramas;

  while (peek().tipo != Tt::LLAVE_R && peek().tipo != Tt::FIN_ARCHIVO) {
    Token t_keyword = check(Tt::IDENTIFICADOR); // algo
    check(Tt::ASIG_BLOQUE);                     // <=>
    std::unique_ptr<Sentencia> cuerpo = parsearBloque();      // { ... }
    check(Tt::PUNTO_COMA);                      // ;
    ramas[t_keyword.lexema] = std::move(cuerpo);
  }

  check(Tt::LLAVE_R);

  arcanos_activos[nombre_arcano.lexema] = def;

  return std::make_unique<SentenciaArcano>(
    nombre_arcano.lexema, std::move(def), std::move(ramas)
  );
}

std::unique_ptr<Sentencia> Parser::parsearLlamadaArcano() {
  Token nombre_arcano = get();
  DefinicionArcano& def = arcanos_activos[nombre_arcano.lexema];
  std::map<std::string, std::unique_ptr<Sentencia>> argumentos;

  bool ultima_key = true;

  // Recorremos el esqueleto para saber qué esperar
  for (const auto& parte : def.esqueleto) {
    if (parte.tipo_parte == TipoParte::PARAMETRO) {
      if (parte.tipo_dato == TPA::COD) {
        if (!parte.es_opcional || ultima_key) {
          argumentos[parte.contenido] = parsearBloqSent();
        }
      } else if (parte.tipo_dato == TPA::EXPR) {
        auto expr = parsearExpresion(MINIMA);
        argumentos[parte.contenido] = std::make_unique<SentenciaExpr>(std::move(expr));
      } else if (parte.tipo_dato == TPA::KEY) {
        Token t = peek();
        if (t.tipo == Tt::IDENTIFICADOR && t.lexema == parte.contenido) {
          get();
          ultima_key = true;
        } else {
          ultima_key = false;
          if (!parte.es_opcional) {
            std::cerr << "Error: Se esperaba la keyword '" << parte.contenido //...
                      << "' para el arcano " << nombre_arcano.lexema << ".\n";
            exit(1);
          }
        }
      }
    }
  }
  return std::make_unique<SentenciaLlamadaArcano>(nombre_arcano.lexema, std::move(argumentos));
}

// Precedencias de las operaciones
int Parser::obtenerPrecedencia(Tt tipo) {
  switch (tipo) {

    // --- Lógicos ---
    case Tt::O_LOGICO:
      return LOGICA_O;
    case Tt::XOR_LOGICO:
      return LOGICA_XOR;
    case Tt::Y_LOGICO:
      return LOGICA_Y;

    // --- Bitwise ---
    case Tt::O_BITWISE:
      return BIT_O;
    case Tt::XOR_BITWISE:
      return BIT_XOR;
    case Tt::Y_BITWISE:
      return BIT_Y;

    // --- Igualdad --- 
    case Tt::IGUAL_CMP:
    case Tt::DISTINTO:
      return IGUALDAD;

    // --- Relacionales ---
    case Tt::MENOR:
    case Tt::MAYOR:
    case Tt::MAYOR_IGUAL:
    case Tt::MENOR_IGUAL:
      return RELACIONAL;

    // --- Shift ---
    case Tt::BITWISE_L:
    case Tt::BITWISE_R:
      return SHIFT;

    // --- Aritméticos ---
    case Tt::MAS:
    case Tt::MENOS:
      return SUMA;
 
    case Tt::ASTERISCO:
    case Tt::BARRA:
    case Tt::MODULO:
      return MULT;

    case Tt::POTENCIA:
      return POTENCIA;

    case Tt::INCREMENTAR:
    case Tt::DECREMENTAR:
    case Tt::FACTORIAL:
      return SUFIJO;

    case Tt::CORCH_L:
      return ACCESO;

    default: return MINIMA;
  }
}

// Algoritmo de Pratt
std::unique_ptr<Expresion> Parser::parsearExpresion(int precedenciaMinima) {
  // 1. Empezamos con un átomo (número o id)
  auto izquierda = parsearPrefijo();

  // 2. Comprobamos precedencias
  while (precedenciaMinima < obtenerPrecedencia(peek().tipo)) {
    Token op = get();

    // Casos de Sufijos
    if (op.tipo == Tt::INCREMENTAR || op.tipo == Tt::DECREMENTAR ||
        op.tipo == Tt::FACTORIAL) {
      izquierda = std::make_unique<ExprUnaria>(
          op.lexema, std::move(izquierda), false);
      continue;
    }

    if (op.tipo == Tt::CORCH_L) {
      izquierda = parsearAcceso(std::move(izquierda));
      continue;
    }

    // Casos Binarios
    int precPropia = obtenerPrecedencia(op.tipo);
    int precDerecha = (op.tipo == Tt::POTENCIA) ?  (precPropia - 1) : precPropia;
    auto derecha = parsearExpresion(precDerecha);
    // Creamos el nodo binario y lo hacemos la nueva izquierda
    izquierda = std::make_unique<ExprBinaria>(
      op.lexema, std::move(izquierda), std::move(derecha)
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
