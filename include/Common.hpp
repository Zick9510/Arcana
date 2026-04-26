// Common.hpp

#pragma once

#include "Includes.hpp"

#include "Types.hpp"

// --- Precedencia --- 
enum class Precedencia : int {
  MINIMA = 0,

  ASIGNACION,   // =

  LOGICA_O,     // ||
  LOGICA_XOR,   // ^^
  LOGICA_Y,     // &&

  BIT_O,        // |
  BIT_XOR,      // ^
  BIT_Y,        // &
 
  IGUALDAD,     // == !=
  RELACIONAL,   // < > <= >=

  SHIFT,        // << >>
 
  SUMA,         // + -

  MULT,         // * / %

  UNARIO_DEBIL, // - + ~ (Ej: - 2 ** 3 se evalúa cómo - (2 ** 3))

  POTENCIA,     // ** */

  //USER_OPERATOR, //... Operadores del usuario

  PREFIJO,      // ++ -- *expr &expr !expr
  SUFIJO,       // ++ --

  ACCESO,       // expr[i]

  LLAMADA       // func() .method()

};

using Pr = Precedencia;

inline int operator-(Pr lhs, int rhs) {
  return static_cast<int>(lhs) - rhs;
}

// --- TipoToken ---
enum class Tt {
  // Tipos Inferibles
  VAR,

  // Constantes
  CONST,

  // Tipos de datos
  VOID,
  BYTE, CHAR, BOOL,
  SHORT, INT, UINT,
  FLOAT, DOUBLE,
  STRING,
  VECTOR, MAP, SET,

  SLICE,

  UMBRAL,

  ENUM,

  SHAPE,

  // Modificadores
  UNSIGNED, LONG, VERY_LONG, FULL_LONG, COMPLEJO,

  // Variables y Literales
  IDENTIFICADOR, NUMERO,

  // If-else
  IF, ELSE,

  // Loops
  DO, WHILE, FOR, FOREACH, LOOP,

  BREAK, CONTINUE, PASS,

  // Operadores
  MAS, MENOS, DIV, POTENCIA, RAIZ, MODULO, // La multiplicación es ASTERISCO

  INCREMENTAR, DECREMENTAR,

  ASTERISCO, AMPERSAND, ARROBA,

  // Punteros y Direcciones
  PUNTERO, DIRECCION, SWAP,

  // Comparadores
  MAYOR, MENOR, MAYOR_IGUAL, MENOR_IGUAL, IGUAL_CMP, DISTINTO, FLOAT_EQUAL,

  // Lógica
  Y_LOGICO, O_LOGICO, NO_LOGICO, XO_LOGICO,

  // Bitwise
  Y_BITWISE, O_BITWISE, NO_BITWISE, XO_BITWISE,
  BITWISE_L, BITWISE_R,

  // Funciones
  FUNC, PURE, MATH, RETURN, CEDER, FLECHA,

  // Asignación
  IGUAL_ASIG,

  MAS_IGUAL, MENOS_IGUAL, POR_IGUAL, DIV_IGUAL, POTENCIA_IGUAL, RAIZ_IGUAL, MOD_IGUAL,

  Y_BIT_IGUAL, O_BIT_IGUAL, XO_BIT_IGUAL,
  Y_LOG_IGUAL, O_LOG_IGUAL, XO_LOG_IGUAL,

  BITWISE_L_IGUAL, BITWISE_R_IGUAL,

  ASIG_BLOQUE,

  // Símbolos comunes
  PUNTO, COMA, PUNTO_COMA, DOS_PUNTOS, PREGUNTA, DOS_PREGUNTAS,

  // Delimitadores
  LLAVE_L, LLAVE_R,
  PAREN_L, PAREN_R,
  CORCH_L, CORCH_R,

  // Arcanos
  ARCANE, ARCANITO,
  CODE, EXPR, KEY, RULES,

  // Escritura
  ESCRITURA,

  // Otros
  FIN_ARCHIVO, ERROR

};

/* --- Tipos --- */

inline int obtenerRangoNum(TypeKind t) { //... Distinguir entre tamaño de bits
  switch (t) {
    case TypeKind::INTEGER: { return 1; }
    case TypeKind::FLOAT  : { return 2; }
    case TypeKind::BOOLEAN: { return 3; }
    default               : { return 0; }
  }
}

inline bool esNum  (TypeKind t) { return obtenerRangoNum(t) > 0; }
inline bool esFloat(TypeKind t) { return t == TypeKind::FLOAT  ; }

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

inline std::shared_ptr<ArcanaType> promoverTipos(std::shared_ptr<ArcanaType> izq, std::shared_ptr<ArcanaType> der) {
  int tipoIzq = obtenerRangoNum(izq->kind);
  int tipoDer = obtenerRangoNum(der->kind);
  if (tipoDer >= tipoIzq) { return der; }
  return izq;

}

template<typename... Args>
inline std::shared_ptr<ArcanaType> promoverN(std::shared_ptr<ArcanaType> prim, Args... resto) {
  if constexpr (sizeof...(resto) == 0) {
    return prim;

  } else {
    return promoverTipos(prim, promoverN(resto...));

  }

}

