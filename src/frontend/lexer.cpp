// lexer.cpp

#include "Lexer.hpp"

#include "Common.hpp"

/* --- Lexer --- */

// Checks if we have reached the end of the source string
bool Lexer::esFin() const {
  return cursor >= buffer->content.size();

}

// Returns the character at the current position without advancing
char Lexer::actual() const {
  return esFin() ? '\0' : buffer->content[cursor];

}

// Returns the next character (lookahead) without advancing the cursor
char Lexer::peek() const {
  if (cursor + 1 >= buffer->content.size()) { return '\0'; }
  return buffer->content[cursor + 1];

}

// Consumes and returns the current character, advancing the cursor
// Also increments the line counter if a newline is found
char Lexer::get() {
  char c = buffer->content[cursor++];
  if (c == '\n') {
    linea++;
    buffer->setOffset(cursor);
  }
  return c;

}

// "Conditional consume": If the current char matches 'esp', advance and return true
bool Lexer::match(char esp) { // Expected
  if (esFin() || buffer->content[cursor] != esp) { return false; }
  get();
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
      return  (caracter == '0' || caracter == '1' || caracter == '_');
    }

    case 'o': {
      return  (caracter >= '0' && caracter <= '7' || caracter == '_');
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
  if (buffer->content[inicio] == '0' && validarBase()) {
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
        std::cout << "[150 lexer.cpp] _.\n";
        //... Cant have a dot right after a _
      }

      if (actual() == '.') {
        if (peek() == '_') {
          std::cout << "[156 lexer.cpp ._\n";
          //... Cant have a _ right after a dot
        }

        if (tiene_punto) { break; } // Don't allow two dots
        tiene_punto = true;

      }

      get();

    }

  }
 
  std::string_view valor(buffer->content.data() + inicio, cursor - inicio);

  if (valor.back() == '_') {
    //... Ya cant end a number with a "_"
  }

  tokens.push_back( {Tt::NUMERO, std::string(valor), {cursor, cursor - inicio} } );

}

void Lexer::leerStringChar() { //...

  char delimitador = buffer->content[cursor - 1];
  std::string contenido = "";
  size_t inicio_linea = linea;
  size_t inicio_token = cursor - 1;

  while (!esFin() && actual() != delimitador) { //... Add multiline string support
    char c = get();

    if (c == '\\') {
      if (esFin()) { break; } //... Error: string sin cerrar tras \

      char escape = get();
      switch (escape) {
        case '0': { contenido += '\0'; break; }
        case 'n': { contenido += '\n'; break; }
        case 't': { contenido += '\t'; break; }
        case 'r': { contenido += '\r'; break; }

        case '\\': { contenido += '\\'; break; }
        case '\'': { contenido += '\''; break; }
        case '\"': { contenido += '\"'; break; }

        case 'x': {
          std::string hex_str = "";

          for (int i = 0; i < 2; ++i) { // Two hex chars tops
            if (!esFin() && std::isxdigit(actual())) {
              hex_str += get();

            } else {
              std::cerr << "Error: Secuencia de escape hex incompleta\n";

            }

          }

          if (hex_str.length() == 2) {
            contenido += static_cast<char>(std::stoi(hex_str, nullptr, 16));
          }

          break;

        }

        default: {
          //... Error: Secuencia de escape no conocida
          break;
        }
      }
    } else {
      contenido += c;
    }

    if (c == '\n') {
      linea++;
      std::cerr << "Error: Salto de línea inesperado en literal\n";
      break;
    }
  }

  if (esFin() && (cursor == 0 || buffer->content[cursor - 1] != delimitador)) {
    std::cerr << "Error: Literal no cerrado\n";

  } else {
    get();

  }

  if (delimitador == '\'') {
    if (contenido.size() > 1) {
      std::cerr << "Error: Literal de caracter con múltiples bytes\n";

    } else {
      std::cout << "[235, lexer.cpp] contenido: '" << contenido << "'\n";
      tokens.push_back( {Tt::CHAR, contenido, {cursor, 1} } );

    }

  } else {
    tokens.push_back( {Tt::STRING, contenido, {cursor, contenido.size()} } );

  }
}

void Lexer::captureSymbol() { //...

}

/* --- Main Tokenization Loop --- */

