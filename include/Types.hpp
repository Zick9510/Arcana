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

class Factory { //...
private:
  std::map<std::tuple<int, bool>, std::unique_ptr<IntegerType>> cacheInteger;
  std::map<std::tuple<float>, std::unique_ptr<FloatType>> cacheFloat;

public:
  IntegerType* getInteger(int bits, bool is_unsigned) {
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

  FloatType* getFloat(int bits) {
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


};
