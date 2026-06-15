#pragma once

#include "Includes.hpp"

#include "Error.hpp"

class ASTVisitor;

class NodoAST {
public:

  Pos pos;

  virtual ~NodoAST() = default;
  virtual void imprimir(int nivel = 0) const = 0;
  virtual void accept(ASTVisitor* visitor) = 0;

};

class Sentencia : public NodoAST {
public:
  virtual std::unique_ptr<Sentencia> clonar() const = 0;

};

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
  RANGE,

  MORPH,

  STRUCT,

  TEMPLATE_PARAM,
  TEMPLATE_INSTANCE,
  TEMPLATE_INSTANCE_STRUCT,

  ERROR,

  DESCONOCIDO,
  UNRESOLVED,

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
  bool operator<(const Dt& otro) const;

  bool esPrimitivo() const;

  std::string tipoString() const;

};

struct Scope;

struct InfoVariable {
  Dt tipo;
  bool es_const = false;

  llvm::AllocaInst* alloca = nullptr;
  llvm::Value* array_size = nullptr;

};

struct InfoFuncion {
  std::string nombre;
  Dt tipo_retorno;
  std::vector<std::pair<std::string, InfoVariable>> tipos_parametros;
  std::string firma;
  bool is_external = true;
  bool is_variadic = false;

};

struct InfoStruct {
  std::string nombre;

  std::unordered_map<std::string, InfoVariable> propiedades;
  std::unordered_map<std::string, InfoFuncion > metodos    ;

  std::vector<std::string> orden_props;

};

struct InfoTemplateParam {
  std::string nombre;

  std::optional<Dt> default_type;

  bool is_packed = false;

};

struct InfoTemplate {
  std::string nombre;
  std::vector<std::pair<std::string, std::variant<InfoTemplateParam, InfoVariable>>> args;
  std::unique_ptr<Sentencia> ast;
  Scope* scope_def;

};

struct Scope {
  std::unordered_map<std::string, InfoVariable> variables;
  std::unordered_map<std::string, std::vector<InfoFuncion>> funciones;
  std::unordered_map<std::string, InfoStruct  > structs  ;
  std::unordered_map<std::string, InfoTemplate> templates;
  std::unordered_map<std::string, std::string > typedefs ;

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
  std::vector<std::unordered_map<std::string, InfoTemplateParam>> pilaTempaltes;

public:
  GestorTablas();

  bool getLectura() const;
  void switchMode();

  // --- Scopes ---
  void entrarScope();
  Scope* getScopeActual();
  void setScopeActual(Scope* scope);
  void salirScope();
  void resetScope();
  void promoverScopes(size_t cantidad);

  // --- Variables ---
  bool añadirVariable(const std::string& name, InfoVariable info);
  InfoVariable* buscarVariable(const std::string& name);

  // --- Functions ---
  bool añadirFuncion(const std::string& name, InfoFuncion info);
  std::vector<InfoFuncion>* buscarFuncionName(const std::string& name);
  InfoFuncion* buscarFuncionFirma(const std::string& name, const std::string& firma);
  InfoFuncion* getCurrentFunction();

  void pushFunction(InfoFuncion* function);
  void popFunction();

  // --- Structs ---
  bool añadirStruct(const std::string& name, InfoStruct info);
  bool actualizarStruct(const std::string& name, InfoStruct infoActualizada);
  InfoStruct* buscarStruct(const std::string& name);

  // --- Templates ---
  void entrarScopeTemplate();
  void salirScopeTemplate();

  bool añadirTemplate(const std::string& name, InfoTemplate);
  bool updateTemplate(const std::string& name, Scope* scope);
  InfoTemplate* buscarTemplate(const std::string& name);

  bool añadirTemplateParam(const std::string& name, InfoTemplateParam);
  InfoTemplateParam* buscarTemplateParam(const std::string& name);

  // --- Typedefs ---
  bool añadirTypedef(const std::string& alias, const std::string& name);
  std::string resolverTypedef(const std::string& name);

};