std::vector<Token> Lexer::tokenize() {

  tokens.clear();
  cursor = 0;

  while (!esFin()) {
    size_t inicio = cursor;
    char c = get();

    // Ignore whitespace
    if (std::isspace(c)) {
      continue;
    }

    // --- Identifiers and Keywords ---
    if (std::isalpha(c) || c == '_') { // Must start with alpha or _

      // Consumir el identificador completo
      while (std::isalnum(actual()) || actual() == '_') { get(); }
      std::string_view texto(buffer->content.data() + inicio, cursor - inicio);

      // Encontrar dónde empiezan los números (si existen)
      auto it_digito = std::find_if(texto.begin(), texto.end(), ::isdigit);
      std::string_view prefijo     = texto.substr(0, std::distance(texto.begin(), it_digito));
      std::string_view sufijo      = texto.substr(std::distance(texto.begin(), it_digito));
      auto es_keyword    = keywords.find(std::string(prefijo));

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

      tokens.push_back( {tipo_final, std::string(texto),  {inicio, texto.length()} } );
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
      case '(': { tokens.push_back( {Tt::PAREN_L, "(", {inicio, 1} } ); break; }
      case ')': { tokens.push_back( {Tt::PAREN_R, ")", {inicio, 1} } ); break; }
      case '{': { tokens.push_back( {Tt::LLAVE_L, "{", {inicio, 1} } ); break; }
      case '}': { tokens.push_back( {Tt::LLAVE_R, "}", {inicio, 1} } ); break; }
      case '[': { tokens.push_back( {Tt::CORCH_L, "[", {inicio, 1} } ); break; }
      case ']': { tokens.push_back( {Tt::CORCH_R, "]", {inicio, 1} } ); break; }

      case '.': { tokens.push_back( {Tt::PUNTO, ".", {inicio, 1} } ); break; }
      case ',': { tokens.push_back( {Tt::COMA , ",", {inicio, 1} } ); break; }

      case ';': { tokens.push_back( {Tt::PUNTO_COMA, ";", {inicio, 1} } ); break; }
      case ':': { tokens.push_back( {Tt::DOS_PUNTOS, ":", {inicio, 1} } ); break; }

      case '+': {
        if      (match('+')) { tokens.push_back( {Tt::INCREMENTAR, "++", {inicio, 2} } ); }
        else if (match('=')) { tokens.push_back( {Tt::MAS_IGUAL  , "+=", {inicio, 2} } ); }
        else                      { tokens.push_back( {Tt::MAS        , "+" , {inicio, 1} } ); }
        break;
      }

      case '-': {
        if      (match('>')) { tokens.push_back( {Tt::FLECHA     , "->", {inicio, 2} } ); }
        else if (match('-')) { tokens.push_back( {Tt::DECREMENTAR, "--", {inicio, 2} } ); }
        else if (match('=')) { tokens.push_back( {Tt::MENOS_IGUAL, "-=", {inicio, 2} } ); }
        else                      { tokens.push_back( {Tt::MENOS      , "-" , {inicio, 1} } ); }
        break;
      }

      case '*': {
        if        (match('*')) {
          if   (match('=')) { tokens.push_back( {Tt::POTENCIA_IGUAL, "**=", {inicio, 3} } ); }
          else                   { tokens.push_back( {Tt::POTENCIA      , "**" , {inicio, 3} } ); }

        } else if (match('/')) {
          if   (match('=')) { tokens.push_back( {Tt::RAIZ_IGUAL    , "*/=", {inicio, 3} } ); }
          else                   { tokens.push_back( {Tt::RAIZ          , "*/" , {inicio, 2} } ); }

        } else if (match('=')) {
          tokens.push_back( {Tt::POR_IGUAL, "*=", {inicio, 2} } );

        } else {
          tokens.push_back( {Tt::ASTERISCO, "*", {inicio, 1} } );

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
              get(); get();
              comment_closed = true;
              break;

            }

            get();

          }

          if (!comment_closed) {
            //... Call ErrorHandler
          }

        } else if (match('=')) {
          tokens.push_back( {Tt::DIV_IGUAL, "/=", {inicio, 2} } );

        } else {
          tokens.push_back( {Tt::DIV      , "/" , {inicio, 1} } );

        }

        break;
      }

      case '%': {
        if   (match('=')) { tokens.push_back( {Tt::MOD_IGUAL, "%=", {inicio, 2} } ); }
        else                   { tokens.push_back( {Tt::MODULO   , "%" , {inicio, 1} } ); }
        break;
      }

      case '&': {
        if        (match('&')) {
          if   (match('=')) { tokens.push_back( {Tt::Y_LOG_IGUAL, "&&=", {inicio, 3} } ); }
          else                   { tokens.push_back( {Tt::Y_LOGICO   , "&&" , {inicio, 2} } ); }

        } else if (match('=')) {
          tokens.push_back( {Tt::Y_BIT_IGUAL, "&=", {inicio, 2} } );

        } else                      {
          tokens.push_back( {Tt::AMPERSAND  , "&" , {inicio, 1} } );

        }

        break;
      }

      case '|': {
        if        (match('|')) {
          if   (match('=')) { tokens.push_back( {Tt::O_LOG_IGUAL, "||=", {inicio, 3} } ); }
          else                   { tokens.push_back( {Tt::O_LOGICO   , "||" , {inicio, 2} } ); }

        } else if (match('=')) {
          tokens.push_back( {Tt::O_BIT_IGUAL, "|=", {inicio, 2} } );

        } else                      {
          tokens.push_back( {Tt::O_BITWISE, "|", {inicio, 1} } );

        }

        break;
      }

      case '^': {
        if        (match('^')) {
          if   (match('=')) { tokens.push_back( {Tt::XO_LOG_IGUAL, "^^=", {inicio, 3} } ); }
          else                   { tokens.push_back( {Tt::XO_LOGICO   , "^^" , {inicio, 2} } ); }

        } else if (match('=')) {
          tokens.push_back( {Tt::XO_BIT_IGUAL, "^=", {inicio, 2} } );

        } else                      {
          tokens.push_back( {Tt::XO_BITWISE, "^", {inicio, 1} } );

        }

        break;
      }

      case '~': {
        if   (match('=')) { tokens.push_back( {Tt::FLOAT_EQUAL, "~=", {inicio, 2} } ); }
        else                   { tokens.push_back( {Tt::NO_BITWISE , "~" , {inicio, 1} } ); }

        break;
      }

      case '!': {
        if   (match('=')) { tokens.push_back( {Tt::DISTINTO , "!=", {inicio, 2} } ); }
        else                   { tokens.push_back( {Tt::NO_LOGICO, "!" , {inicio, 1} } ); }
        break;
      }

      case '<': {
        if        (match('<')) {
          if   (match('=')) { tokens.push_back( {Tt::BITWISE_L_IGUAL, "<<=", {inicio, 3} } ); }
          else                   { tokens.push_back( {Tt::BITWISE_L      , "<<" , {inicio, 2} } ); }

        } else if (match('=')) {
          if   (match('>')) { tokens.push_back( {Tt::ASIG_BLOQUE, "<=>", {inicio, 3} } ); }
          else                   { tokens.push_back( {Tt::MENOR_IGUAL, "<=" , {inicio, 2} } ); }

        } else if (match('>')) {
          tokens.push_back( {Tt::DISTINTO, "<>", {inicio, 2} } );

        } else {
          tokens.push_back( {Tt::MENOR, "<", {inicio, 1} });

        }

        break;
      }

      case '>': {
        if        (match('>')) {
          if   (match('=')) { tokens.push_back( {Tt::BITWISE_R_IGUAL, ">>=", {inicio, 3} } ); }
          else                   { tokens.push_back( {Tt::BITWISE_R      , ">>" , {inicio, 2} } ); }

        } else if (match('<')) {
          tokens.push_back( {Tt::SWAP       , "><", {inicio, 2} } );

        } else if (match('=')) {
          tokens.push_back( {Tt::MAYOR_IGUAL, ">=", {inicio, 2} } );

        } else                      {
          tokens.push_back( {Tt::MAYOR      , ">" , {inicio, 1} } );

        }

        break;
      }

      case '=': {
        if   (match('=')) { tokens.push_back( {Tt::IGUAL_CMP , "==", {inicio, 2} } ); }
        else                   { tokens.push_back( {Tt::IGUAL_ASIG, "=" , {inicio, 1} } ); }
        break;
      }

      case '?': {
        if   (match('?')) { tokens.push_back( {Tt::DOS_PREGUNTAS, "??", {inicio, 2} } ); }
        else                   { tokens.push_back( {Tt::PREGUNTA     , "?" , {inicio, 1} } ); }
        break;
      }

      case '@': {
        tokens.push_back( {Tt::ARROBA, "@", {inicio, 1} } );
        break;
      }

      case '#': {
        tokens.push_back( {Tt::TATETI, "#", {inicio, 1} } );
        break;
      }

    }
  }

  // EOF Token at the end
  tokens.push_back( {Tt::EOF_TT, "", {cursor, 1} } );
 
  return tokens;

}
