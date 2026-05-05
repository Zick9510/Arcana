// error.cpp

#include "Error.hpp"

#include "Includes.hpp"

ErrorHandler::ErrorHandler() {}

std::string_view ErrorHandler::getTemplate(CE ce) {
  switch (ce) {
    case CE::E_EXPECTED_TOKEN: { return "Se esperaba '{}' pero se encontró '{}'"; }
    default: { return "Error desconocido"; }
  }
}

void ErrorHandler::show() {
  for (const auto& e : errores) {
    std::cout << e.msg << '\n';
  }

}

bool ErrorHandler::checkErrors() {
  return errorCount;
}

bool ErrorHandler::checkWarnings() {
  return warningCount;
}
