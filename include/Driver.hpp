// Driver.hpp

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
  std::optional<std::string> readSourceFile(const std::filesystem::path& path) const;
};