template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };


// --- Tipo Operador ---
enum class TipoOperador {

  LOGICO_O,
  LOGICO_XO,
  LOGICO_Y,
  LOGICO_NO,

  BITWISE_O,
  BITWISE_XO,
  BITWISE_Y,
  BITWISE_NO,
  BITWISE_SHIFT_L,
  BITWISE_SHIFT_R,

  CMP_IGUAL,
  CMP_DISTINTO,
  CMP_MENOR,
  CMP_MAYOR,
  CMP_MENOR_IGUAL,
  CMP_MAYOR_IGUAL,

  A_SUMA,
  A_RESTA,
  A_MULT,
  A_DIV,
  A_MOD,
  A_POT,
  A_RAIZ,

  INC_PREF,
  DEC_PREF,

  INC_SUF,
  DEC_SUF,

  PTR_REF,
  PTR_DEREF,

  DESCONOCIDO,

};

inline std::string operadorString(TipoOperador op) { //... Agregar los demás casos
  switch (op) {
    case TipoOperador::A_SUMA : { return "+"      ; }
    case TipoOperador::A_RESTA: { return "-"      ; }
    case TipoOperador::A_MULT : { return "*"      ; }
    case TipoOperador::A_DIV  : { return "/"      ; }
    case TipoOperador::A_POT  : { return "**"     ; }
    default                   : { return "unknown"; }
  }
}

inline TipoOperador convertirEnTipoOperador(Tt op) { //... Agregar los demás casos
  switch (op) {
    case Tt::MAS      : { return TipoOperador::A_SUMA     ; }
    case Tt::MENOS    : { return TipoOperador::A_RESTA    ; }
    case Tt::ASTERISCO: { return TipoOperador::A_MULT     ; }
    case Tt::DIV      : { return TipoOperador::A_DIV      ; }
    case Tt::POTENCIA : { return TipoOperador::A_POT      ; }

    case Tt::MENOR    : { return TipoOperador::CMP_MENOR  ; }

    default           : { return TipoOperador::DESCONOCIDO; }

  }
}

/* --- Lexer --- */
struct Token {
  Tt tipo;
  std::string lexema;
  int linea;
};


// --- Keywords ---
inline std::map<std::string, Tt> keywords = {

  // Tipos Inferibles
  {"var", Tt::VAR}, // auto

  // Tipos explícitos
  {"void", Tt::VOID}, // void
  {"short", Tt::SHORT}, // int16
  {"int", Tt::INT}, // int32
  {"raw", Tt::UINT}, // uint32

  {"float", Tt::FLOAT}, // float32
  {"double", Tt::DOUBLE}, // float64

  {"bool", Tt::BOOL}, // bool

  {"char", Tt::CHAR}, // char
  {"runa", Tt::CHAR}, // char

  {"pergamino", Tt::STRING},

  {"tomo", Tt::VECTOR}, // Array
  {"pacto", Tt::MAP}, // Map
  {"acervo", Tt::SET}, // Set

  {"umbral", Tt::UMBRAL},  // Slice

  {"enum", Tt::ENUM}, // Enums
  {"shape", Tt::SHAPE}, // Variants

  // Modificadores de Tipos

  {"unsigned", Tt::UNSIGNED},
  {"const", Tt::CONST},
  {"exo", Tt::LONG},
  {"magno", Tt::VERY_LONG},
  {"magna", Tt::VERY_LONG},
  {"ilustre", Tt::FULL_LONG},
  {"quid", Tt::COMPLEJO},

  // --- Estructuras ---
  // Arcanos
  {"arcane"  , Tt::ARCANE  },
  {"arcanito", Tt::ARCANITO},
  {"rules"   , Tt::RULES   },

  {"code", Tt::CODE},
  {"expr", Tt::EXPR},
  {"key" , Tt::KEY },

  // If-else
  {"if"  , Tt::IF  },
  {"else", Tt::ELSE},

  // While y do-while
  {"while", Tt::WHILE},
  {"do"   , Tt::DO   },

  // For y foreach
  {"for" , Tt::FOR    },
  {"each", Tt::FOREACH},

  // Break, continue and pass
  {"break"   , Tt::BREAK   },
  {"continue", Tt::CONTINUE},
  {"pass"    , Tt::PASS},

  // Functions
  {"func", Tt::FUNC},
  {"pure", Tt::PURE},
  {"math", Tt::MATH},
  {"return", Tt::RETURN},

};

struct DatosPesados;

using ValorVar = std::variant <
  std::monostate, // Desconocido
  char,           // bool y char
  int16_t,        // short
  int32_t,        // int
  int64_t,        // long
  float,          // float
  double,         // double
  std::string,    // string
  std::unique_ptr<DatosPesados>
>;

struct DatosPesados {
  std::vector<ValorVar> elementos_array;
  std::vector<uint64_t> valor_pesado;
};

/* --- Arcanos --- */

struct InfoVariable {
  Dt tipo;
  bool es_const = false;

  llvm::AllocaInst* alloca = nullptr;

};

enum class TPA { NULO, KEY, EXPR, CODE };

struct ReglaArcano {
  std::string keyword; // trigger keyword
  std::vector<Token> componentes;

};

