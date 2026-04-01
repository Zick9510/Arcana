// main.cpp

#include "Includes.hpp"
#include "Common.hpp"
#include "Driver.hpp"

#include <chrono>

/*
 * Style Guide:
 *
 * Soruce : camelCase.cpp
 * Haeders: PascalCase.hpp
 *
 * Clases     : PascalCase
 * Estructuras: PascalCase
 *
 * Argumentos (Clases): snake_case
 *
 * Funciones : camelCase
 * Métodos   : camelCase
 *
 * Argumentos (Funciones): camelCase
 *
 * Elementos de enums: SCREAMING_SNAKE_CASE
 *
 * Variables: snake_case
 *
 * //... Means "There is somehting that in this line that should be checked out later" or "This is for debugging"
 *
 * */

CompilerConfig parsearArgumentos(int argc, const char *argv[]) {
  CompilerConfig config;

  std::vector<std::string> args(argv + 1, argv + argc);

  for (size_t i = 0; i < args.size(); ++i) {
    const std::string arg = args[i];

    if (arg == "-help") {
      config.ayuda = true;

    } else if (arg == "-o") {
      if (i + 1 < args.size()) {
        config.archivo_salida = args[++i];

      } else {
        std::cerr << COLOR_RED << "[Error] " << COLOR_RESET << "Se esperaba un archivo de salida después de '-o'.\n";
        exit(1);
      }

    } else if (arg == "-shh") {
      config.mute_decorado = true;

    } else if (arg == "-w") {
      config.mute_warnings = true;

    } else if (arg.starts_with("-")) {
      std::cerr << "Error: Flag desconocida '" << arg << "'.\n";
      exit(1);

    } else { // No tiene signo menos, asumimos que es el archivo de entrada
      config.archivo_entrada = arg;

    }

  }

  return config;
}

int main(int argc, const char *argv[]) { //... I am still translating the code to english

  //... Debug. Just measuring speed
  auto inicio = std::chrono::high_resolution_clock::now();

  CompilerConfig config = parsearArgumentos(argc, argv);

  std::filesystem::path archivo_entrada;
  std::filesystem::path archivo_salida;

  if (config.archivo_entrada.has_value()) {
    if (std::filesystem::exists(*config.archivo_entrada)) {
      archivo_entrada = *config.archivo_entrada;

    } else {
      std::cerr << "Error: La ruta '" << *config.archivo_entrada << "' no existe.\n";

    }
  }

  if (config.archivo_salida.has_value()) {
    archivo_salida = *config.archivo_salida;

  } else {
    std::cerr << "Error: Se esperaba un valor para el archivo de salida.\n";

  }

  Driver driver;

  bool resultado = driver.compile(config);

  //... Debug
  auto fin = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> tiempo = fin - inicio;

  std::cout << "En " << tiempo.count() << " ms.\n";

  if (resultado) {
    return 0;

  } else {
    return 1;

  } 

}
