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

std::vector<Token> Lexer::tokenize() {
  std::vector<Token> tokens;

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
      while (std::isdigit(actual()) || actual() == '.') { get(); }
      tokens.push_back(
          {Tt::NUMERO,
              std::string(source.substr(inicio, cursor - inicio)),
              linea}
      );
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
          while (peek() != '\n' && !esFin()) { get(); }

        } else if (match('-')) {
          while (!esFin()) {
            if (actual() == '-' && match('/')) {
                break;
            }
            get();
          }

        } else if (match('=')) {
          tokens.push_back( {Tt::DIV_IGUAL, "/=", linea} );
        }
        break;
      }

      case '&': {
        if        (match('&')) {
          if   (match('=')) { tokens.push_back( {Tt::Y_LOG_IGUAL, "&&=", linea} ); }
          else                   { tokens.push_back( {Tt::Y_LOGICO   , "&&" , linea} ); }

        } else if (match('=')) {
          tokens.push_back( {Tt::Y_BIT_IGUAL, "&=", linea} );

        } else                 {
          tokens.push_back( {Tt::AMPERSAND  , "&" , linea} );

        }

        break;
      }

      case '!': {
        if   (match('=')) { tokens.push_back( {Tt::DISTINTO , "!=", linea} ); }
        else                   { tokens.push_back( {Tt::FACTORIAL, "!" , linea} ); }
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
        if (match('>')) {
          if   (match('=')) { tokens.push_back( {Tt::BITWISE_R_IGUAL, ">>=", linea} ); }
          else                   { tokens.push_back( {Tt::BITWISE_R      , ">>" , linea} ); }

        } else if (match('<')) {
          tokens.push_back( {Tt::SWAP, "><", linea} );

        } else if (match('=')) {
          tokens.push_back( {Tt::MAYOR_IGUAL, ">=", linea});

        } else {
          tokens.push_back( {Tt::MAYOR, ">", linea});

        }

        break;
      }

      case '=': {
        if   (match('=')) { tokens.push_back( {Tt::IGUAL_CMP , "==", linea}); }
        else                   { tokens.push_back( {Tt::IGUAL_ASIG, "=" , linea}); }
        break;
      }
    }
  }
  tokens.push_back( {Tt::FIN_ARCHIVO, "", linea} );
  return tokens;
}