struct ParteArcano {
  std::string contenido;
  TPA tipo_dato;
};

// Auxiliares para manejar tipos
std::string nombreTipo(Tt tipo);

bool esModificador(Tt);
bool esInfiere(Tt);
bool esTipoComp(Tt);
bool esTipo(Tt);


// Declaraciones previas
class ExprNumero;
class ExprVariable;
class ExprArray;

class ExprUnaria;
class ExprBinaria;
class ExprCasteo;

class ExprRango;
class ExprAcceso;

class ExprFuncCall;

class Bloque;

class SentenciaAsignarVar;
class SentenciaExpr;
class SentenciaReasignacionVar;
class SentenciaSi;
class SentenciaSino;
class SentenciaMientras;

class SentenciaBreak;
class SentenciaContinue;

class SentenciaReturn;
class SentenciaFuncDecl;

class SentenciaEscritura;
class SentenciaArcano;
class SentenciaLlamadaArcano;


class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;

  // Expresiones
  virtual void visitar(ExprNumero* nodo)   = 0;
  virtual void visitar(ExprVariable* nodo) = 0;
  virtual void visitar(ExprArray* nodo)    = 0;

  virtual void visitar(ExprUnaria* nodo)  = 0;
  virtual void visitar(ExprBinaria* nodo) = 0;
  virtual void visitar(ExprCasteo* nodo)  = 0;

  virtual void visitar(ExprRango* nodo)  = 0;
  virtual void visitar(ExprAcceso* nodo) = 0;

  virtual void visitar(ExprFuncCall* nodo) = 0;

  // Sentencias
  virtual void visitar(Bloque* nodo)        = 0;

  virtual void visitar(SentenciaAsignarVar* nodo)  = 0;
  virtual void visitar(SentenciaExpr* nodo) = 0;

  virtual void visitar(SentenciaReasignacionVar* nodo) = 0;

  virtual void visitar(SentenciaSi* nodo)   = 0;
  virtual void visitar(SentenciaSino* nodo) = 0;

  virtual void visitar(SentenciaMientras* nodo) = 0;

  virtual void visitar(SentenciaBreak   * nodo) = 0;
  virtual void visitar(SentenciaContinue* nodo) = 0;

  virtual void visitar(SentenciaReturn  * nodo) = 0;
  virtual void visitar(SentenciaFuncDecl* nodo) = 0;

  virtual void visitar(SentenciaEscritura* nodo) = 0;

  virtual void visitar(SentenciaArcano* nodo)        = 0;
  virtual void visitar(SentenciaLlamadaArcano* nodo) = 0;

};

/* --- AST --- */
class NodoAST {
public:

  int linea = 0;

  virtual ~NodoAST() = default;
  virtual void imprimir(int nivel = 0) const = 0;
  virtual void accept(ASTVisitor* visitor) = 0;

};

template <typename Base, typename Derived>
class NodoBase : public Base {
public:

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(static_cast<Derived*>(this));
  }

  std::unique_ptr<Base> clonar() const override {
    return std::make_unique<Derived>(static_cast<const Derived&>(*this));
  }

};

// Subclases principales
class Expresion : public NodoAST {
public:
  Dt tipo_resuelto;

  virtual std::unique_ptr<Expresion> clonar() const = 0;

  virtual bool isLValue() const { return false; }

};

class Sentencia : public NodoAST {
public:
  virtual std::unique_ptr<Sentencia> clonar() const = 0;

};

// --- Manejo de Arcanos --- //

struct ArcaneSegment {
  std::string                                       br_key ;
  std::vector<std::pair<std::string, InfoVariable>> br_args; // Variables
  std::vector<std::string>                          br_expr; // Expressions
  std::unique_ptr<Sentencia>                        br_cont;

  ArcaneSegment() = default;
  ArcaneSegment(ArcaneSegment&&) = default;
  ArcaneSegment& operator=(ArcaneSegment&&) = default;

  ArcaneSegment(const ArcaneSegment& otra)
    : br_key(otra.br_key), br_args(otra.br_args), br_expr(otra.br_expr) {
    if (otra.br_cont) {
      br_cont = otra.br_cont->clonar();
    }
  }

  ArcaneSegment& operator=(const ArcaneSegment& otra) {
    if (this != &otra) {
      br_key = otra.br_key;
      br_args = otra.br_args;

      if (otra.br_cont) {
        br_cont = otra.br_cont->clonar();
      } else {
        br_cont = nullptr;
      }
    }
    return *this;
  }

};

struct ArcaneBranch {
  std::string rule_tag; // @rule
  std::vector<ArcaneSegment> segmentos;

  //...

};

struct ArcaneDef {
  std::string name; // CustomIf

  std::vector<ParteArcano>  args    ; // Arcane args
  std::vector<ReglaArcano>  rules   ; // @rule1, @rule2, ... , @ruleN
  std::vector<ArcaneBranch> branches;

  ArcaneDef() = default;
  ArcaneDef(ArcaneDef&&) = default;
  ArcaneDef& operator=(ArcaneDef&&) = default;

  ArcaneDef(const ArcaneDef& otra)
    : name(otra.name), args(otra.args), rules(otra.rules), branches(otra.branches) {}

