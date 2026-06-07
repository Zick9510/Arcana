// Common.hpp

#pragma once

#include "Includes.hpp"

#include "Symbol.hpp"
#include "Types.hpp"
#include "Error.hpp"

// --- Precedencia --- 
enum class Precedencia : int {
  MINIMA = 0,

  ASIGNACION,   // =

  TERNARIO,     // ? :

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

  SWAP,         // ><

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
  LET,

  // Constantes
  CONST,

  // Tipos de datos
  VOID_TYPE,
  BYTE_TYPE, CHAR_TYPE, BOOL_TYPE,
  SHORT_TYPE, INT_TYPE, UINT_TYPE,
  FLOAT_TYPE, DOUBLE_TYPE,

  STRING_TYPE,

  SLICE_TYPE,

  VECTOR_TYPE, MAP_TYPE, SET_TYPE,

  ENUM,

  STRUCT,
  SHAPE, //...?

  // Modificadores
  UNSIGNED, LONG, VERY_LONG, FULL_LONG, COMPLEJO,

  // Variables y Literales
  IDENTIFICADOR, NUMERO, CHAR, STRING,

  TRUE, FALSE,

  // If-else
  IF, ELSE,

  // Loops
  WHILE, FOR, FOREACH,

  BREAK, CONTINUE, REDO, PASS,

  // Operadores
  MAS, MENOS, DIV, POTENCIA, RAIZ, MODULO, // La multiplicación es ASTERISCO

  INCREMENTAR, DECREMENTAR,

  ASTERISCO, AMPERSAND,

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
  PUNTO, COMA, PUNTO_COMA, DOS_PUNTOS,
  PREGUNTA, DOS_PREGUNTAS,
  ARROBA, TATETI,

  // Delimitadores
  LLAVE_L, LLAVE_R,
  CORCH_L, CORCH_R,
  PAREN_L, PAREN_R,

  // Arcanos
  ARCANE, ARCANITE,
  CODE, EXPR, KEY, VAR,
  RULES, CHAINS,

  // Escritura
  ESCRITURA,

  // Templates
  TEMPLATE, TYPE,

  // Otros
  EOF_TT, ERROR

};

inline bool isStructural(Tt t) {
  switch (t) {
    case Tt::EOF_TT:

    case Tt::PAREN_R:
    case Tt::CORCH_R:
    case Tt::LLAVE_R:

    case Tt::PUNTO_COMA: {
      return true;
    }

    default: { return false; }

  }

}

inline bool isKeyword(Tt t) {
  switch (t) {
    case Tt::IF     :
    case Tt::ELSE   :
    case Tt::WHILE  :
    //case Tt::FOR    :
    //case Tt::FOREACH:
    case Tt::RETURN : {
      return true;
    }

    default: { return false; }
  }

}

// --- ParseMode --- //
enum class Pm {
  STRICT,
  RELAXED
};

/* --- Tipos --- */

inline std::string generarFirma(const std::string& nombre, const std::vector<Dt>& tiposArgs) {
  std::string firma = nombre;
  for (const auto& tipo : tiposArgs) {
    firma += "_" + tipo.tipoString();
  }
  return firma;
}

inline int obtenerRangoNum(TypeKind t) { //... Distinguir entre tamaño de bits
  switch (t) {
    case TypeKind::BOOLEAN: { return 1; }
    case TypeKind::CHAR   : { return 2; }
    case TypeKind::INTEGER: { return 3; }
    case TypeKind::FLOAT  : { return 4; }
    default               : { return 0; }
  }
}

inline bool esNum  (TypeKind t) { return obtenerRangoNum(t) > 0; }
inline bool esFloat(TypeKind t) { return t == TypeKind::FLOAT  ; }

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

//template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };

// --- Tipo Operador --- //
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

  CMP_MAYOR,
  CMP_MAYOR_IGUAL,
  CMP_MENOR_IGUAL,
  CMP_MENOR,

  SUMA,
  RESTA,
  MULT,
  DIV,
  MOD,
  POT,
  RAIZ,

  SWAP,
  TERNARY,

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
    case TipoOperador::SUMA   : { return "+"      ; }
    case TipoOperador::RESTA  : { return "-"      ; }
    case TipoOperador::MULT   : { return "*"      ; }
    case TipoOperador::DIV    : { return "/"      ; }
    case TipoOperador::POT    : { return "**"     ; }

    case TipoOperador::SWAP   : { return "><"     ; }
    case TipoOperador::TERNARY: { return "?:"     ; }

    default                   : { return "unknown"; }
  }
}

