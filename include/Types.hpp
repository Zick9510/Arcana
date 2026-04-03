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

/* --- Factory --- */

class TypeFactory { //...
private:
  std::map<std::tuple<int, bool>, std::shared_ptr<IntegerType>> cacheInteger;
  std::map<float, std::shared_ptr<FloatType>> cacheFloat;

public:
  std::shared_ptr<IntegerType> getInteger(int bits, bool is_unsigned);
  std::shared_ptr<FloatType>   getFloat  (int bits);

};
