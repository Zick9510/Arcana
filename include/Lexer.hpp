// Lexer.hpp

#pragma once

#include "Common.hpp"

class Lexer {
private:
  std::shared_ptr<SourceBuffer> buffer;
  std::vector<Token> tokens;
  size_t cursor = 0;
  size_t linea  = 1;

  char actual() const;
  char peek() const;
  char get();

  bool match(char esperado);
  bool esFin() const;

public:
  Lexer(std::shared_ptr<SourceBuffer> b)
    : buffer(b) {}

  bool validarBase();
  bool validarCaracterBase(char caracter, char base);

  bool esSufijoNum();
  void consumirSufijoNum();

  void leerNumero();
  void leerStringChar();

  void captureSymbol();

  std::vector<Token> tokenize();
};