inline TipoOperador convertirEnTipoOperador(Tt op) { //... Agregar los demás casos
  switch (op) {
    case Tt::MAS      : { return TipoOperador::SUMA ; }
    case Tt::MENOS    : { return TipoOperador::RESTA; }
    case Tt::ASTERISCO: { return TipoOperador::MULT ; }
    case Tt::DIV      : { return TipoOperador::DIV  ; }
    case Tt::POTENCIA : { return TipoOperador::POT  ; }
    case Tt::SWAP     : { return TipoOperador::SWAP ; }

    case Tt::IGUAL_CMP: { return TipoOperador::CMP_IGUAL   ; }
    case Tt::DISTINTO : { return TipoOperador::CMP_DISTINTO; }

    case Tt::MAYOR      : { return TipoOperador::CMP_MAYOR      ; }
    case Tt::MAYOR_IGUAL: { return TipoOperador::CMP_MAYOR_IGUAL; }
    case Tt::MENOR_IGUAL: { return TipoOperador::CMP_MENOR_IGUAL; }
    case Tt::MENOR      : { return TipoOperador::CMP_MENOR      ; }

    default           : { return TipoOperador::DESCONOCIDO; }

  }
}

// --- Traits --- //
enum class BlockTrait {
  LOOP,
  NOSCOPE,
  UNKNOWN
};

using BT = BlockTrait;

static const std::unordered_map<std::string_view, BT> string_traits = {
  {"loop", BT::LOOP},
  {"noscope", BT::NOSCOPE},
};

inline BT stringToTrait(std::string_view name) {
  auto it = string_traits.find(name);
  if (it != string_traits.end()) {
    return it->second;
  }

  return BT::UNKNOWN;

}

inline std::string_view traitToString(BT trait) {
  switch (trait) {
    case BT::LOOP   : { return "loop"   ; }
    case BT::NOSCOPE: { return "noscope"; }
    default         : { return "unknown"; }
  }
}


/* --- Lexer --- */
struct Token {
  Tt tipo;
  std::string lexema;
  Pos pos;
};


// --- Keywords --- //
inline std::map<std::string, Tt> keywords = {

  // Tipos Inferibles
  {"let", Tt::LET},

  // Tipos explícitos
  {"void", Tt::VOID_TYPE}, // void

  {"short", Tt::SHORT_TYPE}, // int16
  {"int"  , Tt::INT_TYPE  }, // int32
  {"uint" , Tt::UINT_TYPE }, // uint32
  {"raw"  , Tt::UINT_TYPE }, // uint32

  {"float" , Tt::FLOAT_TYPE }, // float32
  {"double", Tt::DOUBLE_TYPE}, // float64

  {"bool", Tt::BOOL_TYPE}, // bool

  {"char" , Tt::CHAR_TYPE}, // char
  {"runa", Tt::CHAR_TYPE}, // char

  {"string", Tt::STRING_TYPE},

  {"slice", Tt::SLICE_TYPE},  // Slice

  {"enum", Tt::ENUM}, // Enums
  {"struct", Tt::STRUCT},

  // Modificadores de Tipos

  {"unsigned", Tt::UNSIGNED},
  {"const", Tt::CONST},
  {"exo", Tt::LONG},
  {"magno", Tt::VERY_LONG},
  {"magna", Tt::VERY_LONG},
  {"ilustre", Tt::FULL_LONG},
  {"quid", Tt::COMPLEJO},

  // Booleans
  {"true" , Tt::TRUE},
  {"false", Tt::FALSE},

  // --- Estructuras ---
  // Arcanos
  {"arcane"  , Tt::ARCANE  },
  {"arcanite", Tt::ARCANITE},
  {"rules"   , Tt::RULES   },
  {"chains"  , Tt::CHAINS  },

  {"code", Tt::CODE},
  {"expr", Tt::EXPR},
  {"key" , Tt::KEY },
  {"var" , Tt::VAR},

  // If-else
  {"if"  , Tt::IF  },
  {"else", Tt::ELSE},

  // While y do-while
  {"while", Tt::WHILE},
  //{"do"   , Tt::DO   },

  // For y foreach
  //{"for" , Tt::FOR    },
  //{"each", Tt::FOREACH},

  // Break, continue and pass
  {"break"   , Tt::BREAK   },
  {"continue", Tt::CONTINUE},
  {"pass"    , Tt::PASS    },
  {"redo"    , Tt::REDO    },

  // Functions
  {"func", Tt::FUNC},
  {"pure", Tt::PURE},
  {"math", Tt::MATH},
  {"return", Tt::RETURN},

  // Templates
  {"template", Tt::TEMPLATE},
  {"type", Tt::TYPE},

};

