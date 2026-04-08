// types.cpp

#include "Types.hpp"

#include "Includes.hpp"

/* --- Arcana Types --- */

ArcanaType::~ArcanaType() = default;

// --- UnkownType ---

UnknownType::UnknownType()
  : ArcanaType(TypeKind::DESCONOCIDO) {}

std::string UnknownType::toString() const { return "unkown"; }

int UnknownType::getBitSize()       const { return 0       ; }

bool UnknownType::esIgual(const ArcanaType* otro) const {
  return (otro->kind == TypeKind::DESCONOCIDO);
}

// --- VoidType ---

VoidType::VoidType()
  : ArcanaType(TypeKind::VOID) {}

std::string VoidType::toString() const { return "void"; }

int VoidType::getBitSize()       const { return 0     ; }

bool VoidType::esIgual(const ArcanaType* otro) const {
  return (otro->kind == TypeKind::VOID);
}

// --- PointerType ---
PointerType::PointerType(std::shared_ptr<ArcanaType> t_a)
  : ArcanaType(TypeKind::POINTER), tipo_apuntado(t_a) {}

std::string PointerType::toString() const { return tipo_apuntado->toString() + "*"; }

int PointerType::getBitSize() const { return 64; }

bool PointerType::esIgual(const ArcanaType* otro) const {
  if (otro->kind != TypeKind::POINTER) { return false; }

  auto o = static_cast<const PointerType*>(otro);

  return (this->tipo_apuntado == o->tipo_apuntado);

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

std::shared_ptr<UnknownType> TypeFactory::getUnknown() {

  if (cacheUnknown != nullptr) {
    return cacheUnknown;

  }

  auto nueva_instancia = std::make_shared<UnknownType>();
  cacheUnknown = nueva_instancia;

  return nueva_instancia;

}

std::shared_ptr<VoidType> TypeFactory::getVoid() {

  if (cacheVoid != nullptr) {
    return cacheVoid;

  }

  auto nueva_instancia = std::make_shared<VoidType>();
  cacheVoid = nueva_instancia;

  return nueva_instancia;

}

std::shared_ptr<PointerType> TypeFactory::getPointer(std::shared_ptr<ArcanaType> base) {
  if (cachePointer.find(base) != cachePointer.end()) {
    return cachePointer[base];
  }

  auto nueva_instancia = std::make_shared<PointerType>(base);
  cachePointer[base] = nueva_instancia;
  return nueva_instancia;

}

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
