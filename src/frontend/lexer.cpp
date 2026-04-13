// lexer.cpp

#include "Common.hpp"

#include "Lexer.hpp"

/* --- Lexer --- */

// Checks if we have reached the end of the source string
bool Lexer::esFin() const {
  return cursor >= source.size();

}

// Returns the character at the current position without advancing
char Lexer::actual() const {
  return esFin() ? '\0' : source[cursor];

}

// Returns the next character (lookahead) without advaincing the cursor
char Lexer::peek() const {
  if (cursor + 1 >= source.size()) { return '\0'; }
  return source[cursor + 1];

}

// Consumes and returns the current character, advaincing the cursor
// Also increments the line counter if a newline is found
char Lexer::get() {
  char c = source[cursor++];
  if (c == '\n') { linea++; }
  return c;

}

// "Conditional consume": If the current char matches 'esp', advance and return true
bool Lexer::match(char esp) { // Expected
  if (esFin() || source[cursor] != esp) { return false; }
  cursor++;
  return true;

}

/* --- Numerical Parsing Logic --- */

// Checks if the current char indicates a numeric base (b, o, x)
bool Lexer::validarBase() {
  if (actual() == 'b' || actual() == 'o' || actual() == 'x') { return true; }
  // If it detects B, O, X we should raise an error: They are not allowed
  return false;

}

// Validates if 'caracter' is a valid digit for the given base (binary, octal, hex)
bool Lexer::validarCaracterBase(char caracter, char base) {
  switch (base) {

    case 'b': {
      return  (caracter == '0' || caracter == '1'  || caracter == '_');
    }

    case 'o': {
      return  (caracter >= '0' && caracter <= '7'  || caracter == '_');
    }

    case 'x': {
      return ((caracter >= '0' && caracter <= '9') ||
              (caracter >= 'A' && caracter <= 'F') || caracter == '_'
      );
    }

    default: {
      return false;
    }

  }
}

// Checks for numeric suffixes (like 'u' for unsigned, 'f' for float, etc.)
bool Lexer::esSufijoNum() {
  return (actual() == 'f' || actual() == 'u' || actual() == 'i');
}

// Consumes suffixes and any trailing digits (e.g., f32, i64)
void Lexer::consumirSufijoNum() {
  if (!esSufijoNum()) {
    return;
  }

  while (!esFin() && std::isdigit(peek())) { get(); }

}

void Lexer::leerNumero() {

  size_t inicio = cursor - 1; // get() was already called in tokenize()
  bool tiene_punto = false;
  bool scientific_notation = false;

  // Handle non-decimal bases: 0b..., 0o..., 0x...
  if (source[inicio] == '0' && validarBase()) {
    char base = get(); // Consume the 'b', 'o', or 'x'

    while (validarCaracterBase(actual(), base)) {
      get();

    }

  } else { // Handle standar decimals and floating point
    while (std::isdigit(actual())             ||
           actual() == '.' || actual() == '_' ||
           esSufijoNum()   || actual() == 'e') {

      if (actual() == 'e') {
        scientific_notation = true;

        if (peek() == '+' || peek() == '-')  {
          get();

        } else if (!std::isdigit(peek())) {
          //... Se esperaba un dígito o un signo después de 'e'

        }

        while (std::isdigit(peek())) { get(); }

      }

      if (esSufijoNum()) {

        if (actual() == 'f' && !tiene_punto) {
          //... Literal mal formado, el sufijo 'f' siempre requiere un literal con un punto

        } else if (tiene_punto) {
          //... Literal mal formado, un sufijo distinto de 'f' no puede tener punto decimal
        }

        consumirSufijoNum();
        get();
        break;

      } else if (std::islower(actual())) {
        //... Sufijo inválido
        break;

      }

      if (actual() == '_' && peek() == '.') {
        std::cout << "[151 lexer.cpp] _.\n";
        //... Cant have a dot right after a _
      }

      if (actual() == '.') {
        if (peek() == '_') {
          std::cout << "[157 lexer.cpp ._\n";
          //... Cant have a _ right after a dot
        }

        if (tiene_punto) { break; } // Don't allow two dots
        tiene_punto = true;

      }

      get();

    }

  }
 
  std::string valor(source.substr(inicio, cursor - inicio));

  if (valor.back() == '_') {
    //... Ya cant end a number with a "_"
  }

  tokens.push_back( {Tt::NUMERO, std::string(valor), linea} );

}

void Lexer::leerStringChar() { //...
  bool comillas_dobles = (actual() == '"');
  // Logic to consume until closing quote would go here

}

/* --- Main Tokenization Loop --- */