/* --- Arcanos --- */

enum class TPA { NULO, KEY, EXPR, CODE, VAR };

struct ReglaArcano {
  std::string keyword; // trigger keyword
  std::vector<Token> componentes;

};

struct EnlaceCadena {
  std::string target_rule;
  bool optional;
};

struct ParteArcano {
  std::string contenido;
  TPA tipo_dato;
};

// --- Meta Directivas --- //
enum class MetaID {
  CHAIN,
  DESCONOCIDO
};

static const std::unordered_map<std::string, MetaID> meta_string_to_id = {
  {"chain", MetaID::CHAIN},
};

inline MetaID metaStringToID(std::string name) {
  auto it = meta_string_to_id.find(name);
  if (it != meta_string_to_id.end()) {
    return it->second;
  }
  return MetaID::DESCONOCIDO;
}

inline std::string metaIDToString(MetaID id) {
  switch (id) {
    case MetaID::CHAIN: { return "chain"  ; }
    default           : { return "unknown"; }
  }
}

// Auxiliares para manejar tipos
std::string nombreTipo(Tt tipo);

bool esModificador(Tt);
bool esInfiere(Tt);
bool esTipoComp(Tt);
bool esTipo(Tt);


// Declaraciones previas
class ErrorNode;

class ExprLiteral;
class ExprVariable;
class ExprArray;

class ExprUnaria;
class ExprBinaria;
class ExprTernaria;

class ExprCasteo;

class ExprRango;
class ExprAcceso;
class ExprAccesoPunto;

class ExprFuncCall;

class ExprInitList;

class Bloque;

class SentenciaAsignarVar;
class SentenciaExpr;
class SentenciaReasignacionVar;
class SentenciaSi;
class SentenciaSino;
class SentenciaMientras;

class SentenciaBreak;
class SentenciaContinue;
class SentenciaRedo;

class SentenciaReturn;
class SentenciaFuncDecl;

class SentenciaStruct;

class SentenciaEscritura;
class SentenciaArcano;
class SentenciaLlamadaArcano;
class SentenciaMetaDirective;

class SentenciaTemplate;

class ASTVisitor {
public:
  virtual ~ASTVisitor() = default;

  virtual void visitar(ErrorNode* nodo) = 0;

  // Expresiones
  virtual void visitar(ExprLiteral * nodo) = 0;
  virtual void visitar(ExprVariable* nodo) = 0;

  virtual void visitar(ExprArray* nodo)    = 0;

  virtual void visitar(ExprUnaria* nodo)   = 0;
  virtual void visitar(ExprBinaria* nodo)  = 0;
  virtual void visitar(ExprTernaria* nodo) = 0;

  virtual void visitar(ExprCasteo* nodo)  = 0;

  virtual void visitar(ExprRango* nodo)       = 0;
  virtual void visitar(ExprAcceso* nodo)      = 0;
  virtual void visitar(ExprAccesoPunto* nodo) = 0;

  virtual void visitar(ExprFuncCall* nodo) = 0;

  virtual void visitar(ExprInitList* nodo) = 0;

  // Sentencias
  virtual void visitar(Bloque* nodo) = 0;

  virtual void visitar(SentenciaAsignarVar* nodo) = 0;
  virtual void visitar(SentenciaExpr* nodo) = 0;

  virtual void visitar(SentenciaReasignacionVar* nodo) = 0;

  virtual void visitar(SentenciaSi* nodo)   = 0;
  virtual void visitar(SentenciaSino* nodo) = 0;

  virtual void visitar(SentenciaMientras* nodo) = 0;

  virtual void visitar(SentenciaBreak   * nodo) = 0;
  virtual void visitar(SentenciaContinue* nodo) = 0;
  virtual void visitar(SentenciaRedo    * nodo) = 0;

