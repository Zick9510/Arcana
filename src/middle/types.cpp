// types.cpp

#include "Types.hpp"

/* --- Arcana Types --- */

// --- VoidType ---


// --- IntegerType ---
IntegerType::IntegerType(int b, bool u)
  : ArcanaType(TypeKind::Integer), bits(b), is_unsigned(u) {}

std::string IntegerType::toString() const {
  return (is_unsigned? "u" : "i") + std::to_string(bits);
}

int IntegerType::getBitSize() const { return bits; }

bool IntegerType::esIgual(const ArcanaType* otro) const {
  if (otro->kind != TypeKind::Integer) { return false; }

  auto o = static_cast<const IntegerType*>(otro);

  return (this->bits        == o->bits       ) &&
         (this->is_unsigned == o->is_unsigned);

}

// --- FloatType ---
FloatType::FloatType(int b)
  : ArcanaType(TypeKind::Float), bits(b) {}

std::string FloatType::toString() const {
  return 'f' + std::to_string(bits);
}

int FloatType::getBitSize() const { return bits; }

bool FloatType::esIgual(const ArcanaType* otro) const {
  const FloatType* otroFloat = dynamic_cast<const FloatType*>(otro);

  if (otroFloat == nullptr) {
    return false;

  }

  return (this->bits == otroFloat->bits);
}

