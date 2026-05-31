// checker.cpp

#include "Checker.hpp"

#include "Common.hpp"

/* --- Trait Checker Handler --- */
TraitChecker::TraitChecker(Checker& c)
  : checker(c) {}

void TraitChecker::despacharTrait(Bloque* nodo, size_t idx) {
  if (idx >= nodo->traits.size()) {
    for (const auto& inst : nodo->instrucciones) {
      inst->accept(&checker);
    }
    return ;
  }

  auto trait = nodo->traits[idx];

  switch (trait) {
    case BT::NOSCOPE: {
      handleNoscope(nodo, idx);
      break;
    }

    default: {
      despacharTrait(nodo, idx + 1);
      break;
    }

  }
}

void TraitChecker::handleNoscope(Bloque* nodo, size_t idx) {
  checker.tablas.salirScope();
  despacharTrait(nodo, idx + 1);
  checker.tablas.entrarScope();

}

/* --- Checker --- */
Checker::Checker(GestorTablas& t, std::vector<std::unique_ptr<Sentencia>>& a, ErrorHandler& e, TypeFactory& tf, ContextoArcanos& ca)
  : tablas(t), ast(a), errHandler(e), typeFactory(tf), contextoArcanos(ca), traits(*this) {}


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

    return casteo;

  }

  //... Error, bad cast

  return hijo;

}

// --- Verificar Expresiones --- //
std::shared_ptr<ArcanaType> Checker::verificarSuma(const Dt& izq, const Dt& der) {
  std::cout << "[45, checker.cpp] verificarSuma\n";
  if (izq.esPrimitivo() && der.esPrimitivo()) {
    std::cout << "[47, checker.cpp] ambos primitivos\n";
    TypeKind p_izq = izq.valor->kind;
    TypeKind p_der = der.valor->kind;

    // Regla 1: Suma de números
    if (esNum(p_izq) && esNum(p_der)) {
      return promoverTipos(izq.valor, der.valor);
    }

    //... Regla 2: Concatenación de strings

    //... Regla 3: Concatenación de arrays (Maps, sets, etc.)
 
  }

  std::cout << "[62, checker.cpp] alguno no primitivo\n";

  if (izq.valor->kind == TypeKind::STRUCT) {
    std::cout << "[65, checker.cpp] izq struct\n";
    auto struct_type =  std::static_pointer_cast<StructType>(izq.valor);
    std::string firma_buscada = struct_type->info->nombre + "_" + generarFirma(dunder::ADD, {der});

    std::cout << "[69, checker.cpp] firma: '" << firma_buscada << "'\n";
    auto it_metodo = struct_type->info->metodos.find(firma_buscada);
    if (it_metodo != struct_type->info->metodos.end()) {
      std::cout << "[72, checker.cpp]\n";
      return it_metodo->second.tipo_retorno.valor;

    }

  }

  // Si el código llega acá, se intentó sumar cosas inválidas
  //... Reportar al errHandler

  return nullptr; //... Quizás retornar un tipo error?

}

std::shared_ptr<ArcanaType> Checker::verificarResta(const Dt& izq, const Dt& der) {
  if (izq.esPrimitivo() && der.esPrimitivo()) {
    TypeKind p_izq = izq.valor->kind;
    TypeKind p_der = der.valor->kind;

    // Regla 1: Resta de números
    if (esNum(p_izq) && esNum(p_der)) {
      return promoverTipos(izq.valor, der.valor);
    }

  }

  //... Reportar al errHandler
  return nullptr;

}

