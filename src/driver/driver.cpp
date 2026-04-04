// driver.cpp

#include "Driver.hpp"

#include "Lexer.hpp"
#include "Parser.hpp"
#include "Checker.hpp"
#include "Emitter.hpp"
#include "Common.hpp"
#include "Includes.hpp"

bool Driver::compile(const CompilerConfig& config) {

  std::filesystem::path file_path = config.archivo_entrada.value();

  // 1. Read source file
  auto source = read_source_file(file_path);
  if (!source) {
    std::cerr << "Error: No se pudo abrir el archivo " << file_path << '\n';
    return false;
  }

  // 2. Set up the error handler
  std::vector<Error> errores;
  ErrorHandler errHandler(errores);

  // 3. Lexical Analysis (Code -> Tokens)
  Lexer lexer(*source);
  std::vector<Token> tokens = lexer.tokenize();

  // 4. Syntactic Analysis (Tokens -> AST)
  TypeFactory factory;
  Parser parser(tokens, factory);
  std::vector<std::unique_ptr<Sentencia>> ast = std::move(parser.parsearPrograma());

  // 5. Semántic Analysis (AST Check)
  std::vector<Scope> scopes{};
  GestorTablas tablas(errHandler, scopes);

  Checker checker(tablas, ast, errHandler, factory);
  checker.verificarPrograma();

  if (errHandler.notificar()) { //... Hay al menos un error
    std::cerr << "[54 driver.cpp]: Error\n";
    return false;
  }

  // 6. Generación de Código (AST -> Source)
  Emitter emitter;
  for (auto& nodo : ast) {
    nodo->accept(&emitter);
  }

  std::string codigo = emitter.obtenerCodigo();

  //... Debug
  std::cout << "\n --- TOKENS --- \n\n";
  for (const auto& t : tokens) {
    std::cout << "< Token: '" << t.lexema << "' | "
              << "L: " << t.linea
              << " >\n";
  }

  std::cout << "\n --- ARBOL DE SINTAXIS ABSTRACTA (AST) ---\n\n";
  for (const auto& nodo : ast) {
    if (nodo) { nodo->imprimir(); }
    else      { std::cout << "[Nodo Nulo]\n"; }
    std::cout << "---------------------------\n";
  }

  std::ofstream outFile("salida.cpp"); //... Change this to accept the actual output file from the user
  if (outFile.is_open()) {
    outFile << codigo;
    outFile.close();
  } else {
    std::cerr << "Error: No se pudo escribir en el archivo de salida.\n";
    return false;
  }

  return true;
}

std::optional<std::string> Driver::read_source_file(const std::filesystem::path& path) const {

  // 1. Verificaciones de Seguridad
  std::error_code ec;
  if (!std::filesystem::exists(path, ec)) {
    std::cerr << "Error: La ruta '" << path << "' no existe.\n";
    return std::nullopt;
  }
  if (!std::filesystem::is_regular_file(path, ec)) {
    std::cerr << "Error: '" << path
              << "' no es un archivo regular (es un directorio o un dispositivo).\n";
    return std::nullopt;
  }

  // 2. Intentar abrir el archivo
  std::ifstream file(path, std::ios::in | std::ios::binary);

  if (!file.is_open()) {
    std::cerr << "Error: No se tienen permisos de lectura para '" << path << "'\n.";
    return std::nullopt;
  }

  // 3. Lectura
  try {
    auto size = std::filesystem::file_size(path);
    if (size == 0) { return ""; } // Si está vacío, devolvemos un string vacío pero válido

    std::string buffer;
    buffer.resize(size); // Pre-asignamos la memoria

    if (file.read(&buffer[0], size)) {
      return buffer;
    }

  } catch (const std::filesystem::filesystem_error& e) {
    std::cerr << "Error de sistema al acceder al archivo: " << e.what() << '\n';

  } catch (const std::bad_alloc& e) {
    std::cerr << "Error: El archivo es demasiado grande para la memoria disponible.\n";

  }

  return std::nullopt;
}
