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
  std::cout << "[25, checker.cpp]\n";
  if (scopes.empty()) {
    scopes.push_back(Scope());
    std::cout << "[27, checker.cpp]\n";
    scopes.back().variables[nombre] = info;
    return true;
  }

  std::cout << "[30, checker.cpp]\n";

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
  std::cout << "[48, checker.cpp] " << nombre << '\n';
  // Buscamos desde el ámbito actual hacia el global
  for (auto it = scopes.rbegin(); it != scopes.rend(); ++it) {
    if (it->variables.count(nombre)) {
      std::cout << "[52, checker.cpp]\n";
      return &it->variables[nombre];
    }
  }
  //std::vector<std::string> detalle = {nombre};
  //errHandler.reportar(CE::ERR_VARIABLE_USO_SIN_DECLARAR, linea, detalle);
  std::cout << "[58, checker.cpp]\n";
  return nullptr; // No encontrada
}

/* --- Checker --- */

Checker::Checker(GestorTablas& t, std::vector<std::unique_ptr<Sentencia>>& a, ErrorHandler& e, TypeFactory& tf)
  : tablas(t), ast(a), errHandler(e), typeFactory(tf) {}

// --- Verificar Expresiones ---
std::shared_ptr<ArcanaType> Checker::verificarSuma(const Dt& izq, const Dt& der) {

  if (izq.esPrimitivo() && der.esPrimitivo()) {
    TypeKind pIzq = izq.valor->kind;
    TypeKind pDer = der.valor->kind;

    // Regla 1: Suma de números
    if (esNum(pIzq) && esNum(pDer)) {
      return promoverTipos(izq.valor, der.valor);
    }

    //... Regla 2: Concatenación de strings

    //... Regla 3: Concatenación de arrays (Maps, sets, etc.)
 
  }

  // Si el código llega acá, se intentó sumar cosas inválidas
  //... Reportar al errHandler

  return nullptr; //... Quizás retornar un tipo error?

}

std::shared_ptr<ArcanaType> Checker::verificarResta(const Dt& izq, const Dt& der) {
  if (izq.esPrimitivo() && der.esPrimitivo()) {
    TypeKind pIzq = izq.valor->kind;
    TypeKind pDer = der.valor->kind;

    // Regla 1: Resta de números
    if (esNum(pIzq) && esNum(pDer)) {
      return promoverTipos(izq.valor, der.valor);
    }

  }

  //... Reportar al errHandler
  return nullptr;

}

std::shared_ptr<ArcanaType> Checker::verificarMult(const Dt& izq, const Dt& der) {

  if (izq.esPrimitivo() && der.esPrimitivo()) {
    TypeKind pIzq = izq.valor->kind;
    TypeKind pDer = der.valor->kind;

    // Regla 1: Multiplicación de números
    if (esNum(pIzq) && esNum(pDer)) {
      return promoverTipos(izq.valor, der.valor);
    }

    // Regla 2: Multiplicar una string con un entero
    //...

  }

  //... Reportar al errHandler
  return nullptr;

}

std::shared_ptr<ArcanaType> Checker::verificarDiv(const Dt& izq, const Dt& der) {

  if (izq.esPrimitivo() && der.esPrimitivo()) {
    TypeKind pIzq = izq.valor->kind;
    TypeKind pDer = der.valor->kind;

    // Regla 1: División de números
    if (esNum(pIzq) && esNum(pDer)) { //... Ajustar esto para que retorne como mínimo, float64
      return promoverTipos(izq.valor, der.valor);
    }

  }

  //... Reportar al errHandler
  return nullptr;

}

std::shared_ptr<ArcanaType> Checker::verificarPotencia(const Dt& izq, const Dt& der) {

  if (izq.esPrimitivo() && der.esPrimitivo()) {
    std::cout << "[143 checker.cpp]\n";
    TypeKind pIzq = izq.valor->kind;
    TypeKind pDer = der.valor->kind;
    std::cout << "[146 checker.cpp]\n";

    // Regla 1: Potenciación de números
    if (esNum(pIzq) && esNum(pDer)) { //... Ojo con (-x) ** ( 1 / (2n) )
      // Obtenemos el tipo más preciso de los dos
      std::shared_ptr<ArcanaType> promovido = promoverTipos(izq.valor, der.valor);
      std::cout << "[152 checker.cpp]\n";
      TypeKind pProm = promovido->kind;
      std::cout << "[154 checker.cpp]\n";

      if (esFloat(pProm)) {
        // Si es flotante, el piso es double
        if (obtenerRangoNum(pProm) < obtenerRangoNum(TypeKind::FLOAT)) {
          return typeFactory.getFloat(64);
        }

      } else {
        // Si es entero, el piso es long
        if (obtenerRangoNum(pProm) < obtenerRangoNum(TypeKind::INTEGER)) {
          return typeFactory.getInteger(64, false);

        }

      }

      // El tipo promovido ya era más preciso que long o double
      return promovido;

    }

  }

  //... Reportar al errHandler
  std::cout << "[179 checker.cpp]\n";
  return nullptr;

}

Dt Checker::verificarOperandos(const Dt& izq, const Dt& der, const TipoOperador op) { //...

  //... Añadir comprobación de error o desconocido en izq y der

  if (!izq.valor || !der.valor) {
    //...
    std::cout << "[190, checker.cpp]\n";
  }

  if (izq.valor->kind == TypeKind::DESCONOCIDO || der.valor->kind == TypeKind::DESCONOCIDO) {
    //...
    std::cout << "[195, checker.cpp]\n";

  }

  switch(op) { //... Añadir más casos

    case TipoOperador::A_SUMA: {
      return verificarSuma(izq, der);
    }

    case TipoOperador::A_RESTA: {
      return verificarResta(izq, der);
    }

    case TipoOperador::A_MULT: {
      return verificarMult(izq, der);
    }

    case TipoOperador::A_DIV: {
      return verificarDiv(izq, der);
    }

    case TipoOperador::A_POT: {
      return verificarPotencia(izq, der);
    }

    default: {
      std::cout << "[222 checker.cpp] Operador desconocido: " << operadorString(op) << "\n";
      //... Retornar algo
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
    TypeKind pO = origen.valor->kind;
    TypeKind pD = destino.valor->kind;

    if (esNum(pO) && esNum(pD)) {
      //... Comprobar pérdida de precisión
      return true;
    }

    //... Char -> int
 
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