std::vector<Token> Lexer::tokenize() {

  tokens.clear();
  cursor = 0;

  while (!esFin()) {
    size_t inicio = cursor;
    char c = get();

    // Ignore whitespace
    if (std::isspace(c)) { continue; }

    // --- Identifiers and Keywords ---
    if (std::isalpha(c) || c == '_') { // Must start with alpha or _

      // Consumir el identificador completo
      while (std::isalnum(actual()) || actual() == '_') { get(); }
      std::string_view texto = source.substr(inicio, cursor - inicio);

      // Encontrar dónde empiezan los números (si existen)
      auto it_digito = std::find_if(texto.begin(), texto.end(), ::isdigit);
      std::string_view prefijo = texto.substr(0, std::distance(texto.begin(), it_digito));
      std::string_view sufijo  = texto.substr(std::distance(texto.begin(), it_digito));
      auto es_keyword = keywords.find(std::string(prefijo));

      // Se asume identificador a menos que cumpla todos los requisitos de ser una keyword
      Tt tipo_final = Tt::IDENTIFICADOR;

      if (auto it = keywords.find(std::string(prefijo));
               it != keywords.end()) { // Si está en las keywords
        if (sufijo.empty()) { // Caso: "int", "while", "arcano"
          tipo_final = it->second;

        } else if (esTipo(it->second)) { // Caso: "float64", "int123"
          int valor = 0;
          auto [ptr, ec] =
            std::from_chars(sufijo.data(), sufijo.data() + sufijo.size(), valor);
          bool es_valido   = (ec == std::errc());
          bool es_potencia = (valor >= 8) && (isPowerOf2(valor));

          if        (es_valido && es_potencia) { // Caso: "int128", byte16
            tipo_final = it->second;

          } else if (es_valido)                { // Caso: "float3", "char9"
            //... Lanzar warning: El nombre de la variable puede ser confuso

          }
        } else { // Caso: "if42", "for999"
          //... Lanzar warning: El nombre de la variable puede ser confuso

        }
      }

      tokens.push_back( {tipo_final, std::string(texto), linea} );
      continue;
    }

    // --- Numbers ---
    if (std::isdigit(c)) {
      leerNumero();
      continue;
    }

    // --- Strings and Characters ---
    if (c == '"' || c == '\'') {
      leerStringChar();
      continue;

    }

    // --- Symbols ---
    switch (c) { // Greedy matching
      case '(': { tokens.push_back( {Tt::PAREN_L, "(", linea} ); break; }
      case ')': { tokens.push_back( {Tt::PAREN_R, ")", linea} ); break; }
      case '{': { tokens.push_back( {Tt::LLAVE_L, "{", linea} ); break; }
      case '}': { tokens.push_back( {Tt::LLAVE_R, "}", linea} ); break; }
      case '[': { tokens.push_back( {Tt::CORCH_L, "[", linea} ); break; }
      case ']': { tokens.push_back( {Tt::CORCH_R, "]", linea} ); break; }

      case '.': { tokens.push_back( {Tt::PUNTO, ".", linea} ); break; }
      case ',': { tokens.push_back( {Tt::COMA , ",", linea} ); break; }

      case ';': { tokens.push_back( {Tt::PUNTO_COMA, ";", linea} ); break; }
      case ':': { tokens.push_back( {Tt::DOS_PUNTOS, ":", linea} ); break; }

      case '+': {
        if      (match('+')) { tokens.push_back( {Tt::INCREMENTAR, "++", linea} ); }
        else if (match('=')) { tokens.push_back( {Tt::MAS_IGUAL  , "+=", linea} ); }
        else                      { tokens.push_back( {Tt::MAS        , "+" , linea} ); }
        break;
      }

      case '-': {
        if      (match('>')) { tokens.push_back( {Tt::FLECHA     , "->", linea} ); }
        else if (match('-')) { tokens.push_back( {Tt::DECREMENTAR, "--", linea} ); }
        else if (match('=')) { tokens.push_back( {Tt::MENOS_IGUAL, "-=", linea} ); }
        else                      { tokens.push_back( {Tt::MENOS      , "-" , linea} ); }
        break;
      }

      case '*': {
        if        (match('*')) {
          if   (match('=')) { tokens.push_back( {Tt::POTENCIA_IGUAL, "**=", linea} ); }
          else                   { tokens.push_back( {Tt::POTENCIA      , "**" , linea} ); }

        } else if (match('/')) {
          if   (match('=')) { tokens.push_back( {Tt::RAIZ_IGUAL    , "*/=", linea} ); }
          else                   { tokens.push_back( {Tt::RAIZ          , "*/" , linea} ); }

        } else if (match('=')) {
          tokens.push_back( {Tt::POR_IGUAL, "*=", linea} );

        } else {
          tokens.push_back( {Tt::ASTERISCO, "*" , linea} );

        }

        break;
      }

      case '/': {
        if        (match('/')) { // Single-line comment: //
          char temp = peek();
          while ((temp = get()) != '\n' && !esFin()) {}

        } else if (match('-')) { // Multi-line comment: /- ... -/

          bool comment_closed = false;

          while (!esFin()) {

            if (actual() == '-' && peek() == '/') {
              get(); get(); // Consume - and /
              comment_closed = true;
              break;

            }

            get();

          }

          if (!comment_closed) {
            //... Call ErrorHandler
          }

        } else if (match('=')) {
          tokens.push_back( {Tt::DIV_IGUAL, "/=", linea} );

        } else {
          tokens.push_back( {Tt::DIV      , "/" , linea} );

        }

        break;
      }

      case '%': {
        if   (match('=')) { tokens.push_back( {Tt::MOD_IGUAL, "%=", linea} ); }
        else                   { tokens.push_back( {Tt::MODULO   , "%" , linea} ); }
        break;
      }

      case '&': {
        if        (match('&')) {
          if   (match('=')) { tokens.push_back( {Tt::Y_LOG_IGUAL, "&&=", linea} ); }
          else                   { tokens.push_back( {Tt::Y_LOGICO   , "&&" , linea} ); }

        } else if (match('=')) {
          tokens.push_back( {Tt::Y_BIT_IGUAL, "&=", linea} );

        } else                      {
          tokens.push_back( {Tt::AMPERSAND  , "&" , linea} );

        }

        break;
      }

      case '|': {
        if        (match('|')) {
          if   (match('=')) { tokens.push_back( {Tt::O_LOG_IGUAL, "||=", linea} ); }
          else                   { tokens.push_back( {Tt::O_LOGICO   , "||" , linea} ); }

        } else if (match('=')) {
          tokens.push_back( {Tt::O_BIT_IGUAL, "|=", linea} );

        } else                      {
          tokens.push_back( {Tt::O_BITWISE, "|", linea} );

        }

        break;
      }

      case '^': {
        if        (match('^')) {
          if   (match('=')) { tokens.push_back( {Tt::XO_LOG_IGUAL, "^^=", linea} ); }
          else                   { tokens.push_back( {Tt::XO_LOGICO   , "^^" , linea} ); }

        } else if (match('=')) {
          tokens.push_back( {Tt::XO_BIT_IGUAL, "^=", linea} );

        } else                      {
          tokens.push_back( {Tt::XO_BITWISE, "^", linea} );

        }

        break;
      }

      case '~': { //... x ~= y has no sense. We could use the "~=" symbol for flaots comparations
        if   (match('=')) { tokens.push_back( {Tt::NO_BIT_IGUAL, "~=", linea} ); }
        else                   { tokens.push_back( {Tt::NO_BITWISE  , "~" , linea} ); }

        break;
      }

      case '!': {
        if   (match('=')) { tokens.push_back( {Tt::DISTINTO , "!=", linea} ); }
        else                   { tokens.push_back( {Tt::NO_LOGICO, "!" , linea} ); }
        break;
      }

      case '<': {
        if        (match('<')) {
          if   (match('=')) { tokens.push_back( {Tt::BITWISE_L_IGUAL, "<<=", linea} ); }
          else                   { tokens.push_back( {Tt::BITWISE_L      , "<<" , linea} ); }

        } else if (match('=')) {
          if   (match('>')) { tokens.push_back( {Tt::ASIG_BLOQUE, "<=>", linea} ); }
          else                   { tokens.push_back( {Tt::MENOR_IGUAL, "<=" , linea} ); }

        } else if (match('>')) {
          tokens.push_back( {Tt::DISTINTO, "<>", linea} );

        } else {
          tokens.push_back( {Tt::MENOR, "<", linea});

        }

        break;
      }

      case '>': {
        if        (match('>')) {
          if   (match('=')) { tokens.push_back( {Tt::BITWISE_R_IGUAL, ">>=", linea} ); }
          else                   { tokens.push_back( {Tt::BITWISE_R      , ">>" , linea} ); }

        } else if (match('<')) {
          tokens.push_back( {Tt::SWAP       , "><", linea} );

        } else if (match('=')) {
          tokens.push_back( {Tt::MAYOR_IGUAL, ">=", linea} );

        } else                      {
          tokens.push_back( {Tt::MAYOR      , ">" , linea} );

        }

        break;
      }

      case '=': {
        if   (match('=')) { tokens.push_back( {Tt::IGUAL_CMP , "==", linea} ); }
        else                   { tokens.push_back( {Tt::IGUAL_ASIG, "=" , linea} ); }
        break;
      }

      case '?': {
        if   (match('?')) { tokens.push_back( {Tt::DOS_PREGUNTAS, "??", linea} ); }
        else                   { tokens.push_back( {Tt::PREGUNTA     , "?" , linea} ); }
        break;
      }
    }
  }

  // EOF Token at the end
  tokens.push_back( {Tt::FIN_ARCHIVO, "", linea} );
 
  return tokens;

}