std::shared_ptr<ArcanaType> Checker::verificarMult(const Dt& izq, const Dt& der) {

  if (izq.esPrimitivo() && der.esPrimitivo()) {
    TypeKind p_izq = izq.valor->kind;
    TypeKind p_der = der.valor->kind;

    // Regla 1: Multiplicación de números
    if (esNum(p_izq) && esNum(p_der)) {
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
    TypeKind p_izq = izq.valor->kind;
    TypeKind p_der = der.valor->kind;

    // Regla 1: División de números
    if (esNum(p_izq) && esNum(p_der)) { //... Ajustar esto para que retorne como mínimo, float64
      return promoverTipos(izq.valor, der.valor);
    }

  }

  //... Reportar al errHandler
  return nullptr;

}

std::shared_ptr<ArcanaType> Checker::verificarPotencia(const Dt& izq, const Dt& der) {

  if (izq.esPrimitivo() && der.esPrimitivo()) {
    std::cout << "[217 checker.cpp]\n";
    TypeKind p_izq = izq.valor->kind;
    TypeKind p_der = der.valor->kind;
    std::cout << "[220 checker.cpp]\n";

    // Regla 1: Potenciación de números
    if (esNum(p_izq) && esNum(p_der)) { //... Ojo con (-x) ** ( 1 / (2n) )

      // Obtenemos el tipo más preciso de los dos
      std::shared_ptr<ArcanaType> promovido = promoverTipos(izq.valor, der.valor);
      std::cout << "[227 checker.cpp]\n";
      TypeKind p_prom = promovido->kind;
      std::cout << "[229 checker.cpp]\n";

      if (esFloat(p_prom)) {
        // Si es flotante, el piso es double
        if (obtenerRangoNum(p_prom) < obtenerRangoNum(TypeKind::FLOAT)) {
          return typeFactory.getFloat(64);
        }

      } else {
        // Si es entero, el piso es long
        if (obtenerRangoNum(p_prom) < obtenerRangoNum(TypeKind::INTEGER)) {
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

    TypeKind p_izq = izq.valor->kind;
    TypeKind p_der = der.valor->kind;

    // Regla 1: Suma de números
    if (esNum(p_izq) && esNum(p_der)) {
      return promoverTipos(izq.valor, der.valor);
    }

  }

  return nullptr;

}

std::shared_ptr<ArcanaType> Checker::verificarTernary(const Dt& izq, const Dt& der) {
  return verificarSwap(izq, der);
}

std::shared_ptr<ArcanaType> Checker::verificarCmpIgualdad(const Dt& izq, const Dt& der) {

  if (izq.esPrimitivo() && der.esPrimitivo()) {
    TypeKind pIzq = izq.valor->kind;
    TypeKind pDer = der.valor->kind;

    // Regla 1: Números
    if (esNum(pIzq) && esNum(pDer)) {
      return typeFactory.getBoolean();

    }

    // Regla 2: Punteros
    if (pIzq == TypeKind::POINTER && pDer == TypeKind::POINTER) {
      return typeFactory.getBoolean();
    }

    // Regla 3: Booleanos
    if (pIzq == TypeKind::BOOLEAN && pDer == TypeKind::BOOLEAN) {
      return typeFactory.getBoolean();
    }

  }

  //... Reportar al errHandler
  return nullptr;

}

std::shared_ptr<ArcanaType> Checker::verificarCmpRelacional(const Dt& izq, const Dt& der) {

  if (izq.esPrimitivo() && der.esPrimitivo()) {
    TypeKind pIzq = izq.valor->kind;
    TypeKind pDer = der.valor->kind;

    // Regla 1: Números
    if (esNum(pIzq) && esNum(pDer)) {
      return typeFactory.getBoolean();

    }

    // Regla 2: Punteros
    if (pIzq == TypeKind::POINTER && pDer == TypeKind::POINTER) {
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

    case TipoOperador::SUMA: {
      return verificarSuma(izq, der);
    }

    case TipoOperador::RESTA: {
      return verificarResta(izq, der);
    }

    case TipoOperador::MULT: {
      return verificarMult(izq, der);
    }

    case TipoOperador::DIV: {
      return verificarDiv(izq, der);
    }

    case TipoOperador::POT: {
      return verificarPotencia(izq, der);
    }

    // Comparadores

    case TipoOperador::CMP_IGUAL   :
    case TipoOperador::CMP_DISTINTO: {
      return verificarCmpIgualdad(izq, der);
    }

    case TipoOperador::CMP_MAYOR      :
    case TipoOperador::CMP_MAYOR_IGUAL:
    case TipoOperador::CMP_MENOR_IGUAL:
    case TipoOperador::CMP_MENOR      : {
      return verificarCmpRelacional(izq, der);
    }

    // Extra

    case TipoOperador::SWAP: {
      return verificarSwap(izq, der);
    }

    case TipoOperador::TERNARY: {
      return verificarTernary(izq, der);
    }


    default: {
      std::cout << "[264 checker.cpp] Operador desconocido: " << operadorString(op) << '\n';
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

  for (auto& nodo : ast) {
    nodo->accept(this);
  }

  mode = ModoChecker::VERIFICACION;

  for (auto& nodo : ast) {
    nodo->accept(this);
  }

}
