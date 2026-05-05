// Error.hpp

#pragma once

#include "Includes.hpp"

struct Pos {
  size_t line;
  size_t col;
  size_t len;
};

/* --- Errores y Warnings --- */
enum class CodigoError { // Codigo Error

  // --- Errores Léxicos (1000) ---
  E_CARACTER_ILEGAL = 1000,
  E_NUMERO_MAL_FORMADO = 1001,

  E_NO_CERRADO_CADENA = 1100,
  E_NO_CERRADO_COMENTARIO = 1101,

  // --- Errores Sintácticos (2000) ---
  E_EXPECTED_TOKEN = 2000, // Se esperaba un token que no está
  E_NO_ESPERABA = 2001, // No se esperaba un token que sí está

  E_DESBALANCE_PARENTESIS = 2100,
  E_DESBALANCE_LLAVES = 2101,
  E_DESBALANCE_CORCHETES = 2102,

  E_EXPECTED_EXPRESSION = 2200,
  E_EXPRESION_INVALIDA = 2201, // Ej: 2 +-+ 3. 3**


  // --- Erores Semánticos (3000) ---

  // Tipos (3000)
  E_TIPO_ESPERADO = 3000, // Se entrega un tipo T1 cuando se esperaba un tipo T2
  E_CASTEO_INVALIDO = 3001, // Se intenta hacer un casteo incompatible (ej: (int)string)

  // Variables (3100)
  E_VARIABLE_USO_SIN_DECLARAR = 3100, // Cuando se manipula una variable sin declarar
  E_VARIABLE_USO_SIN_DEFINIR = 3101, // Se usa una variable sin valor
  E_VARIABLE_REDECLARADA = 3102,
  E_CONSTANTE_MUTADA = 3103,

  // Funciones (3200)
  E_USO_SIN_DEFINIR_F = 3200, // Cuando se llama a una función cuyo nombre y firma no coincide
  E_CANTIDAD_ARGUMENTOS_INCORRECTA = 3201,
  E_RETORNO_INVALIDO = 3202,
  E_FALTA_RETORNO = 3203,

  // Extra (3900)
  E_NOMBRE_INVALIDO = 3900,

  // --- Warnings (>= 4000) ---
  W_VARIABLE_SIN_USAR = 4000,
  W_FUNCION_SIN_USAR = 4001,
  W_CODIGO_INACCESIBLE = 4002,
  W_CONVERSION_PELIGROSA = 4003, // Pérdida de precisión

};

using CE = CodigoError;

enum class ErrorKind {
  WARNING,
  ERROR,
  INFO
};

using EK = ErrorKind;

struct Error {
  CE ce;
  Pos pos;
  std::string msg;
  EK kind;
};

class ErrorHandler {
private:
  std::vector<Error> errores;

  size_t errorCount   = 0;
  size_t warningCount = 0;

public:
  ErrorHandler();

  std::string_view getTemplate(CE ce);

  template<typename... Args>
  void report(CE ce, Pos pos, Args&&... args) {
    Error err;
    err.ce  = ce ;
    err.pos = pos;
    std::string_view templ = getTemplate(ce);

    if constexpr (sizeof...(args) > 0) {
      err.msg = std::vformat(templ, std::make_format_args(args...));

    } else {
      err.msg = std::string(templ);
    }

    err.kind = (static_cast<int>(ce) >= 4000) ? EK::WARNING : EK::ERROR;

    if   (err.kind == EK::ERROR) { errorCount  ++; }
    else                         { warningCount++; }

    errores.push_back(err);

  }

  void show();

  bool checkErrors();
  bool checkWarnings();

};
