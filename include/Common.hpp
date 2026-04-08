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

  LLAMADA       // func() .

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
  SHORT, INT,
  FLOAT, DOUBLE,
  STRING,
  VECTOR, MAP, SET,

  SLICE,

  UMBRAL,

  ENUM,

  // Modificadores
  UNSIGNED, LONG, VERY_LONG, FULL_LONG, COMPLEJO,

  // Variables y Literales
  IDENTIFICADOR, NUMERO,

  // If-else
  SI, SINO,

  // Loops
  HACE, MIENTRAS, PARA, CADA, LOOP,

  SALIR, SEGUIR, // break, continue

  // Operadores
  MAS, MENOS, DIV, POTENCIA, RAIZ, MODULO, // La multiplicación es ASTERISCO

  INCREMENTAR, DECREMENTAR,

  // Punteros y Direcciones
  PUNTERO, DIRECCION, SWAP,

  // Comparadores
  MAYOR, MENOR, MAYOR_IGUAL, MENOR_IGUAL, IGUAL_CMP, DISTINTO,

  // Lógica
  Y_LOGICO, O_LOGICO, NO_LOGICO, XO_LOGICO,

  // Bitwise
  Y_BITWISE, O_BITWISE, NO_BITWISE, XO_BITWISE,
  BITWISE_L, BITWISE_R,

  // Funciones
  FUNC, PURA, RETORNAR, CEDER, FLECHA, REQ, OP,

  ASTERISCO, AMPERSAND,

  // Asignación
  IGUAL_ASIG,

  MAS_IGUAL, MENOS_IGUAL, POR_IGUAL, DIV_IGUAL, POTENCIA_IGUAL, RAIZ_IGUAL, MOD_IGUAL,

  Y_BIT_IGUAL, O_BIT_IGUAL, NO_BIT_IGUAL, XO_BIT_IGUAL,
  Y_LOG_IGUAL, O_LOG_IGUAL, NO_LOG_IGUAL, XO_LOG_IGUAL,

  BITWISE_L_IGUAL, BITWISE_R_IGUAL,

  ASIG_BLOQUE,

  // Símbolos comunes
  PUNTO, COMA, PUNTO_COMA, DOS_PUNTOS, PREGUNTA, DOS_PREGUNTAS,

  // Delimitadores
  LLAVE_L, LLAVE_R,
  PAREN_L, PAREN_R,
  CORCH_L, CORCH_R,

  ARCANO, ARCANITO,
  COD, EXPR, KEY, REGLAS,
  ESCRITURA,

  // Otros
  FIN_ARCHIVO, ERROR
};

/* --- Tipos --- */

inline int obtenerRangoNum(TypeKind t) { //... Distinguir entre tamaño de bits
  switch (t) {
    case TypeKind::INTEGER: { return 1; }
    case TypeKind::FLOAT  : { return 2; }
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
    case Tt::POTENCIA : { return TipoOperador::A_POT      ; }
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

  {"float", Tt::FLOAT}, // float32
  {"double", Tt::DOUBLE}, // float64

  {"bool", Tt::BOOL}, // bool

  {"char", Tt::CHAR},
  {"runa", Tt::CHAR},

  {"pergamino", Tt::STRING},

  {"tomo", Tt::VECTOR}, // Array
  {"pacto", Tt::MAP}, // Map
  {"acervo", Tt::SET}, // Set

  {"umbral", Tt::UMBRAL},  // Slice

  {"enum", Tt::ENUM}, // Enums

  // Modificadores de Tipos
  {"eterno", Tt::CONST}, // const
  {"exo", Tt::LONG},
  {"magno", Tt::VERY_LONG},
  {"magna", Tt::VERY_LONG},
  {"ilustre", Tt::FULL_LONG},
  {"quid", Tt::COMPLEJO},

  // --- Estructuras ---
  // Arcanos
  {"arcano", Tt::ARCANO},
  {"arcanito", Tt::ARCANITO},
  {"reglas", Tt::REGLAS},

  {"code", Tt::COD},
  {"expr", Tt::EXPR},
  {"key", Tt::KEY},

  // If-else
  {"si", Tt::SI},
  {"sino", Tt::SINO},

  // While y do-while
  {"trama", Tt::MIENTRAS},
  {"obra", Tt::HACE},

  // For y foreach
  {"late", Tt::PARA},
  {"vela", Tt::CADA},

  // Break y continue
  {"salir", Tt::SALIR},
  {"seguir", Tt::SEGUIR},

};

/* --- Arcanos --- */
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

enum class TPA { NULO, CODE, EXPR, KEY };

struct ParteArcano {
  std::string contenido;
  TPA tipo_dato;
};

struct DefinicionArcano {
  std::string keyword;
  std::vector<ParteArcano> esqueleto;
};

// Auxiliares para manejar tipos
std::string nombreTipo(Tt tipo);

bool esModificador(Tt);
bool esInfiere(Tt);
bool esTipoComp(Tt);
bool esTipo(Tt);

struct InfoVariable {
  Dt tipo;
  bool es_const = false;
};

// Declaraciones previas
class ExprNumero;
class ExprVariable;
class ExprArray;

class ExprUnaria;
class ExprBinaria;
class ExprCasteo;

class ExprRango;
class ExprAcceso;
class ExprLlamadaArcano;

class Bloque;

class SentenciaVar;
class SentenciaExpr;
class SentenciaAsignacion;
class SentenciaSi;
class SentenciaSino;
class SentenciaMientras;

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

  virtual void visitar(ExprLlamadaArcano* nodo) = 0;

  // Sentencias
  virtual void visitar(Bloque* nodo)        = 0;

  virtual void visitar(SentenciaVar* nodo)  = 0;
  virtual void visitar(SentenciaExpr* nodo) = 0;

  virtual void visitar(SentenciaAsignacion* nodo) = 0;

  virtual void visitar(SentenciaSi* nodo)   = 0;
  virtual void visitar(SentenciaSino* nodo) = 0;

  virtual void visitar(SentenciaMientras* nodo) = 0;

  virtual void visitar(SentenciaEscritura* nodo) = 0;

  virtual void visitar(SentenciaArcano* nodo)        = 0;
  virtual void visitar(SentenciaLlamadaArcano* nodo) = 0;
};

