// Common.hpp

#pragma once

#include "Includes.hpp"


// --- Precedencia --- 
enum Predencia {
  MINIMA = 0,
  ASIGNACION, // =
  LOGICA_O,   // ||
  LOGICA_XOR, // ^^
  LOGICA_Y,   // &&
  BIT_O,      // |
  BIT_XOR,    // ^
  BIT_Y,      // &
  IGUALDAD,   // == !=
  RELACIONAL, // < > <= >=
  SHIFT,      // << >>
  SUMA,       // + -
  MULT,       // * / %
  POTENCIA,   // ** */
  PREFIJO,    // - ++ -- *Expr &Expr ~Expr
  SUFIJO,     // ++ -- ! (factorial)
  ACCESO,     // Expr[i]
  LLAMADA     // func() .
};

// --- TipoToken ---
enum class Tt {
  // Tipos Inferibles
  VAR, ETERNO,

  // Tipos de datos
  BYTE, RUNA, DUAL,
  WYN, DOX,
  REAL, VASTO,
  PERGAMINO,
  TOMO, SAGA, PACTO, GRIMORIO, ACERVO,

  UMBRAL,

  // Modificadores
  NAT, EXO, MAGNO, ILUSTRE, COMPLEJO,

  // Variables y Literales
  IDENTIFICADOR, NUMERO,

  // If-else
  SI, SINO,

  // Loops
  HACE, MIENTRAS, PARA, CADA, LOOP,

  SALIR, SEGUIR, // break, continue

  // Operadores
  MAS, MENOS, POR, DIV, POTENCIA, RAIZ, MODULO,

  INCREMENTAR, DECREMENTAR,

  FACTORIAL,

  // Punteros y Direcciones
  PUNTERO, DIRECCION, SWAP,

  // Comparadores
  MAYOR, MENOR, MAYOR_IGUAL, MENOR_IGUAL, IGUAL_CMP, DISTINTO,

  // Lógica
  Y_LOGICO, O_LOGICO, NO_LOGICO, XOR_LOGICO,

  // Bitwise
  Y_BITWISE, O_BITWISE, NO_BITWISE, XOR_BITWISE,
  BITWISE_L, BITWISE_R,

  // Funciones
  FUNC, PURA, RETORNAR, CEDER, FLECHA,

  ASTERISCO, AMPERSAND,

  // Asignación
  IGUAL_ASIG, MAS_IGUAL, MENOS_IGUAL, POR_IGUAL, DIV_IGUAL, POTENCIA_IGUAL, RAIZ_IGUAL,
  Y_BIT_IGUAL, O_BIT_IGUAL, NO_BIT_IGUAL, XO_BIT_IGUAL,
  Y_LOG_IGUAL, O_LOG_IGUAL, NO_LOG_IGUAL, XO_LOG_IGUAL,
  BITWISE_L_IGUAL, BITWISE_R_IGUAL,

  ASIG_BLOQUE,

  // Símbolos comunes
  PUNTO, COMA, PUNTO_COMA, DOS_PUNTOS, BARRA,

  // Delimitadores
  LLAVE_L, LLAVE_R,
  PAREN_L, PAREN_R,
  CORCH_L, CORCH_R,

  ARCANO, REQ, OP, COD, EXPR, KEY,
  ESCRITURA,

  // Otros
  FIN_ARCHIVO, ERROR
};

// Token
struct Token {
  Tt tipo;
  std::string lexema;
  int linea;
};


