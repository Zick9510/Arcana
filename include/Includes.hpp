// Includes.hpp

#pragma once

// --- STD --- //
#include <algorithm>
#include <cstdint>
#include <cctype>
#include <charconv>
#include <memory>
#include <iostream>
#include <fstream>
#include <optional>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <map>
#include <filesystem>
#include <stacktrace>

// --- LLVM --- //
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Value.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

// IWYU #pragma: export
