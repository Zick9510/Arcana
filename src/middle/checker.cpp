// checker.cpp

#include "Checker.hpp"

#include "Common.hpp"

/* --- Checker --- */
Checker::Checker(GestorTablas& t, std::vector<std::unique_ptr<Sentencia>>& a, ErrorHandler& e, TypeFactory& tf, ContextoArcanos& ca)
  : tablas(t), ast(a), errHandler(e), typeFactory(tf), contextoArcanos(ca) {}


// --- Casteos --- //
std::unique_ptr<Expresion> Checker::forzarTipo(std::unique_ptr<Expresion> hijo, const Dt& tipoEsperado) {

  if (!hijo || !hijo->tipo_resuelto.valor || !tipoEsperado.valor) { return hijo; }

  Dt tipo_actual = hijo->tipo_resuelto;

  if (tipo_actual == tipoEsperado) {
    return hijo;

  }

  if (esCasteoValido(tipo_actual, tipoEsperado)) {
    auto casteo = std::make_unique<ExprCasteo>(std::move(hijo), tipoEsperado);
    casteo->tipo_resuelto = tipoEsperado;
    casteo->es_implicito  = true;

    //... Warning, implicit cast

    return casteo;

  }

  //... Error, bad cast

  return hijo;

}

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
    // Regla 3: Multipliar un array con un entero ([x] * 3 == [x] + [x] + [x] == [x, x, x])
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
    std::cout << "[217 checker.cpp]\n";
    TypeKind pIzq = izq.valor->kind;
    TypeKind pDer = der.valor->kind;
    std::cout << "[220 checker.cpp]\n";

    // Regla 1: Potenciación de números
    if (esNum(pIzq) && esNum(pDer)) { //... Ojo con (-x) ** ( 1 / (2n) )

      // Obtenemos el tipo más preciso de los dos
      std::shared_ptr<ArcanaType> promovido = promoverTipos(izq.valor, der.valor);
      std::cout << "[227 checker.cpp]\n";
      TypeKind pProm = promovido->kind;
      std::cout << "[229 checker.cpp]\n";

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
  return nullptr;

}

std::shared_ptr<ArcanaType> Checker::verificarSwap(const Dt& izq, const Dt& der) {

  if (izq.esPrimitivo() && der.esPrimitivo()) {
    if (izq == der) {
      return der.valor;

    }
  }

  return nullptr;

}

std::shared_ptr<ArcanaType> Checker::verificarCmpMenor(const Dt& izq, const Dt& der) {

  if (izq.esPrimitivo() && der.esPrimitivo()) {
    TypeKind pIzq = izq.valor->kind;
    TypeKind pDer = der.valor->kind;

    // Regla 1: Comparación de números
    if (esNum(pIzq) && esNum(pDer)) {
      return typeFactory.getBoolean();

    }

  }

  //... Reportar al errHandler
  return nullptr;

}

Dt Checker::verificarOperandos(const Dt& izq, const Dt& der, const TipoOperador op) { //...

  //... Añadir comprobación de error o desconocido en izq y der

  //...
  if (!izq.valor) {
    std::cout << "[282, checker.cpp] izq null\n";
  }

  if (!der.valor) {
    std::cout << "[286, checker.cpp] der null\n";
  }

  if (izq.valor->kind == TypeKind::DESCONOCIDO || der.valor->kind == TypeKind::DESCONOCIDO) {
    //...
    std::cout << "[291, checker.cpp]\n";

  }

  switch(op) { //... Añadir más casos

    // Aritméticos

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

    case TipoOperador::A_SWAP: {
      return verificarSwap(izq, der);
    }
    // Comparadores

    case TipoOperador::CMP_MENOR: {
      return verificarCmpMenor(izq, der);
    }

    default: {
      std::cout << "[326 checker.cpp] Operador desconocido: " << operadorString(op) << '\n';
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

    if (esNum(pO) && pD == TypeKind::BOOLEAN) {
      return true;
    }

    if (pO == TypeKind::BOOLEAN && esNum(pD)) {
      return true;
    }

    if (pO == TypeKind::CHAR && esNum(pD)) {
      return true;
    }

    //... Char -> int
 
  }

  //... Relga 3: Punteros

  // No se puede
  return false;

}

void Checker::verificarPrograma() {

  //tablas.entrarScope();

  for (auto& nodo : ast) {
    nodo->accept(this);
  }

  //tablas.salirScope();
}
