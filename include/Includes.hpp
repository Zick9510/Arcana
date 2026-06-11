// Includes.hpp

#pragma once

// --- C++ --- //
#include <algorithm>
#include <cctype>
#include <charconv>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <stacktrace>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>

// --- LLVM --- //
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Verifier.h>

#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_ostream.h>

// --- LIBCLANG --- //
#include <clang-c/Index.h>

// IWYU #pragma: export
