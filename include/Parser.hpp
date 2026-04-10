// Parser.hpp

#pragma once

#include "Common.hpp"

class Parser {
private:
  std::vector<Token> tokens;
  unsigned long long pos;
  std::unordered_map<std::string, Tt> aliasLexicos;
  std::unordered_map<std::string, DefinicionArcano> arcanosActivos;

  TypeFactory& typeFactory;

  Token resolverAlias(Token t);
  Token peek();
  Token get();
  Token check(Tt tipoEsperado);

  Token coincide(std::initializer_list<Tt> tipos);

public:
  Parser(std::vector<Token> t, TypeFactory& tf);

  InfoVariable parsearTipo();

  std::unique_ptr<Expresion> parsearRangoOArray();

  std::unique_ptr<Expresion> parsearRango();
  std::unique_ptr<Expresion> parsearAcceso(std::unique_ptr<Expresion> contenedor);
  std::pair<std::string, std::string> partirLexemaNum(std::string lexema);
  std::unique_ptr<Expresion> parsearPrefijo();
  std::unique_ptr<Expresion> parsearCasteo();

  std::unique_ptr<Sentencia> parsearEscritura();
  std::unique_ptr<Sentencia> parsearDeclaracionVar();
  std::unique_ptr<Sentencia> parsearSentenciaExpresion();
  std::unique_ptr<Sentencia> parsearBloqSent();
  std::unique_ptr<Sentencia> parsearBloque();
  std::unique_ptr<Sentencia> parsearSentencia();
  std::unique_ptr<Sentencia> parsearSi();
  std::unique_ptr<Sentencia> parsearSino();
  std::unique_ptr<Sentencia> parsearMientras();

  std::unique_ptr<Sentencia> parsearReturn();
  std::unordered_map<std::string, InfoVariable> parsearFuncArgs();
  std::unique_ptr<Sentencia> parsearFuncDecl();

  std::pair<std::string, Regla> parsearReglaArcano();
  std::map<std::string, Regla> parsearReglasArcano();
  std::map<std::string, std::unique_ptr<Sentencia>> parsearCuerpoArcano();
  std::unique_ptr<Sentencia> parsearArcano();
  std::unique_ptr<Sentencia> parsearLlamadaArcano();

  Pr obtenerPrecedencia(Tt tipo);

  std::unique_ptr<Expresion> parsearExpresion(Pr precedenciaMinima);

  std::vector<std::unique_ptr<Sentencia>> parsearPrograma();
};
