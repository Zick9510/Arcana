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

    virtual ~ArcanaType()                             ;
    virtual std::string toString()               const = 0;
    virtual int getBitSize()                     const = 0;
    virtual bool esIgual(const ArcanaType* otro) const = 0;
};

class UnknownType : public ArcanaType {
public:
  UnknownType();
  std::string toString()               const override;
  int getBitSize()                     const override;
  bool esIgual(const ArcanaType* otro) const override;

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
  std::shared_ptr<UnknownType> cacheUnknown;
  std::shared_ptr<VoidType> cacheVoid;
  std::map<std::tuple<int, bool>, std::shared_ptr<IntegerType>> cacheInteger;
  std::map<float, std::shared_ptr<FloatType>> cacheFloat;

public:
  std::shared_ptr<UnknownType>  getUnknown();
  std::shared_ptr<VoidType>    getVoid();
  std::shared_ptr<IntegerType> getInteger(int bits, bool is_unsigned);
  std::shared_ptr<FloatType>   getFloat  (int bits);

};
