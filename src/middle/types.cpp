// types.cpp

#include "Types.hpp"

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

Factory::Factory() : {}

IntegerType* Factory::getInteger(int bits, bool is_unsigned) {
  auto key = std::make_tuple(bits, is_unsigned);
  auto it = cacheInteger.find(key);

  if (it != cacheInteger.end()) {
    return it->second.get();

  }

  auto nueva_instancia = std::make_unique<IntegerType>(bits, is_unsigned);
  IntegerType* ptr = nueva_instancia.get();
  cacheInteger[key] = std::move(nueva_instancia);

  return ptr;
}

FloatType* Factory::getFloat(int bits) {
  auto key = std::make_tuple(bits);
  auto it = cacheFloat.find(key);

  if (it != cacheFloat.end()) {
    return it->second.get();

  }

  auto nueva_instancia = std::make_unique<FloatType>(bits);
  FloatType* ptr = nueva_instancia.get();
  cacheFloat[key] = std::move(nueva_instancia);

  return ptr;

}


//... StructType
//... ArcanaType
//... GenericType (Templates)
// this is going to be hell