  ArcaneDef& operator=(const ArcaneDef& otra) {

    if (this != &otra) {
      name = otra.name;
      args = otra.args;
      rules = otra.rules;
      branches = otra.branches;

    }

    return *this;

  }

};

class ContextoArcanos { //...
private:
  std::unordered_map<std::string, ArcaneDef  > activos;
  std::unordered_map<std::string, ReglaArcano> reglas;
  std::unordered_map<std::string, std::unique_ptr<Sentencia>> ramas;
  std::unordered_map<std::string, std::string> keywordArcano;

public:

  bool esKeywordArcano(const std::string& key) const {
    return keywordArcano.find(key) != keywordArcano.end();
  }

  void registrarDefinicion(const std::string& nombre, ArcaneDef def) {
    for (const auto& arg : def.args) {
      if (arg.tipo_dato == TPA::KEY) {
        keywordArcano[arg.contenido] = nombre;
      }
    }
    activos[nombre] = std::move(def);
  }

  ArcaneDef& buscarDefinicionPorNombre(const std::string& name) {
    if (activos.find(name) == activos.end()) {
      throw std::runtime_error("Error interno: No se encontró la definición del Arcano: " + name);
    }

    return activos.at(name);
  }

  ArcaneDef& buscarDefinicionPorKeyword(const std::string& key) {
    if (keywordArcano.find(key) == keywordArcano.end()) {
      throw
        std::runtime_error("Error interno: Se intentó buscar la definición de una keyword inexistente: " + key);
    }
    return activos.at(keywordArcano.at(key));
  }

  void registrarRegla(const std::string& nombre, const ReglaArcano& regla) {
    reglas[nombre] = regla;
  }

  void registrarRama(const std::string& nombre, const std::unique_ptr<Sentencia>& rama) {
    ramas[nombre] = rama->clonar();
  }

  bool existeRegla(const std::string& nombre) const {
    std::cout << "[622, Common.hpp] Rules:\n";
    for (const auto& r : reglas) {
      std::cout << r.first << '\n';
    }
    return reglas.count(nombre) > 0;
  }

  ReglaArcano obtenerRegla(const std::string& nombre) {
    return reglas.at(nombre);
  }

};

// Bloque
class Bloque : public NodoBase<Sentencia, Bloque> {
public:
  std::vector<std::unique_ptr<Sentencia>> instrucciones;

  Bloque() = default;

  Bloque(const Bloque& otra) {
    for (const auto& inst : otra.instrucciones) {
      instrucciones.push_back(inst->clonar());
    }
  }

  void agregarSentencia(std::unique_ptr<Sentencia> sent) {
    instrucciones.push_back(std::move(sent));
  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "{ (Bloque)\n";
    for (const auto& sent : instrucciones) {
      sent->imprimir(nivel + 1);
    }
    std::cout << "| " << sangria << "}\n";
  }

};

// - Nodos -

// Expresiones
class ExprNumero : public NodoBase<Expresion, ExprNumero> {
public:
  std::string valor;
  std::string sufijo;

  ExprNumero(std::string val, std::string suf)
    : valor(val), sufijo(suf) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- " << valor << sufijo << " [" << tipo_resuelto.tipoString() << "]\n";
  }

};

class ExprVariable : public NodoBase<Expresion, ExprVariable> {
public:
  std::string nombre;

  ExprVariable(std::string nom)
    : nombre(nom) {}

  bool isLValue() const override { return true; }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- " << nombre << " [";

    if (tipo_resuelto.valor != nullptr) {
      std::cout << tipo_resuelto.tipoString();

    } else {
      std::cout << "unknown";

    }
    std::cout << "]\n";
  }

};

class ExprArray : public NodoBase<Expresion, ExprArray> {
public:
  std::vector<std::unique_ptr<Expresion>> elementos;

  ExprArray(std::vector<std::unique_ptr<Expresion>> elem)
    : elementos(std::move(elem)) {}

  ExprArray(const ExprArray& otra) {
    elementos.reserve(otra.elementos.size());

    for (const auto& e : otra.elementos) {
      elementos.push_back(e->clonar());
    }

    this->tipo_resuelto = otra.tipo_resuelto;
    this->linea         = otra.linea        ;

  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Array\n";
    for (const auto& e : elementos) {
      e->imprimir(nivel + 1);
    }
  }

};

class ExprUnaria : public NodoBase<Expresion, ExprUnaria> {
public:
  TipoOperador operador;
  std::unique_ptr<Expresion> operando;
  bool es_prefijo;

  ExprUnaria(TipoOperador op, std::unique_ptr<Expresion> arg, bool pref)
    : operador(op), operando(std::move(arg)), es_prefijo(pref) {}

  ExprUnaria(const ExprUnaria& otra)
    : operador(otra.operador), operando(otra.operando->clonar()), es_prefijo(otra.es_prefijo) {

    this->tipo_resuelto = otra.tipo_resuelto;
    this->linea         = otra.linea        ;

  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << (es_prefijo ? "Prefijo" : "Sufijo") << " [" << operadorString(operador) << "]:\n";
    operando->imprimir(nivel + 1);
  }

};

