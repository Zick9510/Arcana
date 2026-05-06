// error.cpp

#include "Error.hpp"

#include "Includes.hpp"
#include "Common.hpp"

ErrorHandler::ErrorHandler(std::shared_ptr<SourceBuffer> b)
  : buffer(b) {}

std::string_view ErrorHandler::getTemplate(CE ce) {
  switch (ce) {
    case CE::E_EXPECTED_TOKEN: { return "expected '{}' but '{}' token was found"; }
    case CE::E_EXPECTED_EXPRESSION: { return "expected expression but '{}' token was found"; }

    default: { return "unkown error"; }
  }

}

void ErrorHandler::show() {
  for (const auto& e : errores) {
    switch (e.kind) {
      case EK::ERROR  : { std::cout << COLOR_RED    << "[error]"  ; break; }
      case EK::WARNING: { std::cout << COLOR_YELLOW << "[warning]"; break; }
      case EK::INFO   : { std::cout << COLOR_BLUE   << "[info]"   ; break; }
      default: { break; }
    }

    std::cout << COLOR_RESET << " ";

    std::cout << e.msg << '\n';
    std::cout << e.pos.line << " | " << buffer->getLine(e.pos.line) << '\n';

  }

}

bool ErrorHandler::checkErrors  () { return errorCount  ; }

bool ErrorHandler::checkWarnings() { return warningCount; }
