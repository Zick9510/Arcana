// error.cpp

#include "Error.hpp"

#include "Includes.hpp"
#include "Common.hpp"

ErrorHandler::ErrorHandler(std::shared_ptr<SourceBuffer> b)
  : buffer(b) {}

std::string_view ErrorHandler::getTemplate(CE ce) {
  switch (ce) {
    case CE::E_EXPECTED_TOKEN  : { return "expected '{}' but '{}' token was found"; }
    case CE::E_UNEXPECTED_TOKEN: { return "unexpected '{}' token"                 ; }
    case CE::E_EXPECTED_EXPRESSION: { return "expected expression but '{}' token was found"; }

    default: { return "unkown error"; }
  }

}

void ErrorHandler::show() {
  for (const auto& e : errores) {

    std::string color;

    if        (e.kind == EK::ERROR  ) {
      color = COLOR_RED  ;
      std::cout << color << "[error]"  ;

    } else if (e.kind == EK::WARNING) {
      color = COLOR_YELLOW;
      std::cout << color << "[warning]";

    } else if (e.kind == EK::INFO   ) {
      color = COLOR_BLUE  ;
      std::cout << color << "[info]"   ;

    } else                            {
      color = COLOR_RESET ;
      std::cout << color << "[]"       ;
    }

    std::cout << COLOR_RESET << " C" << static_cast<int>(e.ce) << ": " << e.msg << '\n';

    ResolvedPos res = buffer->resolvePos(e.pos.cur);

    std::string_view source_line = buffer->getLine(res.line);

    std::string line_num = std::to_string(res.line);
    std::string padding(line_num.length(), ' ');

    std::cout << COLOR_MAGENTA << line_num << " | " << COLOR_RESET << source_line << '\n';
    std::cout << padding << "   " << COLOR_RESET;

    for (size_t i = 0; i < res.col - 1 && i < source_line.length(); ++i) {
      if   (source_line[i] == '\t') { std::cout << '\t'; }
      else             { std::cout << ' ' ; }

    }

    std::cout << color;
    if (e.pos.len > 1) {
      for (size_t i = 0; i < e.pos.len - 1;   i++) { std::cout << "~"; }
    }

    std::cout << "^" << COLOR_RESET << '\n';

  }

  std::cout << '\n';

}

bool ErrorHandler::checkErrors  () { return errorCount  ; }

bool ErrorHandler::checkWarnings() { return warningCount; }