class ExprBinaria : public NodoBase<Expresion, ExprBinaria> {
public:
  TipoOperador operador;
  std::unique_ptr<Expresion> izquierda;
  std::unique_ptr<Expresion> derecha;

  ExprBinaria(TipoOperador op, std::unique_ptr<Expresion> izq, std::unique_ptr<Expresion> der)
    : operador(op), izquierda(std::move(izq)), derecha(std::move(der)) {}

  ExprBinaria(const ExprBinaria& otra)
    : operador (otra.operador),
      izquierda(otra.izquierda->clonar()),
      derecha  (otra.derecha  ->clonar()) {

    this->tipo_resuelto = otra.tipo_resuelto;
    this->linea         = otra.linea        ;

  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Op (" << operadorString(operador) << ") [" << tipo_resuelto.tipoString() << "]\n";
    izquierda->imprimir(nivel + 1);
    derecha->imprimir(nivel + 1);
  }

};

class ExprCasteo : public NodoBase<Expresion, ExprCasteo> {
public:
  std::unique_ptr<Expresion> expresion;
  Dt tipo_casteo;
  bool es_implicito = false;

  ExprCasteo(std::unique_ptr<Expresion> e, Dt t_c)
    : expresion(std::move(e)), tipo_casteo(t_c) {}

  ExprCasteo(const ExprCasteo& otra)
    : expresion(otra.expresion->clonar()), tipo_casteo(otra.tipo_casteo) {

    this->tipo_resuelto = otra.tipo_resuelto;
    this->linea         = otra.linea        ;

  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Cast (impl = " << es_implicito << ") [" << tipo_casteo.tipoString() << "]\n";
    expresion->imprimir(nivel + 1);
  }

};

class ExprRango : public NodoBase<Expresion, ExprRango> {
  public:
    std::unique_ptr<Expresion> inicio;
    std::unique_ptr<Expresion> fin;
    std::unique_ptr<Expresion> paso;

    ExprRango(std::unique_ptr<Expresion> i,
              std::unique_ptr<Expresion> f,
              std::unique_ptr<Expresion> p
              )
      : inicio(std::move(i)), fin(std::move(f)), paso(std::move(p)) {}

    ExprRango(const ExprRango& otra)
      : inicio(otra.inicio ? otra.inicio->clonar() : nullptr),
        fin   (otra.fin    ? otra.fin   ->clonar() : nullptr),
        paso  (otra.paso   ? otra.paso  ->clonar() : nullptr) {

      this->tipo_resuelto = otra.tipo_resuelto;
      this->linea         = otra.linea        ;

    }

    void imprimir(int nivel = 0) const override {
      std::string sangria = "";
      for (int i = 0; i < nivel; ++i) { sangria += "| "; }
      std::cout << sangria << "+- Op (Slice)\n";

      if   (inicio) { inicio->imprimir(nivel + 1); }
      else          { std::cout << sangria << "| +-\n"  ; }

      if   (fin   ) { fin   ->imprimir(nivel + 1); }
      else          { std::cout << sangria << "| +-\n"  ; }

      if   (paso  ) { paso  ->imprimir(nivel + 1); }
      else          { std::cout << sangria << "| +-\n"  ; }

    }

};

class ExprAcceso : public NodoBase<Expresion, ExprAcceso> {
public:
  std::unique_ptr<Expresion> contenedor;
  std::unique_ptr<Expresion> rango;

  ExprAcceso(std::unique_ptr<Expresion> cont, std::unique_ptr<Expresion> idxs)
    : contenedor(std::move(cont)), rango(std::move(idxs)) {}

  ExprAcceso(const ExprAcceso& otra)
    : contenedor(otra.contenedor->clonar()), rango(otra.rango->clonar()) {

      this->tipo_resuelto = otra.tipo_resuelto;
      this->linea         = otra.linea        ;

  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Op (Index)\n";
    contenedor->imprimir(nivel + 1);
    rango->imprimir(nivel + 1);
  }

};

class ExprFuncCall : public NodoBase<Expresion, ExprFuncCall> {
public:
  std::unique_ptr<Expresion> callee;
  std::vector<std::pair<std::string, std::unique_ptr<Expresion>>> argumentos;

  ExprFuncCall(std::unique_ptr<Expresion> c,
               std::vector<std::pair<std::string, std::unique_ptr<Expresion>>> a)
  : callee(std::move(c)), argumentos(std::move(a)) {}

  ExprFuncCall(const ExprFuncCall& otra)
    : callee(otra.callee->clonar()) {

    for (const auto& par : otra.argumentos) {
      argumentos.push_back({par.first, par.second->clonar()});
    }

    this->tipo_resuelto = otra.tipo_resuelto;
    this->linea         = otra.linea        ;

  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Function Call\n";
    callee->imprimir(nivel + 1);

    std::cout << sangria << "| +- Args:\n";
    for (const auto& [arg_name, arg_value] : argumentos) {
      arg_value->imprimir(nivel + 1);
    }
  }

};

// Sentencias
class SentenciaAsignarVar : public NodoBase<Sentencia, SentenciaAsignarVar> {
public:
  std::string nombre;
  InfoVariable tipo_explicito;
  std::unique_ptr<Expresion> valor_inicial;

