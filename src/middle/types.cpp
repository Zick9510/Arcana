// types.cpp

#include "Types.hpp"

#include "Includes.hpp"

/* --- Arcana Types --- */

// --- VoidType ---

VoidType::VoidType()
  : ArcanaType(TypeKind::VOID) {}

std::string VoidType::toString() const { return "void"; }

int VoidType::getBitSize()       const { return 0     ; }

bool VoidType::esIgual(const ArcanaType* otro) const {
  return (otro->kind == TypeKind::VOID);
}

// --- IntegerType ---
IntegerType::IntegerType(int b, bool u)
  : ArcanaType(TypeKind::INTEGER), bits(b), is_unsigned(u) {}

std::string IntegerType::toString() const {
  return (is_unsigned? "u" : "i") + std::to_string(bits);
}

int IntegerType::getBitSize() const { return bits; }

bool IntegerType::esIgual(const ArcanaType* otro) const {
  if (otro->kind != TypeKind::INTEGER) { return false; }

  auto o = static_cast<const IntegerType*>(otro);

  return (this->bits        == o->bits       ) &&
         (this->is_unsigned == o->is_unsigned);

}

// --- FloatType ---
FloatType::FloatType(int b)
  : ArcanaType(TypeKind::FLOAT), bits(b) {}

std::string FloatType::toString() const {
  return "f" + std::to_string(bits);
}

int FloatType::getBitSize() const { return bits; }

bool FloatType::esIgual(const ArcanaType* otro) const {
  const FloatType* otroFloat = dynamic_cast<const FloatType*>(otro);

  if (otroFloat == nullptr) {
    return false;

  }

  return (this->bits == otroFloat->bits);
}

/* --- Factory --- */

std::shared_ptr<IntegerType> TypeFactory::getInteger(int bits, bool is_unsigned) {
  auto key = std::make_tuple(bits, is_unsigned);

  if (cacheInteger.find(key) != cacheInteger.end()) {
    return cacheInteger[key];

  }

  auto nueva_instancia = std::make_shared<IntegerType>(bits, is_unsigned);
  cacheInteger[key]    = nueva_instancia;

  return nueva_instancia;
}

std::shared_ptr<FloatType> TypeFactory::getFloat(int bits) {
  if (cacheFloat.find(bits) != cacheFloat.end()) {
    return cacheFloat[bits];

  }

  auto nueva_instancia = std::make_shared<FloatType>(bits);
  cacheFloat[bits] = nueva_instancia;

  return nueva_instancia;

}


//... StructType
//... ArcanaType
//... GenericType (Templates)
// this is going to be hell
