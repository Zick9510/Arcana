// checker.cpp

#include "Checker.hpp"

#include "Common.hpp"

/* --- Gestor de Tablas (Symbol Table Manager) --- */
GestorTablas::GestorTablas(ErrorHandler& e, std::vector<Scope> s)
  : errHandler(e), scopes(s) {}

// --- Bloques ---
void GestorTablas::entrarBloque(Scope scope) {
  scopes.push_back(scope);
}

void GestorTablas::salirBloque() {
  if (scopes.size() > 1) {
    scopes.pop_back();
  }
}

// --- Variables ---
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

Checker::Checker(GestorTablas t, std::vector<std::unique_ptr<Sentencia>>& a, ErrorHandler& e)
  : tablas(t), ast(a), errHandler(e) {}

// --- Verificar Expresiones ---

Dt Checker::verificarSuma(const Dt& izq, const Dt& der) {

  if (izq.esPrimitivo() && der.esPrimitivo()) {
    TipoPrimitivo pIzq = std::get<TipoPrimitivo>(izq.valor);
    TipoPrimitivo pDer = std::get<TipoPrimitivo>(der.valor);

    // Regla 1: Suma de números
    if (esNum(pIzq) && esNum(pDer)) {
      return promoverTipos(pIzq, pDer);
    }

    // Regla 2: Concatenación de strings
    if (pIzq == TipoPrimitivo::STRING && //... Ajustar esto
        pDer == TipoPrimitivo::STRING) {
      return Dt(TipoPrimitivo::STRING);
    }
  }

  // Si el código llega acá, se intentó sumar cosas inválidas
  //... Reportar al errHandler
  return Dt(TipoPrimitivo::DESCONOCIDO);
}

Dt Checker::verificarResta(const Dt& izq, const Dt& der) {
  if (izq.esPrimitivo() && der.esPrimitivo()) {
    TipoPrimitivo pIzq = std::get<TipoPrimitivo>(izq.valor);
    TipoPrimitivo pDer = std::get<TipoPrimitivo>(der.valor);

    // Regla 1: Resta de números
    if (esNum(pIzq) && esNum(pDer)) {
      return promoverTipos(pIzq, pDer);
    }

  }

  //... Reportar al errHandler
  return Dt(TipoPrimitivo::DESCONOCIDO);
}

Dt Checker::verificarMult(const Dt& izq, const Dt& der) {

  if (izq.esPrimitivo() && der.esPrimitivo()) {
    TipoPrimitivo pIzq = std::get<TipoPrimitivo>(izq.valor);
    TipoPrimitivo pDer = std::get<TipoPrimitivo>(der.valor);

    // Regla 1: Multiplicación de números
    if (esNum(pIzq) && esNum(pDer)) {
      return promoverTipos(pIzq, pDer);
    }

    // Regla 2: Multiplicar una string con un entero
    //...

  }

  //... Reportar al errHandler
  return Dt(TipoPrimitivo::DESCONOCIDO);
}

Dt Checker::verificarOperandos(const Dt& izq, const Dt& der, const TipoOperador op) { //...

  if ( izq.es(TipoPrimitivo::DESCONOCIDO) ||
       der.es(TipoPrimitivo::DESCONOCIDO) ) {
    return Dt(TipoPrimitivo::DESCONOCIDO);
  }

  switch(op) { //...

    case TipoOperador::A_SUMA: {
      return verificarSuma(izq, der);
    }

    case TipoOperador::A_RESTA: {
      return verificarResta(izq, der);
    }

    case TipoOperador::A_MULT: {
      return verificarMult(izq, der);
    }

    default: {
      std::cout << "[77 checker.cpp] Operador desconocido: " << operadorString(op) << "\n";
      return Dt(TipoPrimitivo::DESCONOCIDO);
    }
  }
}

bool Checker::esCasteoValido(const Dt& origen, const Dt& destino) {
  // Regla 1: Identidad
  if (origen == destino) {
    //... Reportar Warning por casteo innecesario
    return true;
  }

  // Regla 2: Numérico
  if (origen.esPrimitivo() && destino.esPrimitivo()) {
    TipoPrimitivo pO = std::get<TipoPrimitivo>(origen.valor);
    TipoPrimitivo pD = std::get<TipoPrimitivo>(destino.valor);

    if (esNum(pO) && esNum(pD)) {
      //... Comprobar pérdida de precisión
      return true;
    }

    // Char -> int
    if (pO == TipoPrimitivo::CHAR && pD == TipoPrimitivo::INT) {
      return true;
    }
  }

  //... Relga 3: Punteros

  // No se puede
  return false;

}

void Checker::verificarPrograma() {
  for (auto& nodo : ast) {
    nodo->accept(this);
  }
}
