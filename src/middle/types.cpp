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

int PointerType::getBitSize()       const { return 64                             ; }

bool PointerType::esIgual(const ArcanaType* otro) const {
  if (otro->kind != TypeKind::POINTER) { return false; }

  auto o = static_cast<const PointerType*>(otro);

  return (this->tipo_apuntado == o->tipo_apuntado);

}

std::shared_ptr<ArcanaType> PointerType::getUnderlyingType() const {
  return tipo_apuntado;
}

// --- BooleanType ---
BooleanType::BooleanType()
  : ArcanaType(TypeKind::BOOLEAN) {}

std::string BooleanType::toString() const { return "bool"; }

int BooleanType::getBitSize()       const { return 8     ; }

bool BooleanType::esIgual(const ArcanaType* otro) const {
  return (otro->kind == TypeKind::BOOLEAN);

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

bool IntegerType::isSigned() const { return !is_unsigned; }

// --- FloatType ---
FloatType::FloatType(int b)
  : ArcanaType(TypeKind::FLOAT), bits(b) {}

std::string FloatType::toString() const {
  return "f" + std::to_string(bits);
}

int FloatType::getBitSize() const { return bits; }

bool FloatType::esIgual(const ArcanaType* otro) const {

  if (otro->kind != TypeKind::FLOAT) {
    return false;
  }

  auto o = static_cast<const FloatType*>(otro);

  return (this->bits == o->bits);
}

bool FloatType::isSigned() const { return true; }

// --- CharType ---
CharType::CharType(int b)
  : ArcanaType(TypeKind::CHAR), bits(b) {}

std::string CharType::toString() const {
  return "c" + std::to_string(bits);
}

int CharType::getBitSize() const { return bits; }

bool CharType::esIgual(const ArcanaType* otro) const {

  if (otro->kind != TypeKind::CHAR) {
    return false;
  }

  auto o = static_cast<const CharType*>(otro);

  return (this->bits == o->bits);

}

// --- MorphType ---
MorphType::MorphType(std::vector<std::shared_ptr<ArcanaType>> st)
  : ArcanaType(TypeKind::MORPH), subtipos(st) {
  bits = -1;
  for (const auto& s : subtipos) {
    bits = std::max(bits, s->getBitSize());
  }
}

std::string MorphType::toString() const {
  std::string res = "[";
  for (size_t i = 0; i < subtipos.size(); ++i) {
    res += subtipos[i]->toString();
    if (i < subtipos.size() - 1) { res += ", "; }
  }
  return res + "]";
}

int MorphType::getBitSize() const { return bits; }

bool MorphType::esIgual(const ArcanaType* otro) const {
  return true; //...
}

// --- ShapeType ---
ShapeType::ShapeType(std::vector<CampoShape> c)
  : ArcanaType(TypeKind::SHAPE), campos(c) {}

std::string ShapeType::toString() const {
  return ""; //...

}

int ShapeType::getBitSize() const {
  return -1; //...

}

bool ShapeType::esIgual(const ArcanaType* otro) const {
  return true; //...
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

std::shared_ptr<BooleanType> TypeFactory::getBoolean() {
  if (cacheBoolean != nullptr)  {
    return cacheBoolean;

  }

  auto nueva_instancia = std::make_shared<BooleanType>();
  cacheBoolean = nueva_instancia;
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

std::shared_ptr<CharType> TypeFactory::getChar(int bits) {

  if (cacheChar.find(bits) != cacheChar.end()) {
    return cacheChar[bits];

  }

  auto nueva_instancia = std::make_shared<CharType>(bits);
  cacheChar[bits] = nueva_instancia;
  return nueva_instancia;

}

std::shared_ptr<MorphType> TypeFactory::getMorph(std::vector<std::shared_ptr<ArcanaType>> subtipos) {
  std::string firma = "[";
  for (size_t i = 0; i < subtipos.size(); ++i) {
    if (subtipos[i] != nullptr) {
      firma += subtipos[i]->toString();
      if (i + 1 < subtipos.size()) { firma += ", "; }
    }
  }
  firma += "]";

  if (cacheMorph.find(firma) != cacheMorph.end()) {
    return cacheMorph[firma];
  }

  auto nueva_instancia = std::make_shared<MorphType>(subtipos);
  cacheMorph[firma] = nueva_instancia;
  return nueva_instancia;
}

std::shared_ptr<ShapeType> TypeFactory::getShape(std::vector<CampoShape> campos) {

}

//... GenericType (Templates)
