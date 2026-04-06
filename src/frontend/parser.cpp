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
Token Parser::peek() {
  if (pos >= tokens.size()) { return tokens.back(); } // EOF
  return resolverAlias(tokens[pos]);
}

// Consume the next token and return it
Token Parser::get() {
  Token actual = peek();
  pos++;
  return actual;
}

// Same use as Token check(Tt) function from this file, but it expects anya of the options
bool Parser::coincide(std::initializer_list<Tt> tipos) {
  for (Tt tipo : tipos) {
    if (peek().tipo == tipo) {
      get();
      return true;
    }
  }
  return false;
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
  exit(1);
}

Parser::Parser(std::vector<Token> t, TypeFactory& tf)
  : tokens(std::move(t)), pos(0), typeFactory(tf) {}

int extraerBits(std::string lexema, int defaultBits) {
  if (lexema == "int" || lexema == "float") { return defaultBits; }

  std::string num;

  for (char c : lexema) {
    if (std::isdigit(c)) { num += c; }
  }

  return num.empty() ? defaultBits : std::stoi(num);

}

InfoVariable Parser::parsearTipo() {
  InfoVariable info;
  bool procesando_tipos = true ;
  bool es_unsigned      = false;

  TypeKind tipo_base = TypeKind::VOID;
  int      bits      = -1;

  while (procesando_tipos) {
    Token t = peek();

    switch (t.tipo) { //...
      case Tt::CONST: {
        if (info.es_const) {
          //... Error: Declaró 2 veces const en una sola var
        } else {
          info.es_const = true;
        }

        get();
        break;
      }

      case Tt::UNSIGNED: {
        if (es_unsigned) {
          //... Error: Declaró 2 veces unsigned
        } else {
          es_unsigned = true;
        }

        get();
        break;
      }

      case Tt::INT: {
        tipo_base = TypeKind::INTEGER;
        bits = extraerBits(t.lexema, 32);

        get();
        break;
      }

      case Tt::FLOAT: {
        tipo_base = TypeKind::FLOAT;
        bits = extraerBits(t.lexema, 64);

        get();
        break;
      }

      default: { // Whatever we found here it aint a type
        procesando_tipos = false; // We aint gonna process it then
        break;
      }
    }
  }

  if        (tipo_base == TypeKind::INTEGER) {
    info.tipo = Dt(typeFactory.getInteger(bits, es_unsigned));

  } else if (tipo_base == TypeKind::FLOAT)   {
    info.tipo = Dt(typeFactory.getFloat(bits));

  }

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

  bool obtener_valores = false;
 
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

  // 4. What use could we assign to the "." at the end of a range? Maybe "Access the value elements from the map"?
  if (peek().tipo == Tt::PUNTO) {
    get();
    obtener_valores = true;

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

  TipoOperador op;

  switch (t.tipo) {
    case Tt::NUMERO: {
      std::pair<std::string, std::string> par = partirLexemaNum(t.lexema);

      return std::make_unique<ExprNumero>(par.first, par.second);

    }

    case Tt::IDENTIFICADOR: { return std::make_unique<ExprVariable>(t.lexema); }

    case Tt::PAREN_L: {
      std::unique_ptr<Expresion> expr = parsearExpresion(Pr::MINIMA);
      check(Tt::PAREN_R);
      return expr;

    }

    // Casos de Prefijos
    case Tt::MAS        : { op = TipoOperador::A_SUMA    ; }
    case Tt::MENOS      : { op = TipoOperador::A_RESTA   ; }
    case Tt::NO_LOGICO  : { op = TipoOperador::LOGICO_NO ; }
    case Tt::NO_BITWISE : { op = TipoOperador::BITWISE_NO; }
    case Tt::INCREMENTAR: { op = TipoOperador::INC_PREF  ; }
    case Tt::DECREMENTAR: { op = TipoOperador::DEC_PREF  ; }
    case Tt::ASTERISCO  : { op = TipoOperador::PTR_DEREF ; }
    case Tt::AMPERSAND  : { op = TipoOperador::PTR_REF   ;
      std::unique_ptr<Expresion> operando = parsearExpresion(Pr::PREFIJO);
      return std::make_unique<ExprUnaria>(op, std::move(operando), true);

    }

    case Tt::POTENCIA: {
      op = TipoOperador::A_MULT;
      std::unique_ptr<Expresion> inner = parsearExpresion(Pr::PREFIJO);
      std::unique_ptr<Expresion> segunda_deref = std::make_unique<ExprUnaria>(
        op,
        std::move(inner),
        true
      );
      return std::make_unique<ExprUnaria>(op, std::move(segunda_deref), true);

    }

    case Tt::Y_LOGICO: {
      op = TipoOperador::BITWISE_Y;
      std::unique_ptr<Expresion> inner = parsearExpresion(Pr::PREFIJO);
      std::unique_ptr<Expresion> segunda_ref =std::make_unique<ExprUnaria>(
        op,
        std::move(inner),
        true
      );
      return std::make_unique<ExprUnaria>(op, std::move(segunda_ref), true);

    }

    case Tt::CORCH_L: {
      return parsearRangoOArray();

    }

    default: { //...
      std::cerr << "Línea: " << t.linea << '\n';
      std::cerr << "Error: No se esperaba el prefijo '" << t.lexema << "'\n";
      exit(1);

    }
  }
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

  if (actual == Tt::SI       ) { return parsearSi()       ; }
  if (actual == Tt::SINO     ) { return parsearSino()     ; }
  if (actual == Tt::MIENTRAS ) { return parsearMientras() ; }
  if (actual == Tt::LLAVE_L  ) { return parsearBloque()   ; }
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
  check(Tt::SI);
  check(Tt::PAREN_L);

  auto condicion = parsearExpresion(Pr::MINIMA);

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

  auto condicion = parsearExpresion(Pr::MINIMA);

  check(Tt::PAREN_R);

  std::unique_ptr<Sentencia> rama_while;
  rama_while = parsearBloqSent();

  std::unique_ptr<Sentencia> rama_sino = nullptr;

  if (peek().tipo == Tt::SINO) {
    get();
    rama_sino = parsearBloqSent();

  }

  return std::make_unique<SentenciaMientras>(
       std::move(condicion),
    std::move(rama_while),
     std::move(rama_sino)
  );

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
      TipoOperador operador = (op.tipo == Tt::INCREMENTAR) ? TipoOperador::INC_SUF: TipoOperador::INC_SUF ;
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

// --- Error Recovery Functions ---
