// error.cpp

#include "Error.hpp"

#include "Common.hpp"
#include "Includes.hpp"

ErrorHandler::ErrorHandler() {}

std::string_view ErrorHandler::getTemplate(CE ce) {
  switch (ce) {
    case CE::E_EXPECTED_TOKEN: { return "Se esperaba '{}' pero se encontró '{}'"; }
    case CE::E_EXPECTED_EXPRESSION: { return "Se esperaba una expresión antes de '{}'"; }

    default: { return "Error desconocido"; }
  }

}

void ErrorHandler::printSourceLine(Pos pos) {

}

void ErrorHandler::show() {
  for (const auto& e : errores) {
    switch (e.kind) {
      case EK::ERROR  : { std::cout << COLOR_RED    << "[Error]"  ; break; }
      case EK::WARNING: { std::cout << COLOR_YELLOW << "[Warning]"; break; }
      case EK::INFO   : { std::cout << COLOR_BLUE   << "[Info]"   ; break; }
      defualt: { break; }
    }

    std::cout << COLOR_RESET << " ";

    std::cout << e.msg << '\n';

    printSourceLine(e.pos);

  }

}

bool ErrorHandler::checkErrors  () { return errorCount  ; }

bool ErrorHandler::checkWarnings() { return warningCount; }