  virtual void visitar(SentenciaReturn  * nodo) = 0;
  virtual void visitar(SentenciaFuncDecl* nodo) = 0;

  virtual void visitar(SentenciaStruct* nodo) = 0;

  virtual void visitar(SentenciaEscritura* nodo) = 0;

  virtual void visitar(SentenciaArcano* nodo)        = 0;
  virtual void visitar(SentenciaLlamadaArcano* nodo) = 0;
  virtual void visitar(SentenciaMetaDirective* nood) = 0;

  virtual void visitar(SentenciaTemplate* nodo) = 0;

};

/* --- AST --- */
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

class ErrorNode : public NodoBase<Expresion, ErrorNode> {
public:

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- ErrorNode\n";

  }

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
  std::unordered_map<std::string, std::vector<EnlaceCadena>> chains; // Maps a parent rule to the sequence of rules that can follow it
  std::vector<ArcaneBranch> branches;

  ArcaneDef() = default;
  ArcaneDef(ArcaneDef&&) = default;
  ArcaneDef& operator=(ArcaneDef&&) = default;

  ArcaneDef(const ArcaneDef& otra)
    : name(otra.name), args(otra.args), rules(otra.rules), chains(otra.chains), branches(otra.branches) {}

  ArcaneDef& operator=(const ArcaneDef& otra) {

    if (this != &otra) {
      name     = otra.name;
      args     = otra.args;
      rules    = otra.rules;
      chains   = otra.chains;
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
  std::vector<BT> traits;

  Bloque() = default;

  Bloque(const Bloque& otra) {
    for (const auto& inst : otra.instrucciones) {
      instrucciones.push_back(inst->clonar());
    }
    traits = otra.traits;
  }

  void agregarSentencia(std::unique_ptr<Sentencia> sent) {
    instrucciones.push_back(std::move(sent));
  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Bloque\n";

    if (!traits.empty()) {
      std::cout << sangria << "| +- Traits:\n";

      for (auto t : traits) {
        std::cout << sangria << "| | +- " << traitToString(t) << '\n';
      }
    }

    std::cout << sangria << "| +- Instructions:\n";
    std::cout << sangria << "| | {\n";
    for (const auto& sent : instrucciones) {
      sent->imprimir(nivel + 2);
    }
    std::cout << sangria << "| | }\n";
  }

};

// - Nodos -

// Expresiones
struct NumberData {
  std::string valor ;
  std::string sufijo;
};

struct BooleanData {
  std::string valor ;
};

struct CharData {
  std::string letra ;
  std::string sufijo;
};

struct StringData {
  std::string contenido;
};

struct RuleData {
  std::string rule;
};

using LiteralData = std::variant<NumberData, BooleanData, CharData, StringData, RuleData>;

class ExprLiteral : public NodoBase<Expresion, ExprLiteral> {
public:
  LiteralData datos;

  ExprLiteral(LiteralData d)
    : datos(std::move(d)) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- ";

    std::visit([](const auto& arg) {
      using T = std::decay_t<decltype(arg)>;

      if      constexpr (std::is_same_v<T, NumberData >) { std::cout <<         arg.valor <<         arg.sufijo; }
      else if constexpr (std::is_same_v<T, CharData   >) { std::cout << '\'' << arg.letra << '\'' << arg.sufijo; }
      else if constexpr (std::is_same_v<T, RuleData   >) { std::cout << '@'  << arg.rule                       ; }
      else if constexpr (std::is_same_v<T, BooleanData>) { std::cout <<         arg.valor                      ; }

    }, datos);

    std::cout << " [" << tipo_resuelto.tipoString() << "]\n";
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
    this->pos           = otra.pos          ;

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
    this->pos           = otra.pos          ;

  }

  bool isLValue() const override { return operador == TipoOperador::PTR_DEREF; }

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
  std::string overload = "";

  ExprBinaria(TipoOperador op, std::unique_ptr<Expresion> izq, std::unique_ptr<Expresion> der)
    : operador(op), izquierda(std::move(izq)), derecha(std::move(der)) {}

  ExprBinaria(const ExprBinaria& otra)
    : operador (otra.operador),
      izquierda(otra.izquierda->clonar()),
      derecha  (otra.derecha  ->clonar()) {

    this->tipo_resuelto = otra.tipo_resuelto;
    this->pos           = otra.pos          ;

  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Op (" << operadorString(operador) << ") [" << tipo_resuelto.tipoString() << "]\n";
    izquierda->imprimir(nivel + 1);
    derecha->imprimir(nivel + 1);
  }

};