/* --- AST --- */
class NodoAST {
public:
  virtual ~NodoAST() = default;
  virtual void imprimir(int nivel = 0) const = 0;
  virtual void accept(ASTVisitor* visitor) = 0;
};

// Subclases principales
class Expresion : public NodoAST {
public:
  Dt tipo_resuelto;
  int linea;

  virtual void accept(ASTVisitor* visitor) = 0;
};

class Sentencia : public NodoAST {};

// Bloque
class Bloque : public Sentencia {
public:
  std::vector<std::unique_ptr<Sentencia>> instrucciones;

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
    std::cout << sangria << "}\n";
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

// - Nodos -

// Expresiones
class ExprNumero : public Expresion {
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

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class ExprVariable : public Expresion {
public:
  std::string nombre;

  ExprVariable(std::string nom)
    : nombre(nom) {}

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

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class ExprArray : public Expresion {
public:
  std::vector<std::unique_ptr<Expresion>> elementos;

  ExprArray(std::vector<std::unique_ptr<Expresion>> elem)
    : elementos(std::move(elem)) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Array\n";
    for (const auto& e : elementos) {
      e->imprimir(nivel + 1);
    }
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }

};

class ExprUnaria : public Expresion {
public:
  TipoOperador operador;
  std::unique_ptr<Expresion> operando;
  bool es_prefijo;

  ExprUnaria(TipoOperador op, std::unique_ptr<Expresion> arg, bool pref)
    : operador(op), operando(std::move(arg)), es_prefijo(pref) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << (es_prefijo ? "Prefijo" : "Sufijo") << " [" << operadorString(operador) << "]:\n";
    operando->imprimir(nivel + 1);
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class ExprBinaria : public Expresion {
public:
  TipoOperador operador;
  std::unique_ptr<Expresion> izquierda;
  std::unique_ptr<Expresion> derecha;

  ExprBinaria(TipoOperador op, std::unique_ptr<Expresion> izq, std::unique_ptr<Expresion> der)
    : operador(op), izquierda(std::move(izq)), derecha(std::move(der)) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Op (" << operadorString(operador) << ") [" << tipo_resuelto.tipoString() << "]\n";
    izquierda->imprimir(nivel + 1);
    derecha->imprimir(nivel + 1);
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class ExprCasteo : public Expresion {
public:
  std::unique_ptr<Expresion> expresion;
  Dt tipo_casteo;

  ExprCasteo(std::unique_ptr<Expresion> e, Dt t_c)
    : expresion(std::move(e)), tipo_casteo(t_c) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Cast (" << tipo_casteo.tipoString() << ")\n";
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }

};

class ExprRango : public Expresion {
  public:
    std::unique_ptr<Expresion> inicio;
    std::unique_ptr<Expresion> fin;
    std::unique_ptr<Expresion> paso;

