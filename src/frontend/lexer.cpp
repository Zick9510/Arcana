// lexer.cpp

#include "Common.hpp"

#include "Lexer.hpp"

/* --- Lexer --- */
bool Lexer::esFin() const {
  return cursor >= source.size();

}

char Lexer::actual() const {
  return esFin() ? '\0' : source[cursor];

}

char Lexer::peek() const {
  if (cursor + 1 >= source.size()) { return '\0'; }
  return source[cursor + 1];

}

char Lexer::get() {
  char c = source[cursor++];
  if (c == '\n') { linea++; }
  return c;

}

bool Lexer::match(char esp) { // Esperado
  if (esFin() || source[cursor] != esp) { return false; }
  cursor++;
  return true;

}

bool Lexer::validarBase() {
  if (actual() == 'b' || actual() == 'o' || actual() == 'x') { return true; }
  //... Si es BOX, reportar error
  return false;

}

bool Lexer::validarCaracterBase(char caracter, char base) {
  switch (base) {

    case 'b': {
      return  (caracter == '0' || caracter == '1'  || caracter == '_');
    }

    case 'o': {
      return  (caracter >= '0' && caracter <= '7'  || caracter == '_');
    }

    case 'x': {
      return ((caracter >= '0' && caracter <= '7') ||
              (caracter >= 'A' && caracter <= 'F') || caracter == '_'
      );
    }

    default: {
      return false;
    }

  }
}

bool Lexer::esSufijoNum() {
  return (actual() == 's' || actual() == 'u' ||
          actual() == 'f' || actual() == 'i' );
}

void Lexer::consumirSufijoNum() {
  if (!esSufijoNum()) {
    return;
  }

  while (!esFin() && std::isdigit(peek())) { get(); }

}

void Lexer::leerNumero() {

  size_t inicio = cursor - 1; // El primer dígito lo consumió el get() en tokenize
  bool es_float = false;

  // 1. Base? (0b, 0o, 0x)
  if (source[inicio] == '0' && validarBase()) {
    char base = get();
    std::cout << base << '\n';

    while (validarCaracterBase(actual(), base)) {
      get();

    }

  } else {
    while (std::isdigit(actual()) || actual() == '.' || actual() == '_' || esSufijoNum()) {

      if (esSufijoNum()) {
        consumirSufijoNum();
        get();
        break;
      }

      if (actual() == '.') {
        if (es_float) { break; }
        es_float = true;
      }

      get();

    }

    // Scientific notation: e.g. 3e11 or 9e-3
    if (actual() == 'e' || actual() == 'E') {
      get(); // consume 'e'
      if (actual() == '+' || actual() == '-') {
          get(); // consume optional sign
      }
      while (std::isdigit(actual())) {
          get(); // consume exponent digits
      }
      es_float = true;
    }

  }
 
  std::string valor(source.substr(inicio, cursor - inicio));

  tokens.push_back( {Tt::NUMERO, std::string(valor), linea} );

}

void Lexer::leerStringChar() {
  bool comillas_dobles = (actual() == '"');

}

std::vector<Token> Lexer::tokenize() {

  tokens.clear();
  cursor = 0;

  while (!esFin()) {
    size_t inicio = cursor;
    char c = get();

    if (std::isspace(c)) { continue; }

    // --- Identificadores y Keywords ---
    if (std::isalpha(c) || c == '_') {
      while (std::isalnum(actual()) || actual() == '_') { get(); }

      std::string_view texto = source.substr(inicio, cursor - inicio);
      std::string s_texto(texto); // Para buscar en el map de keywords

      if (keywords.count(s_texto)) {
        tokens.push_back( {keywords[s_texto], s_texto, linea} );

      } else {
        tokens.push_back( {Tt::IDENTIFICADOR, s_texto, linea} );

      }
      continue;
    }

    // --- Números ---
    if (std::isdigit(c)) {
      leerNumero();
      continue;
    }

    // --- Strings y Chars ---
    if (c == '"' || c == '\'') {
      leerStringChar();
      continue;

    }

    // --- Operadores y Símbolos ---
    switch (c) {
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
        if        (match('/')) {
          char temp = peek();
          while ((temp = get()) != '\n' && !esFin()) {}

        } else if (match('-')) {

          char temp = peek();
          while (!esFin()) {

            if (temp == '-' && match('/')) {
                break;
            }
            temp = get();
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

      case '~': {
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

  tokens.push_back( {Tt::FIN_ARCHIVO, "", linea} );
 
  return tokens;

}
