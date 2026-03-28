// Checker.hpp

#pragma once

#include "Common.hpp"

class Checker {
  private:
    std::vector<std::unique_ptr<Sentencia>> ast;

  public:
    Checker(std::vector<std::unique_ptr<Sentencia>>& a);

    Tt verificarOperandos(Tt izq, Tt der, const Tt op, int linea);

    void verificar(NodoAST* nodo);
    void verificarPrograma();
};
