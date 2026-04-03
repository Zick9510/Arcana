// Lexer.hpp

#pragma once

#include "Common.hpp"

class Lexer {
private:
  std::string_view source;
  std::vector<Token> tokens;
  size_t cursor = 0;
  int linea = 1;

  char actual() const;
  char peek() const;
  char get();

  bool match(char esperado);
  bool esFin() const;

public:
  Lexer(std::string_view src)
    : source(src) {}

  bool validarBase();
  bool validarCaracterBase(char caracter, char base);

  bool esSufijoNum();
  void consumirSufijoNum();

  void leerNumero();
  void leerStringChar();

  std::vector<Token> tokenize();
};
