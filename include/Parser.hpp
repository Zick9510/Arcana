// Parser.hpp

#pragma once

#include "Common.hpp"

class Parser {
  private:
    std::vector<Token> tokens;
    unsigned long long pos;
    std::unordered_map<std::string, Tt> alias_lexicos;
    std::unordered_map<std::string, DefinicionArcano> arcanos_activos;

    Token resolverAlias(Token t);
    Token peek();
    Token get();
    Token check(Tt tipoEsperado);

    bool coincide(std::initializer_list<Tt> tipos);

  public:
    Parser(std::vector<Token> t);

    InfoTipo parsearTipo();

    std::unique_ptr<Expresion> parsearRango();
    std::unique_ptr<Expresion> parsearAcceso(std::unique_ptr<Expresion> contenedor);
    std::unique_ptr<Expresion> parsearPrefijo();

    std::unique_ptr<Sentencia> parsearEscritura();
    std::unique_ptr<Sentencia> parsearDeclaracionVar();
    std::unique_ptr<Sentencia> parsearSentenciaExpresion();
    std::unique_ptr<Sentencia> parsearBloqSent();
    std::unique_ptr<Sentencia> parsearBloque();
    std::unique_ptr<Sentencia> parsearSentencia();
    std::unique_ptr<Sentencia> parsearSi();
    std::unique_ptr<Sentencia> parsearSino();
    std::unique_ptr<Sentencia> parsearMientras();
    std::unique_ptr<Sentencia> parsearArcano();
    std::unique_ptr<Sentencia> parsearLlamadaArcano();

    int obtenerPrecedencia(Tt tipo);

    std::unique_ptr<Expresion> parsearExpresion(int precedenciaMinima);

    std::vector<std::unique_ptr<Sentencia>> parsearPrograma();
};