class ExprTernaria : public NodoBase<Expresion, ExprTernaria> {
public:
  std::unique_ptr<Expresion> condicion ;
  std::unique_ptr<Expresion> rama_true ;
  std::unique_ptr<Expresion> rama_false;

  ExprTernaria(std::unique_ptr<Expresion> c, std::unique_ptr<Expresion> t, std::unique_ptr<Expresion> f)
    : condicion(std::move(c)), rama_true(std::move(t)), rama_false(std::move(f)) {}

  ExprTernaria(const ExprTernaria& otra)
    : condicion(otra.condicion->clonar()), rama_true(otra.rama_true->clonar()), rama_false(otra.rama_false->clonar()) {

    this->tipo_resuelto = otra.tipo_resuelto;
    this->pos           = otra.pos  ;

  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Op Ternario [" << tipo_resuelto.tipoString() << "]\n";
    std::cout << sangria << "| Condición:\n";
    condicion->imprimir(nivel + 1);
    std::cout << sangria << "| Rama true:\n";
    rama_true->imprimir(nivel + 1);
    std::cout << sangria << "| Rama false:\n";
    rama_false->imprimir(nivel + 1);

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
    this->pos           = otra.pos          ;

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
      this->pos           = otra.pos          ;

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
      this->pos           = otra.pos          ;

  }

  bool isLValue() const override { return true; }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Op (Index)\n";
    contenedor->imprimir(nivel + 1);
    rango->imprimir(nivel + 1);
  }

};

class ExprAccesoPunto : public NodoBase<Expresion, ExprAccesoPunto> {
public:
  std::unique_ptr<Expresion> izquierda;
  std::string propiedad;

  ExprAccesoPunto(std::unique_ptr<Expresion> izq, std::string prop)
    : izquierda(std::move(izq)), propiedad(std::move(prop)) {}

  ExprAccesoPunto(const ExprAccesoPunto& otra)
    : izquierda(otra.izquierda->clonar()), propiedad(otra.propiedad) {}

  bool isLValue() const override { return true; }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }

    std::cout << sangria << "ExprAccesoPunto [" << tipo_resuelto.valor->toString() << "]\n";
    izquierda->imprimir(nivel + 1);
    std::cout << sangria << "| +- " << propiedad << '\n';

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
    this->pos           = otra.pos          ;

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

struct ArgumentoInit {
  std::optional<std::string> name ;
  std::unique_ptr<Expresion> value;

};

class ExprInitList : public NodoBase<Expresion, ExprInitList> {
public:
  std::vector<ArgumentoInit> args;

  ExprInitList(std::vector<ArgumentoInit> a)
    : args(std::move(a)) {}

  ExprInitList(const ExprInitList& otra) {
    for (const auto& a : otra.args) {
      args.push_back(ArgumentoInit{a.name, a.value->clonar()});

    }

  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }

    std::cout << sangria <<  "ExprInitList";

    if (tipo_resuelto.valor != nullptr) {
      std::cout << " [" << tipo_resuelto.valor->toString() << "]";
    }

    std::cout << '\n';
    for (const auto& a : args) {

      std::cout << sangria << "| +- ";

      if (a.name.has_value()) {
        std::cout << a.name.value();

      } else {
        std::cout << "[Anon]";

      }

      std::cout << '\n';

      a.value->imprimir(nivel + 2);

    }

  }

};

// Sentencias
class SentenciaAsignarVar : public NodoBase<Sentencia, SentenciaAsignarVar> {
public:
  std::string nombre;
  InfoVariable tipo_explicito;
  std::unique_ptr<Expresion> valor_inicial;
  std::unique_ptr<Expresion> size;

  SentenciaAsignarVar(std::string nom, InfoVariable tipo, std::unique_ptr<Expresion> val, std::unique_ptr<Expresion> s)
    : nombre(nom), tipo_explicito(tipo), valor_inicial(std::move(val)), size(std::move(s)) {}

