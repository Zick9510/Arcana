// Types.hpp

#pragma once

#include "Includes.hpp"

/* --- Type System --- */

enum class TypeKind { //...

  VOID,

  POINTER,

  INTEGER,
  FLOAT,

  STRING,

  ARRAY,

  STRUCT,

  ERROR,

  DESCONOCIDO,

};

class ArcanaType {
  public:
    TypeKind kind;
    ArcanaType(TypeKind k)
      : kind(k) {}

    virtual ~ArcanaType()                    = default;
    virtual std::string toString()               const;
    virtual int getBitSize()                     const;
    virtual bool esIgual(const ArcanaType* otro) const;
};

class VoidType : public ArcanaType {
  public:
    VoidType();

    std::string toString()               const override;
    int getBitSize()                     const override;
    bool esIgual(const ArcanaType* otro) const override;
};

class IntegerType : public ArcanaType {
  public:
    int bits; // 8, 16, 32, 64, ...
    bool is_unsigned;
 
    IntegerType(int b, bool u);
 
    std::string toString()               const override;
    int getBitSize()                     const override;
    bool esIgual(const ArcanaType* otro) const override;

};

class FloatType : public ArcanaType {
  public:
    int bits; // 32, 64, ...

    FloatType(int b);

    std::string toString()               const override;
    int getBitSize()                     const override;
    bool esIgual(const ArcanaType* otro) const override;

};