    ExprRango(std::unique_ptr<Expresion> i,
              std::unique_ptr<Expresion> f,
              std::unique_ptr<Expresion> p
              )
    : inicio(std::move(i)), fin(std::move(f)), paso(std::move(p)) {}

    void imprimir(int nivel = 0) const override {
      std::string sangria = "";
      for (int i = 0; i < nivel; ++i) { sangria += "| "; }
      std::cout << sangria << "+- Op (Slice)\n";

      if (inicio) {
        inicio->imprimir(nivel + 1);

      } else {
        std::cout << sangria << "| +-\n";

      }

      if (fin   ) {
        fin->imprimir(nivel + 1);

      } else {
        std::cout << sangria << "| +-\n";

      }

      if (paso  ) {
        paso->imprimir(nivel + 1);

      } else {

        std::cout << sangria << "| +-\n";

      }

    }

    void accept(ASTVisitor* visitor) override {
      visitor->visitar(this);
    }
};

class ExprAcceso : public Expresion {
public:
  std::unique_ptr<Expresion> contenedor;
  std::unique_ptr<Expresion> rango;

  ExprAcceso(std::unique_ptr<Expresion> cont, std::unique_ptr<Expresion> idxs)
    : contenedor(std::move(cont)), rango(std::move(idxs)) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Op (Index)\n";
    contenedor->imprimir(nivel + 1);
    rango->imprimir(nivel + 1);
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class ExprLlamadaArcano : public Expresion {
public:
  std::string nombre;
  std::vector<std::unique_ptr<Expresion>> args_expr;
  std::vector<std::unique_ptr<Bloque>> args_bloque;

  ExprLlamadaArcano(std::string n) : nombre(std::move(n)) {}
};

// Sentencias
class SentenciaVar : public Sentencia {
public:
  std::string nombre;
  InfoVariable tipo_explicito;
  std::unique_ptr<Expresion> valor_inicial;

