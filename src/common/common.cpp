// common.cpp

#include "Common.hpp"

/* --- Manejo de Tipos --- */

std::string nombreTipo(Tt tipo) {
  switch (tipo) {
    case Tt::VAR: return "VAR";
    case Tt::IDENTIFICADOR: return "IDENTIFICADOR";
    case Tt::NUMERO: return "NUMERO";
    case Tt::PUNTO_COMA: return "PUNTO_COMA";
    case Tt::COMA: return "COMA";
    case Tt::PUNTO: return "PUNTO";
    case Tt::IGUAL_ASIG: return "IGUAL";
    case Tt::FIN_ARCHIVO: return "EOF";
    default: return "DESCONOCIDO";
  }
}

bool esModificador(Tt tipo) {
  return tipo == Tt::NAT      || tipo == Tt::EXO     ||
         tipo == Tt::MAGNO    || tipo == Tt::ILUSTRE ||
         tipo == Tt::COMPLEJO || tipo == Tt::ETERNO;
}

bool esInfiere(Tt tipo) {
  return tipo == Tt::VAR || tipo == Tt::ETERNO;
}

bool esTipoComp(Tt tipo) {
  return tipo == Tt::TOMO  || tipo == Tt::SAGA     ||
         tipo == Tt::PACTO || tipo == Tt::GRIMORIO ||
         tipo == Tt::ACERVO;
}

bool esTipo(Tt tipo) {
  return tipo == Tt::BYTE  || tipo == Tt::RUNA      || tipo == Tt::WYN    ||
         tipo == Tt::DOX   || tipo == Tt::PERGAMINO || tipo == Tt::REAL   ||
         tipo == Tt::VASTO || tipo == Tt::DUAL      || tipo == Tt::UMBRAL ||
         esTipoComp(tipo);
}

/* --- Colores para la terminal --- */

const std::string RESET = "\033[0m";
const std::string RED = "\033[1;31m";
const std::string GREEN = "\033[1;32m";
const std::string YELLOW = "\033[1;33m";
const std::string BLUE = "\033[1;34m";
const std::string MAG = "\033[1;35m";
const std::string CYAN = "\033[1;36m";