  SentenciaAsignarVar(std::string nom, InfoVariable tipo, std::unique_ptr<Expresion> val)
    : nombre(nom), tipo_explicito(tipo), valor_inicial(std::move(val)) {}

  SentenciaAsignarVar(const SentenciaAsignarVar& otra)
    : nombre(otra.nombre), tipo_explicito(otra.tipo_explicito),
      valor_inicial(otra.valor_inicial ? otra.valor_inicial->clonar() : nullptr) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }

    std::cout << sangria << "Asignar Variable:\n";
    std::cout << sangria << "| +- " << nombre << " [" << tipo_explicito.tipo.tipoString() << "]\n";

    if (valor_inicial) {
      valor_inicial->imprimir(nivel + 1);

    } else {
      std::cout << sangria << "| +- [Sin inicializar]\n";

    }
  }

};

class SentenciaExpr : public NodoBase<Sentencia, SentenciaExpr> {
public:
  std::unique_ptr<Expresion> expresion;

  SentenciaExpr(std::unique_ptr<Expresion> expr)
    : expresion(std::move(expr)) {}

  SentenciaExpr(const SentenciaExpr& otra)
    : expresion(otra.expresion->clonar()) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Expresión:\n";
    expresion->imprimir(nivel + 1);
  }

};

class SentenciaReasignacionVar : public NodoBase<Sentencia, SentenciaReasignacionVar> {
public:
  std::unique_ptr<Expresion> izquierda;
  std::unique_ptr<Expresion> derecha  ;

  SentenciaReasignacionVar(std::unique_ptr<Expresion> izq, std::unique_ptr<Expresion> val)
    : izquierda(std::move(izq)), derecha(std::move(val)) {}

  SentenciaReasignacionVar(const SentenciaReasignacionVar& otra)
    : izquierda(otra.izquierda->clonar()), derecha(otra.derecha->clonar()) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Reasignación:\n";
    izquierda->imprimir(nivel + 1);
    derecha  ->imprimir(nivel + 1);
  }

};

class SentenciaSi : public NodoBase<Sentencia, SentenciaSi> {
public:
  std::unique_ptr<Expresion> condicion;
  std::unique_ptr<Sentencia> rama_si;
  std::unique_ptr<Sentencia> rama_sino;

  SentenciaSi(std::unique_ptr<Expresion> cond,
              std::unique_ptr<Sentencia> si,
              std::unique_ptr<Sentencia> no)
    : condicion(std::move(cond)), rama_si(std::move(si)), rama_sino(std::move(no)) {}

  SentenciaSi(const SentenciaSi& otra)
    : condicion (otra.condicion->clonar()),
      rama_si   (otra.rama_si  ->clonar()),
      rama_sino (otra.rama_sino ? otra.rama_sino->clonar() : nullptr) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Si\n";
    std::cout << sangria << "| +- Condición:\n";
    condicion->imprimir(nivel + 1);

    std::cout << sangria << "| +- Entonces:\n";
    rama_si->imprimir(nivel + 1);

    if (rama_sino) { rama_sino->imprimir(nivel); }
  }

};

class SentenciaSino : public NodoBase<Sentencia, SentenciaSino> {
public:
  std::unique_ptr<Sentencia> cuerpo;

  SentenciaSino(std::unique_ptr<Sentencia> c)
    : cuerpo(std::move(c)) {}

  SentenciaSino(const SentenciaSino& otra)
    : cuerpo(otra.cuerpo->clonar()) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Sino\n";
    cuerpo->imprimir(nivel + 1);
  }

};

class SentenciaMientras : public NodoBase<Sentencia, SentenciaMientras> {
public:
  std::unique_ptr<Expresion> condicion;
  std::unique_ptr<Sentencia> rama_while;
  std::unique_ptr<Sentencia> rama_sino;

  SentenciaMientras(
      std::unique_ptr<Expresion> cond,
      std::unique_ptr<Sentencia> r_while,
      std::unique_ptr<Sentencia> r_sino
  ) : condicion(std::move(cond)), rama_while(std::move(r_while)), rama_sino(std::move(r_sino)) {}

  SentenciaMientras(const SentenciaMientras& otra)
    : condicion  (otra.condicion ->clonar()),
      rama_while (otra.rama_while->clonar()),
      rama_sino  (otra.rama_sino ? otra.rama_sino->clonar() : nullptr) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Mientras\n";

    std::cout << sangria << "| +- Condición:\n";
    condicion->imprimir(nivel + 2);

    std::cout << sangria << "| +- Cuerpo:\n";
    rama_while->imprimir(nivel + 2);

    if (rama_sino) {
      std::cout << sangria << "| +- Sino:\n";
      rama_sino->imprimir(nivel + 2);

    }
  }

};

class SentenciaBreak : public NodoBase<Sentencia, SentenciaBreak> {
public:
  int linea;

  SentenciaBreak(int l)
    : linea(l) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Break\n";
  }

};

class SentenciaContinue : public NodoBase<Sentencia, SentenciaContinue> {
public:
  int linea;

