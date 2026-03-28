// main.cpp

#include "Includes.hpp"
#include "Common.hpp"
#include "Driver.hpp"

#include <iostream>

CompilerConfig parsearArgumentos(int argc, const char *argv[]) {
  CompilerConfig config;

  std::vector<std::string> args(argv + 1, argv + argc);

  for (size_t i = 0; i < args.size(); ++i) {
    const std::string arg = args[i];

    std::cout << "Arg " << i << ": " << arg << " Size: " << args.size() << '\n';

    if (arg == "-help") {
      config.ayuda = true;

    } else if (arg == "-o") {
      if (i + 1 < args.size()) {
        config.archivoSalida = args[++i];

      } else {
        std::cerr << "Error: Se esperaba un archivo de salida después de '-o'.\n";
        exit(1);
      }

    } else if (arg == "-shh") {
      config.muteDecorado = true;

    } else if (arg == "-w") {
      config.muteWarnings = true;

    } else if (arg.starts_with("-")) {
      std::cerr << "Error: Flag desconocida '" << arg << "'.\n";
      exit(1);

    } else { // No tiene signo menos, asumimos que es el archivo de entrada
      config.archivoEntrada = arg;

    }

  }

  return config;
}

int main(int argc, const char *argv[]) {

  CompilerConfig config = parsearArgumentos(argc, argv);

  std::filesystem::path archivoEntrada;
  std::filesystem::path archivoSalida;

  if (config.archivoEntrada.has_value()) {
    if (std::filesystem::exists(*config.archivoEntrada)) {
      archivoEntrada = *config.archivoEntrada;

    } else {
      std::cerr << "Error: La ruta '" << archivoEntrada << "' no existe.\n";

    }
  }

  if (config.archivoSalida.has_value()) {
    archivoSalida = *config.archivoSalida;

  } else {
    std::cerr << "Error: Se esperaba un valor para el archivo de salida.\n";

  }

  Driver driver;

  if (driver.compile(config)) {
    return 0;

  } else {
    return 1;

  }

}
