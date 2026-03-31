// common.cpp

#include "Common.hpp"

/* --- Manejo de Tipos --- */

std::string nombreTipo(Tt tipo) {
  switch (tipo) {
    case Tt::VAR: return "VAR";
    case Tt::IDENTIFICADOR: return "IDENTIFICADOR";
    case Tt::NUMERO: return "NUMERO";
    case Tt::REGLAS: return "REGLAS";

    case Tt::ASIG_BLOQUE: return "ASIG_BLOQUE";
    case Tt::PUNTO_COMA: return "PUNTO_COMA";
    case Tt::COMA: return "COMA";
    case Tt::PUNTO: return "PUNTO";
    case Tt::DOS_PUNTOS: return "DOS_PUNTOS";
    case Tt::ASTERISCO: return "ASTERISCO";
    case Tt::AMPERSAND: return "AMPERSAND";

    case Tt::IGUAL_ASIG: return "IGUAL";
    case Tt::FIN_ARCHIVO: return "EOF";

    case Tt::LLAVE_L: return "LLAVE_L";
    case Tt::LLAVE_R: return "LLAVE_R";
    case Tt::PAREN_L: return "PAREN_L";
    case Tt::PAREN_R: return "PAREN_R";
    case Tt::CORCH_L: return "CORCH_L";
    case Tt::CORCH_R: return "CORCH_R";

    case Tt::ARCANO: return "ARCANO";

    default: return "DESCONOCIDO";
  }
}

bool esModificador(Tt tipo) {
  return tipo == Tt::UNSIGNED  || tipo == Tt::LONG      ||
         tipo == Tt::VERY_LONG || tipo == Tt::FULL_LONG ||
         tipo == Tt::COMPLEJO  || tipo == Tt::CONST;
}

bool esInfiere(Tt tipo) {
  return tipo == Tt::VAR || tipo == Tt::CONST;
}

bool esTipoComp(Tt tipo) {
  return tipo == Tt::VECTOR || tipo == Tt::MAP ||
         tipo == Tt::SET;
}

bool esTipo(Tt tipo) {
  return tipo == Tt::BYTE   || tipo == Tt::CHAR   || tipo == Tt::SHORT ||
         tipo == Tt::INT    || tipo == Tt::STRING || tipo == Tt::FLOAT ||
         tipo == Tt::DOUBLE || tipo == Tt::BOOL   || tipo == Tt::SLICE ||
         esTipoComp(tipo);
}

std::string tipoString(const Dt& tipo) {
  return std::visit(overloaded{
    [](TipoPrimitivo p) -> std::string {
      switch (p) {
        case TipoPrimitivo::SHORT      : { return "short"   ; }
        case TipoPrimitivo::INT        : { return "int32"   ; }
        case TipoPrimitivo::LONG       : { return "int64"   ; }
        case TipoPrimitivo::VERY_LONG  : { return "int128"  ; }
        case TipoPrimitivo::FULL_LONG  : { return "int256"  ; }

        case TipoPrimitivo::FLOAT      : { return "float32" ; }
        case TipoPrimitivo::DOUBLE     : { return "float64" ; }
        case TipoPrimitivo::LONG_DOUBLE: { return "float128"; }

        case TipoPrimitivo::BOOL       : { return "bool"    ; }
        case TipoPrimitivo::BYTE       : { return "byte"    ; }
        case TipoPrimitivo::CHAR       : { return "char"    ; }

        case TipoPrimitivo::RANGO      : { return "slice"   ; }

        case TipoPrimitivo::DESCONOCIDO: { return "unknown" ; }
        //...
        default                        : { return "type"    ; }
      }
    },
    [](const TipoUsuario& u) {
      return u.nombre;
    },
    [](const std::shared_ptr<TipoPuntero>& ptr) {
      if (!ptr) { return std::string("null_ptr"); }
      return tipoString(ptr->tipo_base) + "*";
    }

  }, tipo.valor);
}

/* --- Colores para la terminal --- */

const std::string RESET = "\033[0m";
const std::string RED = "\033[1;31m";
const std::string GREEN = "\033[1;32m";
const std::string YELLOW = "\033[1;33m";
const std::string BLUE = "\033[1;34m";
const std::string MAG = "\033[1;35m";
const std::string CYAN = "\033[1;36m";