  SentenciaContinue(int l)
    : linea(l) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Continue\n";
  }

};

class SentenciaReturn : public NodoBase<Sentencia, SentenciaReturn> {
public:
  Dt ret_type;
  std::unique_ptr<Expresion> ret_value;

  SentenciaReturn(Dt r, std::unique_ptr<Expresion> v)
    : ret_type(r), ret_value(std::move(v)) {}

  SentenciaReturn(const SentenciaReturn& otra)
    : ret_type(otra.ret_type),
      ret_value(otra.ret_value ? otra.ret_value->clonar() : nullptr) {
  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Return [" << ret_type.tipoString() << "]\n";
    ret_value->imprimir(nivel + 1);

  }

};

class SentenciaFuncDecl : public NodoBase<Sentencia, SentenciaFuncDecl> {
public:
  std::string nombre_func;
  bool es_pure;
  std::vector<std::pair<std::string, InfoVariable>> args_type;
  std::vector<std::unique_ptr<Sentencia>> cuerpo_func;
  Dt ret_type;

  std::string firma_mangled;

  SentenciaFuncDecl(std::string n,
                    bool pure,
                    std::vector<std::pair<std::string, InfoVariable>> a,
                    std::vector<std::unique_ptr<Sentencia>> c,
                    Dt r)
  : nombre_func(n), es_pure(pure), args_type(a), cuerpo_func(std::move(c)), ret_type(r) {}

  SentenciaFuncDecl(const SentenciaFuncDecl& otra)
    : nombre_func(otra.nombre_func),
      es_pure(otra.es_pure),
      args_type(otra.args_type),
      ret_type(otra.ret_type),
      firma_mangled(otra.firma_mangled) {

    for (const auto& inst: otra.cuerpo_func) {
      cuerpo_func.push_back(inst->clonar());
    }
  
  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Function Declaration\n";
    std::cout << sangria << "| +- "
      << nombre_func
      << " (pure = "
      << es_pure << ") = (|";

    for (const auto [n, i] : args_type) {
      std::cout << " " << i.tipo.valor->toString() << " " << n << " |";
    }
    std::cout << ") -> " << ret_type.valor->toString() << '\n';

    std::cout << sangria << "| ";

    if (!cuerpo_func.empty()) {
      for (const auto& inst : cuerpo_func) {
        inst->imprimir(nivel + 1);
      }

    } else {
      std::cout << sangria << "+- [prototype]\n";

    }

  }

};

class SentenciaEscritura : public NodoBase<Sentencia, SentenciaEscritura> {
public:
  std::string alias;
  Tt original;

  SentenciaEscritura(std::string a, Tt o)
    : alias(std::move(a)), original(o) {}

  SentenciaEscritura(const SentenciaEscritura& otra)
    : alias(otra.alias), original(otra.original) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Escritura\n";
    std::cout << sangria << "| +- " << alias << "\n";
  }

};

class SentenciaArcano : public NodoBase<Sentencia, SentenciaArcano> {

public:
  ArcaneDef def;

  //... Arcanitos

  SentenciaArcano(ArcaneDef d)
    : def(std::move(d)) {}

  SentenciaArcano(const SentenciaArcano& otra) {

    def.name  = otra.def.name ;
    def.args  = otra.def.args ;
    def.rules = otra.def.rules;

    def.branches.reserve(otra.def.branches.size());

    def.branches = otra.def.branches;

  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }

    std::cout << sangria << "+- Arcane: " << def.name << "\n";
 
    std::cout << sangria << "| [ Args ]\n";
    for (const auto& arg : def.args) {
        std::cout << sangria << "| +- ";
        std::string t_str = (arg.tipo_dato == TPA::CODE ? "code" :
                             arg.tipo_dato == TPA::EXPR ? "expr" :
                             arg.tipo_dato == TPA::KEY  ? "key"  : "unknown");
        std::cout << arg.contenido << " <" << t_str << ">";
        std::cout << "\n";
    }

    std::cout << sangria << "| [ Logic ]\n";
    for (const auto& branch : def.branches) {
      std::cout << sangria << "| +- Rule: " << branch.rule_tag << '\n';

      for (const auto& seg : branch.segmentos) {
        std::cout << sangria << "| | +- Keyword: " << seg.br_key << " [";

        for (const auto& info : seg.br_args) {
          std::cout << info.first << ": " << info.second.tipo.tipoString() << ", ";

        }

        std::cout << "]\n";

        seg.br_cont->imprimir(nivel + 1);

      }

    }

  }

};

class SentenciaLlamadaArcano : public NodoBase<Sentencia, SentenciaLlamadaArcano> {
public:
  std::string nombre;
  std::unordered_map<std::string, std::unique_ptr<Sentencia>> args;
  std::unordered_map<std::string, std::unique_ptr<Sentencia>> code;
  std::unordered_map<std::string, std::unique_ptr<Sentencia>> expr;
  size_t indice_rama;

  SentenciaLlamadaArcano(std::string n,
                         std::unordered_map<std::string, std::unique_ptr<Sentencia>> a,
                         std::unordered_map<std::string, std::unique_ptr<Sentencia>> c,
                         std::unordered_map<std::string, std::unique_ptr<Sentencia>> e,
                         size_t idx)
    : nombre(std::move(n)), args(std::move(a)), code(std::move(c)), expr(std::move(e)), indice_rama(idx) {}

