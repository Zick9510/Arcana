// Types.hpp

#pragma once

#include "Common.hpp"

/* --- Type System --- */

enum class TypeKind { //...

  Void,

  Pointer,

  Integer,
  Float,

  Array,

  Struct,

  Error,

};

class ArcanaType {
  public:
    TypeKind kind;
    ArcanaType(TypeKind k)
      : kind(k) {}

    virtual ~ArcanaType() = default;
    virtual std::string toString() const = 0;
    virtual int getBitSize() const = 0;
    virtual bool esIgual(const ArcanaType* otro) const = 0;
};


class VoidType : public ArcanaType {
  public:
    VoidType() : ArcanaType(TypeKind::Void) {}

    std::string toString() const override { return "void"; }
    int getBitSize() const override { return 0; }
    bool esIgual(const ArcanaType* otro) const override {
      return otro->kind == TypeKind::Void;
    }
};

class IntegerType : public ArcanaType {
  public:
    int bits; // 8, 16, 32, 64, ...
    bool is_unsigned;
 
    IntegerType(int b, bool u);
 
    std::string toString() const override;
    int getBitSize() const override;
    bool esIgual(const ArcanaType* otro) const override;

};

class FloatType : public ArcanaType {
  public:
  int bits; // 32, 64, ...

  FloatType(int b);

  std::string toString() const override;
  int getBitSize() const override;
  bool esIgual(const ArcanaType* otro) const override;

};
