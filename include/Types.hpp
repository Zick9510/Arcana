// Types.hpp

#pragma once

#include "Includes.hpp"

#include "Symbol.hpp"

class UnknownType : public ArcanaType {
public:
  UnknownType();

  std::string toString()               const override;
  int getBitSize()                     const override;
  bool esIgual(const ArcanaType* otro) const override;
  bool isNumeric()                     const override;

};

class UnresolvedType : public ArcanaType {
public:
  std::string pending_type;

  UnresolvedType(std::string n);

  std::string toString()               const override;
  int getBitSize()                     const override;
  bool esIgual(const ArcanaType* otro) const override;
  bool isNumeric()                     const override;

};

class VoidType : public ArcanaType {
public:
  VoidType();

  std::string toString()               const override;
  int getBitSize()                     const override;
  bool esIgual(const ArcanaType* otro) const override;
  bool isNumeric()                     const override;

};

class BooleanType : public ArcanaType {
public:
  BooleanType();

  std::string toString()               const override;
  int getBitSize()                     const override;
  bool esIgual(const ArcanaType* otro) const override;

};

class PointerType : public ArcanaType {
public:

  std::shared_ptr<ArcanaType> tipo_apuntado;

  PointerType(std::shared_ptr<ArcanaType> t_a);

  std::string toString()               const override;
  int getBitSize()                     const override;
  bool esIgual(const ArcanaType* otro) const override;

  std::shared_ptr<ArcanaType> getUnderlyingType() const override;

};

class IntegerType : public ArcanaType {
public:
  int bits; // 8, 16, 32, 64, ...
  bool is_unsigned;

  IntegerType(int b, bool u);

  std::string toString()               const override;
  int getBitSize()                     const override;
  bool esIgual(const ArcanaType* otro) const override;
  bool isSigned()                      const override;

};

class FloatType : public ArcanaType {
public:
  int bits; // 32, 64, ...

  FloatType(int b);

  std::string toString()               const override;
  int getBitSize()                     const override;
  bool esIgual(const ArcanaType* otro) const override;
  bool isSigned()                      const override;

};

class CharType : public ArcanaType {
public:
  int bits;

  CharType(int b);

  std::string toString() const override;
  int getBitSize() const override;
  bool esIgual(const ArcanaType* otro) const override;

};

class MorphType : public ArcanaType {
public:
  int selector_bytes; // Bytes to select the one thing we need
  int bits; // Biggest element's size
  std::vector<std::shared_ptr<ArcanaType>> subtipos;

  MorphType(std::vector<std::shared_ptr<ArcanaType>> st);

  std::string toString()               const override;
  int getBitSize()                     const override;
  bool esIgual(const ArcanaType* otro) const override;
  bool isNumeric()                     const override;

};

class StructType : public ArcanaType {
public:
  std::string nombre;
  InfoStruct*  info ;

  StructType(InfoStruct* i);

  std::string toString()               const override;
  int getBitSize()                     const override;
  bool esIgual(const ArcanaType* otro) const override;
  bool isNumeric()                     const override;

};

/* --- Factory --- */

class TypeFactory { //...
private:
  std::shared_ptr<UnknownType> cacheUnknown;
  std::shared_ptr<VoidType   > cacheVoid;
  std::shared_ptr<BooleanType> cacheBoolean;

  std::map<std::string                , std::shared_ptr<UnresolvedType>> cacheUnresolved;
  std::map<std::shared_ptr<ArcanaType>, std::shared_ptr<PointerType   >> cachePointer   ;
  std::map<std::tuple<int, bool>      , std::shared_ptr<IntegerType   >> cacheInteger   ;
  std::map<int                        , std::shared_ptr<FloatType     >> cacheFloat     ;
  std::map<int                        , std::shared_ptr<CharType      >> cacheChar      ;
  std::map<std::string                , std::shared_ptr<MorphType     >> cacheMorph     ;
  std::map<std::string                , std::shared_ptr<StructType    >> cacheStruct    ;

public:
  std::shared_ptr<UnknownType   > getUnknown   ();
  std::shared_ptr<UnresolvedType> getUnresolved(std::string name);
  std::shared_ptr<VoidType      > getVoid      ();
  std::shared_ptr<PointerType   > getPointer   (std::shared_ptr<ArcanaType> base);
  std::shared_ptr<BooleanType   > getBoolean   ();
  std::shared_ptr<IntegerType   > getInteger   (int bits, bool is_unsigned);
  std::shared_ptr<FloatType     > getFloat     (int bits);
  std::shared_ptr<CharType      > getChar      (int bits);
  std::shared_ptr<MorphType     > getMorph     (std::vector<std::shared_ptr<ArcanaType>> subtipos);
  std::shared_ptr<StructType    > getStruct    (InfoStruct* info);

};
