// main.cpp

#include "Includes.hpp"
#include "Common.hpp"
#include "Driver.hpp"

#include <chrono>

/*
 * Style Guide:
 *
 * Folders: snake_case/
 * Source : camelCase.cpp
 * Headers: PascalCase.hpp
 *
 * Classes: PascalCase
 * Arguments (Classes, private): camelCase
 * Arguments (Classes, public ): snake_case
 *
 * Structs: PascalCase
 * Struct's properties: snake_case
 * Struct's methods   : camelCase
 *
 * Functions : camelCase
 * Methods   : camelCase
 *
 * Arguments (Functions): camelCase
 *
 * Enums          : PascalCase
 * Enums' elements: SCREAMING_SNAKE_CASE
 *
 * Variables: snake_case
 *
 * //... Its a comment to self. "Check this part later"
 *
 */

CompilerConfig parseArguments(int argc, const char *argv[]) {
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

    } else { // No minus sign, we assume it is the input file
      config.archivo_entrada = arg;

    }

  }

  return config;

}

int main(int argc, const char *argv[]) { //... We are still translating the code to english

  //... Debug. Just measuring speed
  auto start = std::chrono::high_resolution_clock::now();

  CompilerConfig config = parseArguments(argc, argv);

  std::filesystem::path input_file;
  std::filesystem::path output_file;

  if (config.archivo_entrada.has_value()) {
    if (std::filesystem::exists(*config.archivo_entrada)) {
      input_file = *config.archivo_entrada;

    } else {
      std::cerr << "Error: La ruta '" << *config.archivo_entrada << "' no existe.\n";

    }
  }

  if (config.archivo_salida.has_value()) {
    output_file = *config.archivo_salida;

  } else {
    std::cerr << "Error: Se esperaba un valor para el archivo de salida.\n";

  }

  Driver driver;
  bool result = driver.compile(config);

  //... Debug
  auto end = std::chrono::high_resolution_clock::now();
  std::chrono::duration<double, std::milli> time = end - start;

  std::cout << '\n' << "Time: " << time.count() << " ms.\n";

  if (result) {
    return 0;

  } else {
    return 1;

  }

}