  SentenciaAsignarVar(const SentenciaAsignarVar& otra)
    : nombre(otra.nombre), tipo_explicito(otra.tipo_explicito),
      valor_inicial(otra.valor_inicial ? otra.valor_inicial->clonar() : nullptr),
      size(otra.size ? otra.size->clonar() : nullptr) {}

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

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Break\n";
  }

};

class SentenciaContinue : public NodoBase<Sentencia, SentenciaContinue> {
public:

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Continue\n";
  }

};

class SentenciaRedo : public NodoBase<Sentencia, SentenciaRedo> {
public:

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Redo\n";
  }

};

class SentenciaReturn : public NodoBase<Sentencia, SentenciaReturn> {
public:
  Dt ret_type; //... This is redundent
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
                             arg.tipo_dato == TPA::KEY  ? "key"  :
                             arg.tipo_dato == TPA::VAR  ? "var"   : "unknown");

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
  std::string nombre;   // Keyword
  std::string rule_tag; // Rule
  std::unordered_map<std::string, std::unique_ptr<Sentencia>> args;
  std::unordered_map<std::string, std::unique_ptr<Sentencia>> code;
  std::unordered_map<std::string, std::unique_ptr<Sentencia>> expr;
  std::unordered_map<std::string, std::unique_ptr<Expresion>> vars;
  size_t indice_rama;

  std::vector<std::unique_ptr<SentenciaLlamadaArcano>> chains;

  std::vector<std::unique_ptr<Sentencia>> nodos_expandidos;

  SentenciaLlamadaArcano(std::string n, std::string t,
                         std::unordered_map<std::string, std::unique_ptr<Sentencia>> a,
                         std::unordered_map<std::string, std::unique_ptr<Sentencia>> c,
                         std::unordered_map<std::string, std::unique_ptr<Sentencia>> e,
                         std::unordered_map<std::string, std::unique_ptr<Expresion>> v,
                         std::vector<std::unique_ptr<SentenciaLlamadaArcano>> ch      ,
                         size_t idx
                         )
    : nombre(std::move(n)), rule_tag(std::move(t)),
      args(std::move(a)),
      code(std::move(c)),
      expr(std::move(e)),
      vars(std::move(v)),

      indice_rama(idx), chains(std::move(ch)) {}

  SentenciaLlamadaArcano(const SentenciaLlamadaArcano& otra) //... Todo: Clone the rest of the thing
    : nombre(otra.nombre), indice_rama(otra.indice_rama) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Llamada a Arcano: " << nombre << ' ' << rule_tag << '\n';

    if (!args.empty()) {
      std::cout << sangria << "ARGS:\n";
      for (const auto& [i, a] : args) {
        std::cout << sangria << "| " << i << " ->\n";
        a->imprimir(nivel + 2);
      }
    }

    if (!expr.empty()) {
      std::cout << sangria << "EXPR:\n";
      for (const auto& [i, a] : expr) {
        std::cout << sangria << "| " << i << " ->\n";
        a->imprimir(nivel + 2);
      }
    }

    if (!code.empty()) {
      std::cout << sangria << "CODE:\n";
      for (const auto& [i, a] : code) {
        std::cout << sangria << "| " << i << " ->\n";
        a->imprimir(nivel + 2);
      }
    }

    if (!chains.empty()) {
      std::cout << sangria << "CHAINS:\n";
      for (const auto& i : chains) {
        i->imprimir(nivel + 2);
      }
    }

  }

};

class SentenciaMetaDirective : public NodoBase<Sentencia, SentenciaMetaDirective> {
public:
  MetaID id;
  std::vector<std::unique_ptr<Expresion>> args;
  std::unique_ptr<Sentencia> body;

  SentenciaMetaDirective(MetaID i, std::vector<std::unique_ptr<Expresion>> a, std::unique_ptr<Sentencia> b)
    : id(i), args(std::move(a)), body(std::move(b)) {}

  SentenciaMetaDirective(const SentenciaMetaDirective& otra)
    : id(otra.id) {

    for (const auto& arg : otra.args) {
      args.push_back(arg->clonar());
    }

    body = otra.body->clonar();

  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }

    std::cout << sangria << "Meta Directiva: " << metaIDToString(id) << '\n';
    std::cout << sangria << "Args:\n";

    if (!args.empty()) {
      for (const auto& a : args) {
        a->imprimir(nivel + 1);
      }
    }