  SentenciaVar(std::string nom, InfoVariable tipo, std::unique_ptr<Expresion> val)
    : nombre(nom), tipo_explicito(tipo), valor_inicial(std::move(val)) {}

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

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class SentenciaExpr : public Sentencia {
public:
  std::unique_ptr<Expresion> expresion;
  SentenciaExpr(std::unique_ptr<Expresion> expr)
    : expresion(std::move(expr)) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Expresión:\n";
    expresion->imprimir(nivel + 1);
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class SentenciaAsignacion : public Sentencia {
public:
  std::unique_ptr<Expresion> izquierda;
  std::unique_ptr<Expresion> derecha;

  SentenciaAsignacion(std::unique_ptr<Expresion> izq, std::unique_ptr<Expresion> val)
    : izquierda(std::move(izq)), derecha(std::move(val)) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "Reasignación:\n";
    izquierda->imprimir(nivel + 1);
    derecha->imprimir(nivel + 1);
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class SentenciaSi : public Sentencia {
public:
  std::unique_ptr<Expresion> condicion;
  std::unique_ptr<Sentencia> rama_si;
  std::unique_ptr<Sentencia> rama_sino;

  SentenciaSi(std::unique_ptr<Expresion> cond,
              std::unique_ptr<Sentencia> si)
    : condicion(std::move(cond)), rama_si(std::move(si)), rama_sino(nullptr) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Si\n";
    std::cout << sangria << "| +- Condición:\n";
    condicion->imprimir(nivel + 1);
    std::cout << sangria << "| +- Entonces:\n";
    rama_si->imprimir(nivel + 1);
    if (rama_sino) {
      std::cout << sangria << "| +- Sino:\n";
      rama_sino->imprimir(nivel + 1);
    }
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class SentenciaSino : public Sentencia {
public:
  std::unique_ptr<Sentencia> cuerpo;

  SentenciaSino(std::unique_ptr<Sentencia> c)
    : cuerpo(std::move(c)) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Sino\n";
    cuerpo->imprimir(nivel + 2);
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class SentenciaMientras : public Sentencia {
public:
  std::unique_ptr<Expresion> condicion;
  std::unique_ptr<Sentencia> rama_while;
  std::unique_ptr<Sentencia> rama_sino;
  SentenciaMientras(
      std::unique_ptr<Expresion> cond,
      std::unique_ptr<Sentencia> r_while,
      std::unique_ptr<Sentencia> r_sino
  ) : condicion(std::move(cond)), rama_while(std::move(r_while)), rama_sino(std::move(r_sino)) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Mientras\n";
    std::cout << sangria << "| +- Condición:\n";
    condicion->imprimir(nivel + 1);
    std::cout << sangria << "| +- Cuerpo:\n";
    rama_while->imprimir(nivel + 1);
    if (rama_sino) {
      std::cout << sangria << "| +- Sino:\n";
      rama_sino->imprimir(nivel + 1);
    }
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class SentenciaEscritura : public Sentencia {
public:
  std::string alias;
  Tt original;

  SentenciaEscritura(std::string a, Tt o)
    : alias(std::move(a)), original(o) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Escritura\n";
    std::cout << sangria << "| +- " << alias << "\n";
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class SentenciaArcano : public Sentencia {
public:
  std::string nombre;
  DefinicionArcano esqueleto;
  std::map<std::string, std::unique_ptr<Sentencia>> ramas; // Keywords contextuales

  //... Reglas
  //... Arcanitos

  SentenciaArcano(std::string n, DefinicionArcano e,
                  std::map<std::string, std::unique_ptr<Sentencia>> r)
    : nombre(std::move(n)), esqueleto(std::move(e)), ramas(std::move(r)) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }

    std::cout << sangria << "+- Arcano: " << nombre << "\n";
 
    // --- Sección 1: Interfaz / Esqueleto ---
    std::cout << sangria << "| [ Interfaz ]\n";
    for (const auto& parte : esqueleto.esqueleto) {
        std::cout << sangria << "|   +- ";
        std::string t_str = (parte.tipo_dato == TPA::CODE ? "code" :
                             parte.tipo_dato == TPA::EXPR ? "expr" :
                             parte.tipo_dato == TPA::KEY  ? "key"  : "unknown");
        std::cout << parte.contenido << " <" << t_str << ">";
        std::cout << "\n";
    }
    std::cout << "\n";

    // --- Sección 2: Cuerpo / Ramas ---
    std::cout << sangria << "| [ Implementación ]\n";
    for (const auto& par : ramas) {
        std::cout << sangria << "|   +- Rama contextual: '" << par.first << "'\n";
        if (par.second) {
            par.second->imprimir(nivel + 1);
        }
    }
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

class SentenciaLlamadaArcano : public Sentencia {
public:
  std::string nombre;
  std::map<std::string, std::unique_ptr<Sentencia>> argumentos;

  SentenciaLlamadaArcano(std::string n, std::map<std::string, std::unique_ptr<Sentencia>> args)
    : nombre(std::move(n)), argumentos(std::move(args)) {}

  void imprimir(int nivel = 0) const override {
    std::string sangria = "";
    for (int i = 0; i < nivel; ++i) { sangria += "| "; }
    std::cout << sangria << "+- Llamada a Arcano: " << nombre << "\n";
    for (const auto& par : argumentos) {
      std::cout << sangria << "| +- Param: " << par.first << "\n";
      if (par.second) {
        par.second->imprimir(nivel + 1);
      } else {
        std::cout << sangria << "|    (vacío/ no provisto)\n";
      }
    }
  }

  void accept(ASTVisitor* visitor) override {
    visitor->visitar(this);
  }
};

struct Regla {

  std::vector<Token> componentes;
  uint8_t propiedades;
  /*
   * Propiedades:
   *
   * 0b00 = 0 o más veces
   * 0b01 = 1 o más veces
   * 0b10 = 0 o 1 veces
   * 0b11 = Exactamente 1 vez
   *
   * */

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

struct Scope {
  std::unordered_map<std::string, InfoVariable> variables = std::unordered_map<std::string, InfoVariable>();
};


class GestorTablas {
private:
  ErrorHandler& errHandler;
  std::vector<Scope> scopes = std::vector<Scope>();

public:
  GestorTablas(ErrorHandler& err, std::vector<Scope> scopes);

  void entrarBloque(Scope scope);
  void salirBloque();

  // --- Variables ---
  bool añadirVariable(const std::string& nombre, InfoVariable info, int linea);
  InfoVariable* buscarVariable(const std::string& nombre, int linea);
};


/* --- Extra --- */
inline bool isPowerOf2(int num) { return (num > 0) && ((num & (num - 1)) == 0); }


/* --- Colors --- */
extern const std::string COLOR_RESET  ;
extern const std::string COLOR_RED    ;
extern const std::string COLOR_GREEN  ;
extern const std::string COLOR_YELLOW ;
extern const std::string COLOR_BLUE   ;
extern const std::string COLOR_MAGENTA;
extern const std::string COLOR_CYAN   ;
