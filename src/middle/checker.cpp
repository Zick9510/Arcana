// checker.cpp

#include "Common.hpp"

#include "Checker.hpp"

#include <algorithm>


/* --- Gestor de Tablas (Symbol Table Manager) --- */
GestorTablas::GestorTablas(ErrorHandler& e)
  : errHandler(e) {}

void GestorTablas::entrarBloque(Scope scope) {
  scopes.push_back(scope);
}

void GestorTablas::salirBloque() {
  if (scopes.size() > 1) {
    scopes.pop_back();
  }
}

bool GestorTablas::añadirVariable(const std::string& nombre, InfoVariable info, int linea) {
  // Comprobamos solo en el ámbito actual (redefinición)
  if (scopes.back().variables.count(nombre)) {
    std::vector<std::string> detalle = {nombre};
    errHandler.reportar(CE::ERR_VARIABLE_REDECLARADA, linea, detalle);
    return false;
  }
  //... Añadir comprobación de shadiwing
  scopes.back().variables[nombre] = info;
  return true;
}

InfoVariable* GestorTablas::buscarVariable(const std::string& nombre, int linea) {
  // Buscamos desde el ámbito actual hacia el global
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    if (it->variables.count(nombre)) {
      return &it->variables[nombre];
    }
  }
  std::vector<std::string> detalle = {nombre};
  errHandler.reportar(CE::ERR_VARIABLE_USO_SIN_DECLARAR, linea, detalle);
  return nullptr; // No encontrada
}

/* --- Checker --- */

Checker::Checker(std::vector<std::unique_ptr<Sentencia>>& a)
  : ast(std::move(a)) {}

Tt Checker::verificarOperandos(Tt izq, Tt der, const Tt op, int linea) {
  if (izq == Tt::ERROR || der == Tt::ERROR) { return Tt::ERROR; }
  return Tt::ERROR;
}

void Checker::verificar(NodoAST* nodo) {
  if (!nodo) { return; }

}

void Checker::verificarPrograma() {

}