// keywords
inline std::map<std::string, Tt> keywords = {

  // Tipos Inferibles
  {"var", Tt::VAR},
  {"eterno", Tt::ETERNO}, // const

  // Tipos explícitos
  {"wyn", Tt::WYN}, // int16_t
  {"dox", Tt::DOX}, // int

  {"real", Tt::REAL}, // float
  {"vasto", Tt::VASTO}, // double

  {"dual", Tt::DUAL}, // bool

  {"runa", Tt::RUNA},
  {"pergamino", Tt::PERGAMINO},

  {"tomo", Tt::TOMO},             // Array
  {"saga", Tt::SAGA},           // Linked list
  {"pacto", Tt::PACTO},        // Map
  {"grimorio", Tt::GRIMORIO}, // Mapa ordenado
  {"acervo", Tt::ACERVO},    // Set

  {"umbral", Tt::UMBRAL},  // Slice

  // Modificadores de Tipos
  {"exo", Tt::EXO},
  {"magno", Tt::MAGNO},
  {"magna", Tt::MAGNO},
  {"ilustre", Tt::ILUSTRE},
  {"quid", Tt::COMPLEJO},

  // Estructuras
  // Arcano
  {"arcano", Tt::ARCANO},
  {"req", Tt::REQ},
  {"op", Tt::OP},
  {"cod", Tt::COD},
  {"expr", Tt::EXPR},

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

// Arcanos
struct DatosPesados;

using ValorVar = std::variant <
  std::monostate, // Desconocido
  char,          // dual y runa
  int16_t,      // 16 bits
  int32_t,     // 32 bits
  int64_t,    // 64 bits
  float,     // real
  double,   // vasto
  std::string,    // pergamino
  std::unique_ptr<DatosPesados>
>;

struct DatosPesados {
  std::vector<ValorVar> elementos_tomo;
  std::vector<uint64_t> valor_pesado;
};

enum class TipoParte { PARAMETRO, SEPARADOR };
enum class TPA { NULO, COD, EXPR, KEY };

struct ParteArcano {
  TipoParte tipo_parte;
  std::string contenido;
  TPA tipo_dato;
  bool es_opcional;
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

struct InfoTipo {
  Tt base_tipo;
  bool es_nat = false;
  int multiplicador = 1;
  bool es_complejo = false;
  bool es_eterno = false;

  std::vector<InfoTipo> subtipos;
};

// Declaraciones previas
class ExprNumero;
class ExprVariable;
class ExprUnaria;
class ExprBinaria;
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

    virtual void visitar(ExprUnaria* nodo)  = 0;
    virtual void visitar(ExprBinaria* nodo) = 0;
 
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
class Expresion : public NodoAST {};
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

    ExprNumero(std::string val)
      : valor(val) {}

    void imprimir(int nivel = 0) const override {
      std::string sangria = "";
      for (int i = 0; i < nivel; ++i) { sangria += "| "; }
      std::cout << sangria << "+- " << valor << "\n";
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
      std::cout << sangria << "+- " << nombre << "\n";
    }

    void accept(ASTVisitor* visitor) override {
      visitor->visitar(this);
    }
};

class ExprUnaria : public Expresion {
  public:
    std::string operador;
    std::unique_ptr<Expresion> operando;
    bool esPrefijo;

    ExprUnaria(std::string op, std::unique_ptr<Expresion> arg, bool pref)
      : operador(op), operando(std::move(arg)), esPrefijo(pref) {}

    void imprimir(int nivel = 0) const override {
      std::string sangria = "";
      for (int i = 0; i < nivel; ++i) { sangria += "| "; }
      std::cout << sangria << (esPrefijo ? "Prefijo" : "Sufijo") << " [" << operador << "]:\n";
      operando->imprimir(nivel + 1);
    }

    void accept(ASTVisitor* visitor) override {
      visitor->visitar(this);
    }
};

class ExprBinaria : public Expresion {
  public:
    std::string operador;
    std::unique_ptr<Expresion> izquierda;
    std::unique_ptr<Expresion> derecha;

    ExprBinaria(std::string op, std::unique_ptr<Expresion> izq, std::unique_ptr<Expresion> der)
      : operador(op), izquierda(std::move(izq)), derecha(std::move(der)) {}

    void imprimir(int nivel = 0) const override {
      std::string sangria = "";
      for (int i = 0; i < nivel; ++i) { sangria += "| "; }
      std::cout << sangria << "+- Op (" << operador << ")\n";
      izquierda->imprimir(nivel + 1);
      derecha->imprimir(nivel + 1);
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
    bool obtener_valores;

    ExprRango(std::unique_ptr<Expresion> i,
              std::unique_ptr<Expresion> f,
              std::unique_ptr<Expresion> p,
              bool get_val)
      : inicio(std::move(i)), fin(std::move(f)), paso(std::move(p)), obtener_valores(get_val) {}

    void imprimir(int nivel = 0) const override {
      std::string sangria = "";
      for (int i = 0; i < nivel; ++i) { sangria += "| "; }
      std::cout << sangria << "+- Op ([])\n";
      inicio->imprimir(nivel + 1);
      if (fin) {
        fin->imprimir(nivel + 1);
      }
      if (paso) {
        paso->imprimir(nivel + 1);
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
      std::cout << sangria << "+- Op ([])}\n";
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
    std::string tipo_explicito;
    std::unique_ptr<Expresion> valor_inicial;

    SentenciaVar(std::string nom, std::string tipo, std::unique_ptr<Expresion> val)
      : nombre(nom), tipo_explicito(tipo), valor_inicial(std::move(val)) {}

    void imprimir(int nivel = 0) const override {
      std::string sangria = "";
      for (int i = 0; i < nivel; ++i) { sangria += "| "; }
      std::cout << sangria
                << "Declarar Variable: " << nombre << " [Tipo: " << tipo_explicito << "]\n";
      if (valor_inicial) {
        valor_inicial->imprimir(nivel + 1);
      } else {
        std::cout << sangria << "-+ [Sin inicializar]\n";
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
      expresion->imprimir(nivel + 2);
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
      std::cout << sangria << "Asignación:\n";
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
      condicion->imprimir(nivel + 2);
      std::cout << sangria << "| +- Entonces:\n";
      rama_si->imprimir(nivel + 2);
      if (rama_sino) {
        std::cout << sangria << "| +- Sino:\n";
        rama_sino->imprimir(nivel + 2);
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
      condicion->imprimir(nivel + 2);
      std::cout << sangria << "| +- Cuerpo:\n";
      rama_while->imprimir(nivel + 2);
      if (rama_sino) {
        std::cout << sangria << "| +- Sino:\n";
        rama_sino->imprimir(nivel + 2);
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
    std::map<std::string, std::unique_ptr<Sentencia>> ramas; // keywords contextuales

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
          if (parte.tipo_parte == TipoParte::PARAMETRO) {
              std::string t_str = (parte.tipo_dato == TPA::COD ? "cod" :
                                   parte.tipo_dato == TPA::EXPR ? "expr" : "key");
              std::cout << (parte.es_opcional ? "Opcional: " : "Requerido: ");
              std::cout << parte.contenido << " <" << t_str << ">";
          }
         std::cout << "\n";
      }
      std::cout << "\n";

      // --- Sección 2: Cuerpo / Ramas ---
      std::cout << sangria << "| [ Implementación ]\n";
      for (const auto& par : ramas) {
          std::cout << sangria << "|   +- Rama contextual: '" << par.first << "'\n";
          if (par.second) {
              par.second->imprimir(nivel + 2);
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
          par.second->imprimir(nivel + 2);
        } else {
          std::cout << sangria << "|    (vacío/ no provisto)\n";
        }
      }
    }

    void accept(ASTVisitor* visitor) override {
      visitor->visitar(this);
    }
};

struct CompilerConfig {
  std::vector<std::string> flags;

  std::optional<std::filesystem::path> archivoEntrada;
  std::optional<std::filesystem::path> archivoSalida;

  bool ayuda; // flag '-help' usada
  bool muteDecorado;
  bool muteWarnings;
  bool warningsAsErrors;

};

struct InfoVariable {
  bool es_const = false;
};

/* --- Errores y Warnings --- */
enum class CE { // Codigo Error

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
        //... Mostrar errores por pantalla
      }

      return hayError;
    }
};


/* --- Symbol Table Manager --- */
struct Scope {
  std::unordered_map<std::string, InfoVariable> variables;
};


class GestorTablas {
  private:
    ErrorHandler errHandler;
    std::vector<Scope> scopes;
  public:
    GestorTablas(ErrorHandler& err);

    void entrarBloque(Scope scope);
    void salirBloque();

    bool añadirVariable(const std::string& nombre, InfoVariable info, int linea);
    InfoVariable* buscarVariable(const std::string& nombre, int linea);
};
