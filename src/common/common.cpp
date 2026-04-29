// common.cpp

#include "Common.hpp"

#include "Includes.hpp"

/* --- Manejo de Tipos --- */

std::string nombreTipo(Tt tipo) {
  switch (tipo) {
    case Tt::VAR: return "VAR";
    case Tt::IDENTIFICADOR: return "IDENTIFICADOR";
    case Tt::NUMERO: return "NUMERO";

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

    case Tt::ARCANE: return "ARCANE";
    case Tt::RULES : return "REGLAS";
    case Tt::KEY   : return "KEY"   ;
    case Tt::EXPR  : return "EXPR"  ;
    case Tt::CODE  : return "CODE"  ;

    default: return "DESCONOCIDO";
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
  return tipo == Tt::VECTOR || tipo == Tt::MAP || tipo == Tt::SET;
}

bool esTipo(Tt tipo) {
  return tipo == Tt::BYTE   || tipo == Tt::CHAR   || tipo == Tt::SHORT  ||
         tipo == Tt::INT    || tipo == Tt::UINT   || tipo == Tt::STRING ||
         tipo == Tt::FLOAT  || tipo == Tt::DOUBLE || tipo == Tt::BOOL   ||
         tipo == Tt::SLICE  || esTipoComp(tipo);
}

bool Dt::operator==(const Dt& otro) const { //... Comparar this.es_const
  // Si ambos son nulos, son iguales
  if (!this->valor && !otro.valor) { return true ; }
  // Si uno es uno y el otro no, son distintos
  if (!this->valor || !otro.valor) { return false; }

  return this->valor->esIgual(otro.valor.get());
}

bool Dt::esPrimitivo() const { //...
  switch(valor->kind) {
    case TypeKind::VOID   :
    case TypeKind::BOOLEAN:
    case TypeKind::INTEGER:
    case TypeKind::FLOAT  :
    case TypeKind::ARRAY  : {
      return true;
    }

    default: {
      return false;
    }

  }
}

std::string Dt::tipoString() const {
  if (valor == nullptr) { return "???"; }
  return (es_const? "const": "") + valor->toString();
}

/* --- Gestor de Tablas Maestro --- */

GestorTablas::GestorTablas() {
  root = std::make_unique<Scope>(nullptr);
  scopeActual = root.get();

}

void GestorTablas::prepareForEmitter() {
  lectura = true;
  root->resetNavegacion();
  scopeActual = root.get();

}

void GestorTablas::entrarScope() {
  std::cout << "[110, common.cpp] entrarScope\n";
  if (!lectura) {
    auto nuevo_hijo = std::make_unique<Scope>(scopeActual);
    Scope* ptr_hijo = nuevo_hijo.get();
    scopeActual->hijos.push_back(std::move(nuevo_hijo));
    scopeActual = ptr_hijo;

  } else {
    if (scopeActual->hijo_actual < scopeActual->hijos.size()) {
      scopeActual = scopeActual->hijos[scopeActual->hijo_actual++].get();
    }
  }
}

void GestorTablas::salirScope() {
  std::cout << "[125, common.cpp] salirScope\n";
  if (scopeActual->padre) {
    scopeActual = scopeActual->padre;

  } else {
    std::cerr << "Error: Intento de salir de un scope raíz o nulo.\n";

  }
}

InfoVariable* GestorTablas::buscarVariable(const std::string& name) {
  Scope* cursor = scopeActual;
  while (cursor != nullptr) {
    auto it = cursor->variables.find(name);
    if (it != cursor->variables.end()) {
      return &(it->second);
    }
    cursor = cursor->padre;
  }
  return nullptr;

}

bool GestorTablas::añadirVariable(const std::string& name, InfoVariable info) {

  if (scopeActual->variables.find(name) != scopeActual->variables.end()) {
    return false;

  }

  scopeActual->variables[name] = std::move(info);
  return true;

}

bool GestorTablas::añadirFunction(const std::string& name, InfoFuncion info) {
  if (scopeActual->funciones.count(name)) {
    return false;

  }

  scopeActual->funciones[name] = std::move(info);
  return true;

}

InfoFuncion* GestorTablas::buscarFunction(const std::string& name) {
  Scope* cursor = scopeActual;

  while (cursor != nullptr) {
    auto it = cursor->funciones.find(name);

    if (it != cursor->funciones.end()) {
      return &(it->second);

    }

    cursor = cursor->padre;
  }

  return nullptr;
}

InfoFuncion* GestorTablas::getCurrentFunction() {

  if (!pilaFuncs.empty()) {
    return pilaFuncs.back();

  }

  return nullptr;

}

void GestorTablas::pushFunction(InfoFuncion* function) { pilaFuncs.push_back(function);   }

void GestorTablas::popFunction() {
  if (!pilaFuncs.empty()) { pilaFuncs.pop_back(); }

}

/* --- Colores para la terminal --- */

inline const std::string COLOR_RESET   = "\033[0m"   ;
inline const std::string COLOR_RED     = "\033[1;31m";
inline const std::string COLOR_GREEN   = "\033[1;32m";
inline const std::string COLOR_YELLOW  = "\033[1;33m";
inline const std::string COLOR_BLUE    = "\033[1;34m";
inline const std::string COLOR_MAGENTA = "\033[1;35m";
inline const std::string COLOR_CYAN    = "\033[1;36m";

