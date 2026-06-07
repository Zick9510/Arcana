// Parser.hpp

#pragma once

#include "Common.hpp"

class Parser {
private:
  std::vector<Token> tokens;
  unsigned long long pos;
  std::unordered_map<std::string, Tt> aliasLexicos;

  GestorTablas   & tablas         ;
  ErrorHandler   & errHandler     ;
  ContextoArcanos& contextoArcanos;
  TypeFactory    & typeFactory    ;

  Token resolverAlias(Token t);
  Token peek(size_t offset = 0);
  Token get();
  Token check(Tt tipoEsperado, Pm parseMode = Pm::STRICT);

  Token coincide(std::initializer_list<Tt> tipos);

  bool sync(Tt target);

  bool isType(Token t);

public:
  Parser(std::vector<Token> tok, GestorTablas& t,  ErrorHandler& e, ContextoArcanos& ca, TypeFactory& tf);

  bool isStatement(Token t);

  InfoVariable parsearTipo();

  std::unique_ptr<Expresion> parsearRangoOArray();

  std::unique_ptr<Expresion> parsearRango();
  std::unique_ptr<Expresion> parsearAcceso(std::unique_ptr<Expresion> contenedor);
  std::pair<std::string, std::string> partirLexemaNum(std::string lexema);
  std::unique_ptr<Expresion> parsearCasteo();

  std::unique_ptr<Expresion> parsearFunctionCall(std::unique_ptr<Expresion> callee);

  std::unique_ptr<Expresion> parsearInitList();

  std::unique_ptr<Sentencia> parsearEscritura();
  std::unique_ptr<Sentencia> parsearDeclaracionVar();
  std::unique_ptr<Sentencia> parsearSentenciaExpresion();

  std::unique_ptr<Sentencia> parsearBloqSent();
  std::vector<BT>            parsearTraits();
  std::unique_ptr<Sentencia> parsearBloque();

  std::unique_ptr<Sentencia> parsearSentencia();
  std::unique_ptr<Sentencia> parsearSi();
  std::unique_ptr<Sentencia> parsearSino();
  std::unique_ptr<Sentencia> parsearMientras();
  std::unique_ptr<Sentencia> parsearBreak();
  std::unique_ptr<Sentencia> parsearContinue();
  std::unique_ptr<Sentencia> parsearRedo();

  std::unique_ptr<Sentencia> parsearReturn();
  std::vector<std::pair<std::string, InfoVariable>> parsearFuncArgs(Tt end = Tt::PAREN_R);
  std::unique_ptr<Sentencia> parsearFuncDecl();

  std::pair         <std::string,              ReglaArcano > parsearReglaArcano();
  std::vector       <std::pair  < std::string, ReglaArcano>> parsearReglasArcano();
  std::unordered_map<std::string, std::vector<EnlaceCadena>> parsearCadenasArcano(const std::vector<std::pair<std::string, ReglaArcano>>& reglasDeclaradas);
  std::vector<ArcaneBranch> parsearCuerpoArcano(
    const std::vector<std::pair<std::string, ReglaArcano>>& reglasDeclaradas
  );
  std::unique_ptr<Sentencia> parsearArcano();
  std::unique_ptr<Sentencia> parsearLlamadaArcano(bool checkSc = true);
  std::unique_ptr<Sentencia> parsearMetaDirectiva();

  std::unique_ptr<Sentencia> parsearStruct();

  std::unique_ptr<Sentencia> parsearTemplate();

  Pr obtenerPrecedencia(Tt tipo);

  std::unique_ptr<Expresion> parsearPrefijo();
  std::unique_ptr<Expresion> parsearExpresion(Pr precedenciaMinima);

  std::vector<std::unique_ptr<Sentencia>> parsearPrograma();

};
