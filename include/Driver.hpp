 //... Esto lleva a modificar a la def del arcano, cre //... Esto lleva a modificar a la def del arcano, creoo// Driver.hpp

#pragma once

#include "Common.hpp"

#include <string>
#include <filesystem>
#include <optional>

class Driver {
public:
  Driver() = default;
  bool compile(const CompilerConfig& config);

private:
  std::optional<std::string> read_source_file(const std::filesystem::path& path) const;
};
