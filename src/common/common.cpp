// common.cpp

#include "Common.hpp"

#include "Includes.hpp"

/* --- Manejo de Tipos --- */

std::string nombreTipo(Tt tipo) {
  switch (tipo) {
    case Tt::LET  : { return "LET"  ; }
    case Tt::CONST: { return "CONST"; }

    case Tt::VOID_TYPE  : { return "VOID_TYPE"  ; }
    case Tt::BYTE_TYPE  : { return "BYTE_TYPE"  ; }
    case Tt::CHAR_TYPE  : { return "CHAR_TYPE"  ; }
    case Tt::BOOL_TYPE  : { return "BOOL_TYPE"  ; }
    case Tt::SHORT_TYPE : { return "SHORT_TYPE" ; }
    case Tt::INT_TYPE   : { return "INT_TYPE"   ; }
    case Tt::UINT_TYPE  : { return "UINT_TYPE"  ; }
    case Tt::FLOAT_TYPE : { return "FLOAT_TYPE" ; }
    case Tt::DOUBLE_TYPE: { return "DOUBLE_TYPE"; }
    case Tt::STRING_TYPE: { return "STRING_TYPE"; }
    case Tt::SLICE_TYPE : { return "SLICE_TYPE" ; }

    case Tt::ENUM  : { return "ENUM"  ; }
    case Tt::STRUCT: { return "STRUCT"; }

    case Tt::VAR: { return "VAR"; }
    case Tt::IDENTIFICADOR: { return "ID"; }
    case Tt::NUMERO: { return "NUMERO"; }
    case Tt::CHAR: { return "CHAR"; }

    case Tt::ASIG_BLOQUE: { return "ASIG_BLOQUE"; }
    case Tt::PUNTO_COMA: { return "PUNTO_COMA"; }
    case Tt::COMA: { return "COMA"; }
    case Tt::PUNTO: { return "PUNTO"; }
    case Tt::DOS_PUNTOS: { return "DOS_PUNTOS"; }
    case Tt::ASTERISCO: { return "ASTERISCO"; }
    case Tt::AMPERSAND: { return "AMPERSAND"; }

    case Tt::IGUAL_ASIG: { return "IGUAL_ASIG"; }

    case Tt::LLAVE_L: { return "LLAVE_L"; }
    case Tt::LLAVE_R: { return "LLAVE_R"; }
    case Tt::PAREN_L: { return "PAREN_L"; }
    case Tt::PAREN_R: { return "PAREN_R"; }
    case Tt::CORCH_L: { return "CORCH_L"; }
    case Tt::CORCH_R: { return "CORCH_R"; }

    case Tt::ARCANE: { return "ARCANE"; }
    case Tt::RULES : { return "REGLAS"; }
    case Tt::KEY   : { return "KEY"   ; }
    case Tt::EXPR  : { return "EXPR"  ; }
    case Tt::CODE  : { return "CODE"  ; }

    case Tt::TEMPLATE: { return "TEMPLATE"; }
    case Tt::TYPE    : { return "TYPE"    ; }

    case Tt::EOF_TT: { return "EOF_TT"; }
    case Tt::ERROR : { return "ERROR" ; }

    default: { return "DESCONOCIDO"; }

  }

}

bool esModificador(Tt tipo) {
  return tipo == Tt::UNSIGNED  || tipo == Tt::LONG      ||
         tipo == Tt::VERY_LONG || tipo == Tt::FULL_LONG ||
         tipo == Tt::COMPLEJO  || tipo == Tt::CONST;
}

bool esInfiere(Tt tipo) { //... I think we should not do this
  return tipo == Tt::VAR || tipo == Tt::CONST;
}

bool esTipoComp(Tt tipo) {
  return tipo == Tt::VECTOR_TYPE || tipo == Tt::MAP_TYPE || tipo == Tt::SET_TYPE;
}

bool esTipo(Tt tipo) {
  return tipo == Tt::BYTE_TYPE   || tipo == Tt::CHAR_TYPE   || tipo == Tt::SHORT_TYPE  ||
         tipo == Tt::INT_TYPE    || tipo == Tt::UINT_TYPE   || tipo == Tt::STRING_TYPE ||
         tipo == Tt::FLOAT_TYPE  || tipo == Tt::DOUBLE_TYPE || tipo == Tt::BOOL_TYPE   ||
         tipo == Tt::SLICE_TYPE  || esTipoComp(tipo);
}

/* --- Colores para la terminal --- */

inline const std::string COLOR_RESET   = "\033[0m"   ;
inline const std::string COLOR_RED     = "\033[1;31m";
inline const std::string COLOR_GREEN   = "\033[1;32m";
inline const std::string COLOR_YELLOW  = "\033[1;33m";
inline const std::string COLOR_BLUE    = "\033[1;34m";
inline const std::string COLOR_MAGENTA = "\033[1;35m";
inline const std::string COLOR_CYAN    = "\033[1;36m";