  SentenciaLlamadaArcano(const SentenciaLlamadaArcano& otra) //... Todo: Clone args
    : nombre(otra.nombre), indice_rama(otra.indice_rama) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Llamada a Arcano: " << nombre << "\n";
  }

};

/* --- Configuración --- */
struct CompilerConfig {
  std::vector<std::string> flags;

  std::optional<std::filesystem::path> archivo_entrada;
  std::optional<std::filesystem::path> archivo_salida;

  bool ayuda; // flag '-help' usada
  bool mute_decorado;
  bool mute_warnings;
  bool warnings_as_errors;

};


/* --- Errores y Warnings --- */
enum class CodigoError { // Codigo Error

  // --- Errores Léxicos (1000) ---
  ERR_CARACTER_ILEGAL = 1000,
  ERR_NUMERO_MAL_FORMADO = 1001,

  ERR_NO_CERRADO_CADENA = 1100,
  ERR_NO_CERRADO_COMENTARIO = 1101,

  // --- Errores Sintácticos (2000) ---
  ERR_ESPERABA = 2000, // Se esperaba un token que no está
  ERR_NO_ESPERABA = 2001, // No se esperaba un token que sí está

  ERR_DESBALANCE_PARENTESIS = 2100,
  ERR_DESBALANCE_LLAVES = 2101,
  ERR_DESBALANCE_CORCHETES = 2102,

  ERR_EXPRESION_INVALIDA = 2200, // Ej: 2 +-+ 3. 3**


  // --- Erores Semánticos (3000) ---

  // Tipos (3000)
  ERR_TIPO_ESPERADO = 3000, // Se entrega un tipo T1 cuando se esperaba un tipo T2
  ERR_CASTEO_INVALIDO = 3001, // Se intenta hacer un casteo incompatible (ej: (int)string)

  // Variables (3100)
  ERR_VARIABLE_USO_SIN_DECLARAR = 3100, // Cuando se manipula una variable sin declarar
  ERR_VARIABLE_USO_SIN_DEFINIR = 3101, // Se usa una variable sin valor
  ERR_VARIABLE_REDECLARADA = 3102,
  ERR_CONSTANTE_MUTADA = 3103,

  // Funciones (3200)
  ERR_USO_SIN_DEFINIR_F = 3200, // Cuando se llama a una función cuyo nombre y firma no coincide
  ERR_CANTIDAD_ARGUMENTOS_INCORRECTA = 3201,
  ERR_RETORNO_INVALIDO = 3202,
  ERR_FALTA_RETORNO = 3203,

  // Extra (3900)
  ERR_NOMBRE_INVALIDO = 3900,

  // --- Warnings (>= 4000) ---
  W_VARIABLE_SIN_USAR = 4000,
  W_FUNCION_SIN_USAR = 4001,
  W_CODIGO_INACCESIBLE = 4002,
  W_CONVERSION_PELIGROSA = 4003, // Pérdida de precisión

};

using CE = CodigoError;

struct Error {
  std::vector<std::string> detalle;
  CE codigoError;
  int linea;
};

class ErrorHandler {
private:
  std::vector<Error> errores;

public:
  ErrorHandler(std::vector<Error> e)
    : errores(e) {}

  void reportar(CE codigoError, int linea, std::vector<std::string> detalle) {
    Error err;
    err.codigoError = codigoError;
    err.linea = linea;
    err.detalle = std::move(detalle);
    errores.push_back(err);
  }

  bool notificar() {
    bool hayError = !errores.empty();
    for (const auto& e : errores) {
      std::cout << "Error línea " << e.linea << '\n';
      //... Mostrar errores por pantalla
    }

    return hayError;
  }
};


/* --- Symbol Table Manager --- */

struct FirmaMetodo {
  Dt tipo_retorno;
  std::vector<Dt> tipos_param;
};

struct Clase {
  std::string nombre;
  std::unordered_map<std::string, FirmaMetodo> metodos;
  std::unordered_map<std::string, Dt> campos;
};

struct InfoFuncion {
  std::string nombre;
  Dt tipo_retorno;
  std::vector<std::pair<std::string, InfoVariable>> tipos_parametros;
  int linea;
};

struct Scope {
  std::unordered_map<std::string, InfoVariable> variables;
  std::unordered_map<std::string, InfoFuncion > funciones;

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
  //ErrorHandler& errHandler;

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

};


/* --- Extra --- */
// A power of 2 is just 10..00
// A (power of 2) - 1 i just 01..11
// This & that == 0
inline bool isPowerOf2(int num) { return (num > 0) && ((num & (num - 1)) == 0); }


/* --- Colors --- */
extern const std::string COLOR_RESET  ;
extern const std::string COLOR_RED    ;
extern const std::string COLOR_GREEN  ;
extern const std::string COLOR_YELLOW ;
extern const std::string COLOR_BLUE   ;
extern const std::string COLOR_MAGENTA;
extern const std::string COLOR_CYAN   ;
