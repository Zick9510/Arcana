#pragma once

#include "Includes.hpp"

/* --- Type System --- */

enum class TypeKind { //...

  VOID,

  POINTER,

  BOOLEAN,
  INTEGER,
  FLOAT,

  CHAR,
  STRING,

  ARRAY,

  MORPH,

  STRUCT,

  ERROR,

  DESCONOCIDO,

};

class ArcanaType {
public:
  TypeKind kind;

  ArcanaType(TypeKind k)
    : kind(k) {}

  virtual ~ArcanaType()                                            ;
  virtual std::string toString()                          const = 0;
  virtual int getBitSize()                                const = 0;
  virtual bool esIgual(const ArcanaType* otro)            const = 0;
  virtual bool isNumeric()                                const { return true   ; }
  virtual bool isSigned()                                 const { return false  ; }
  virtual std::shared_ptr<ArcanaType> getUnderlyingType() const { return nullptr; }

};

struct Dt {
  std::shared_ptr<ArcanaType> valor;
  bool es_const = false;

  Dt(std::shared_ptr<ArcanaType> v)
    : valor(v) {}

  Dt() : valor(nullptr) {}

  bool operator==(const Dt& otro) const;

  bool esPrimitivo() const;

  std::string tipoString() const;
};

struct InfoVariable {
  Dt tipo;
  bool es_const = false;

  llvm::AllocaInst* alloca = nullptr;

};

struct InfoFuncion {
  std::string nombre;
  Dt tipo_retorno;
  std::vector<std::pair<std::string, InfoVariable>> tipos_parametros;
};

struct InfoStruct {
  std::string nombre;

  std::unordered_map<std::string, InfoVariable> propiedades;
  std::unordered_map<std::string, InfoFuncion > metodos    ;

  std::vector<std::string> orden_props;

};

struct Scope {
  std::unordered_map<std::string, InfoVariable> variables;
  std::unordered_map<std::string, InfoFuncion > funciones;
  std::unordered_map<std::string, InfoStruct  > structs  ;

  Scope* padre = nullptr;
  std::vector<std::unique_ptr<Scope>> hijos;
  size_t hijo_actual = 0;

  explicit Scope(Scope* padrePtr = nullptr) : padre(padrePtr) {}

  Scope(const Scope&)            = delete;
  Scope& operator=(const Scope&) = delete;
  Scope(Scope&&)            = default;
  Scope& operator=(Scope&&) = default;

  void resetNavegacion() {
    hijo_actual = 0;
    for (auto& hijo : hijos) { hijo->resetNavegacion(); }
  }

};

class GestorTablas {
private:
  std::unique_ptr<Scope> root;
  Scope* scopeActual;

  bool lectura = false;
  std::vector<InfoFuncion*> pilaFuncs;

public:
  GestorTablas();

  void prepareForEmitter();

  void entrarScope();
  void salirScope();

  // --- Variables ---
  bool añadirVariable(const std::string& name, InfoVariable info);
  InfoVariable* buscarVariable(const std::string& name);

  // --- Functions ---
  bool añadirFunction(const std::string& name, InfoFuncion info);
  InfoFuncion* buscarFunction(const std::string& name);
  InfoFuncion* getCurrentFunction();

  void pushFunction(InfoFuncion* function);
  void popFunction();

  bool añadirStruct(const std::string& name, InfoStruct info);
  bool actualizarStruct(const std::string& name, InfoStruct infoActualizada);
  InfoStruct* buscarStruct(const std::string& name);

};