    std::cout << sangria << "Blocks:\n";
    if (body) {
      body->imprimir(nivel + 1);
    }

  }

};

class SentenciaStruct : public NodoBase<Sentencia, SentenciaStruct> {
public:
  std::string name;

  std::vector<std::unique_ptr<Sentencia>> propiedades;
  std::vector<std::unique_ptr<Sentencia>> metodos    ;

  SentenciaStruct(std::string n, std::vector<std::unique_ptr<Sentencia>> prop, std::vector<std::unique_ptr<Sentencia>> met)
    : name(std::move(n)), propiedades(std::move(prop)), metodos(std::move(met)) {}

  SentenciaStruct(const SentenciaStruct& otra)
    : name(otra.name) {

    for (const auto& prop : otra.propiedades) {
      propiedades.push_back(prop->clonar());
    }

    for (const auto& met : otra.metodos) {
      metodos.push_back(met->clonar());
    }

  }

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Struct Decl: " << name << '\n';

    std::cout << sangria << "| +- Props:\n";
    for (const auto& p : propiedades) {
      p->imprimir(nivel + 1);
    }

    std::cout << sangria << "| +- Methods:\n";
    for (const auto& m : metodos) {
      m->imprimir(nivel + 1);
    }

  }

};

class SentenciaTemplate : public NodoBase<Sentencia, SentenciaTemplate> {
public:

  std::string name;
  std::vector<std::pair<std::string, std::variant<InfoTemplateParam, InfoVariable>>> args;
  std::unique_ptr<Sentencia> statement;

  SentenciaTemplate(std::string n, std::vector<std::pair<std::string, std::variant<InfoTemplateParam, InfoVariable>>> a, std::unique_ptr<Sentencia> s)
    : name(std::move(n)), args(std::move(a)), statement(std::move(s)) {}

  SentenciaTemplate(const SentenciaTemplate& otra)
    : name(otra.name), args(otra.args), statement(otra.statement->clonar()) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Template:\n";
    statement->imprimir(nivel + 1);

  }

};

/* --- Config--- */
struct CompilerConfig {
  std::vector<std::string> flags;

  std::optional<std::filesystem::path> archivo_entrada;
  std::optional<std::filesystem::path> archivo_salida;

  bool ayuda;
  bool mute_decorado;
  bool mute_warnings;
  bool warnings_as_errors;

};


/* --- Buffer --- */
struct ResolvedPos {
  size_t line;
  size_t col ;
};

struct SourceBuffer {
  std::string content;
  std::vector<size_t> line_offsets;

  SourceBuffer(std::string&& source) : content(std::move(source)) {
    line_offsets.reserve(128);
    line_offsets.push_back(0);
  }

  void setOffset(size_t offset) {
    line_offsets.push_back(offset);
  }

  ResolvedPos resolvePos(size_t cursor) const {
    auto it = std::upper_bound(line_offsets.begin(), line_offsets.end(), cursor);

    size_t line_idx = std::distance(line_offsets.begin(), it) - 1;
    size_t line_num = line_idx + 1;
    size_t col_num  = cursor - line_offsets[line_idx] + 1;

    return { line_num, col_num };
  }

  std::string_view getLine(size_t lineNum) const {
    if (lineNum == 0 || lineNum > line_offsets.size()) { return ""; }

    size_t start = line_offsets[lineNum - 1];
    size_t end   = (lineNum < line_offsets.size())
                 ? line_offsets[lineNum] - 1
                 : content.size();

    if (end > start && content[end - 1] == '\r') { end--; }

    return std::string_view(content.data() + start, end - start);
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

/* --- Extra --- */
// A power of 2 is just 10..00
// A (power of 2) - 1 is just 01..11
// This & that == 0
inline bool isPowerOf2(int num) { return (num > 0) && ((num & (num - 1)) == 0); }

namespace dunder {
  const std::string ADD  = "__add__" ;
  const std::string SUB  = "__sub__" ;
  const std::string MULT = "__mult__";

};

/* --- Colors --- */
extern const std::string COLOR_RESET  ;
extern const std::string COLOR_RED    ;
extern const std::string COLOR_GREEN  ;
extern const std::string COLOR_YELLOW ;
extern const std::string COLOR_BLUE   ;
extern const std::string COLOR_MAGENTA;
extern const std::string COLOR_CYAN   ;
