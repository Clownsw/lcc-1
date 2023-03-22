/***********************************
 * File:     Type.h
 *
 * Author:   蔡鹏
 *
 * Email:    iiicp@outlook.com
 *
 * Date:     2023/3/7
 ***********************************/

#ifndef LCC_TYPE_H
#define LCC_TYPE_H
#include <bitset>
#include <cstdint>
#include <memory>
#include <optional>
#include <string_view>
#include <variant>
#include <vector>
#include "Utilities.h"
namespace lcc::Sema {

class Type;

class PrimitiveType final {
  std::uint8_t m_bitCount;
  std::uint8_t m_alignOf : 5;
  bool m_isFloatingPoint : 1;
  bool m_isSigned : 1;

public:
  enum class Kind {
    Char,
    UnsignedChar,
    Bool,
    Short,
    UnsignedShort,
    Int,
    UnsignedInt,
    Long,
    UnsignedLong,
    LongLong,
    UnsignedLongLong,
    Float,
    Double,
    LongDouble,
    Void
  };

private:
  Kind m_kind;

  PrimitiveType(bool isFloatingPoint, bool isSigned, std::uint8_t bitCount,
                std::uint8_t alignOf, Kind kind);

public:
  static Type create(bool isConst, bool isVolatile, bool isFloatingPoint,
                     bool isSigned, std::uint8_t bitCount, std::uint8_t alignOf,
                     Kind kind);

  static Type createChar(bool isConst, bool isVolatile);

  static Type createUnsignedChar(bool isConst, bool isVolatile);

  static Type createUnderlineBool(bool isConst, bool isVolatile);

  static Type createShort(bool isConst, bool isVolatile);

  static Type createUnsignedShort(bool isConst, bool isVolatile);

  static Type createInt(bool isConst, bool isVolatile);

  static Type createUnsignedInt(bool isConst, bool isVolatile);

  static Type createLong(bool isConst, bool isVolatile);

  static Type createUnsignedLong(bool isConst, bool isVolatile);

  static Type createLongLong(bool isConst, bool isVolatile);

  static Type createUnsignedLongLong(bool isConst, bool isVolatile);

  static Type createFloat(bool isConst, bool isVolatile);

  static Type createDouble(bool isConst, bool isVolatile);

  static Type createLongDouble(bool isConst, bool isVolatile);

  static Type createVoid(bool isConst, bool isVolatile);

  [[nodiscard]] bool isFloatingPoint() const {
    return m_isFloatingPoint;
  }

  [[nodiscard]] bool isSigned() const {
    return m_isSigned;
  }

  [[nodiscard]] std::uint8_t getByteCount() const;

  [[nodiscard]] std::uint64_t getSizeOf() const {
    return getByteCount();
  }

  [[nodiscard]] std::uint64_t getAlignOf() const {
    return m_alignOf;
  }

  [[nodiscard]] std::uint8_t getBitCount() const {
    return m_bitCount;
  }

  [[nodiscard]] Kind getKind() const {
    return m_kind;
  }

  bool operator==(const PrimitiveType &rhs) const;

  bool operator!=(const PrimitiveType &rhs) const;
};

class ArrayType final {
  std::shared_ptr<const Type> m_type;
  std::size_t m_size;
  bool m_restricted : 1;
  bool m_static : 1;

  ArrayType(bool isRestricted, bool isStatic, std::shared_ptr<Type> type, std::size_t size);

public:
  static Type create(bool isConst, bool isVolatile, bool isRestricted,
                     bool isStatic, Type type, std::size_t size);

  [[nodiscard]] const Type &getType() const {
    return *m_type;
  }

  [[nodiscard]] std::size_t getSize() const {
    return m_size;
  }

  [[nodiscard]] bool isRestricted() const {
    return m_restricted;
  }

  [[nodiscard]] bool isStatic() const {
    return m_static;
  }

  [[nodiscard]] std::uint64_t getSizeOf() const;

  [[nodiscard]] std::uint64_t getAlignOf() const;

  bool operator==(const ArrayType &rhs) const;

  bool operator!=(const ArrayType &rhs) const;
};

class AbstractArrayType final {
  std::shared_ptr<const Type> m_type;
  bool m_restricted;

  AbstractArrayType(bool isRestricted, std::shared_ptr<Type> &&type);

public:
  static Type create(bool isConst, bool isVolatile, bool isRestricted,
                     Type type);

  [[nodiscard]] const Type &getType() const {
    return *m_type;
  }

  [[nodiscard]] bool isRestricted() const {
    return m_restricted;
  }

  [[nodiscard]] std::uint64_t getSizeOf() const {
    LCC_UNREACHABLE;
  }

  [[nodiscard]] std::uint64_t getAlignOf() const;

  bool operator==(const AbstractArrayType &rhs) const;

  bool operator!=(const AbstractArrayType &rhs) const;
};

class ValArrayType final {
  std::shared_ptr<const Type> m_type;
  bool m_restricted : 1;
  bool m_static : 1;

  ValArrayType(bool isRestricted, bool isStatic, std::shared_ptr<Type> &&type);

public:
  static Type create(bool isConst, bool isVolatile, bool isRestricted,
                     bool isStatic, Type type);

  [[nodiscard]] const Type &getType() const {
    return *m_type;
  }

  [[nodiscard]] bool isRestricted() const {
    return m_restricted;
  }

  [[nodiscard]] bool isStatic() const {
    return m_static;
  }

  [[nodiscard]] std::uint64_t getSizeOf() const {
    LCC_UNREACHABLE;
  }

  [[nodiscard]] std::uint64_t getAlignOf() const;

  bool operator==(const ValArrayType &rhs) const;

  bool operator!=(const ValArrayType &rhs) const;
};

class FunctionType final {
  std::shared_ptr<const Type> m_returnType;
  std::vector<std::pair<Type, std::string_view>> m_arguments;
  bool m_lastIsVararg : 1;

  FunctionType(std::shared_ptr<Type> &&returnType,
               std::vector<std::pair<Type, std::string_view>> arguments,
               bool lastIsVararg);

public:
  static Type create(Type returnType,
                     std::vector<std::pair<Type, std::string_view>> &&arguments,
                     bool lastIsVararg);

  [[nodiscard]] const Type &getReturnType() const {
    return *m_returnType;
  }

  [[nodiscard]] const std::vector<std::pair<Type, std::string_view>> &
  getArguments() const {
    return m_arguments;
  }

  [[nodiscard]] bool isLastVararg() const {
    return m_lastIsVararg;
  }

  [[nodiscard]] std::uint64_t getSizeOf() const {
    LCC_UNREACHABLE;
  }

  [[nodiscard]] std::uint64_t getAlignOf() const {
    LCC_UNREACHABLE;
  }

  bool operator==(const FunctionType &rhs) const;

  bool operator!=(const FunctionType &rhs) const;
};

class StructType final {
  std::string_view m_name;
  std::size_t m_id;

  StructType(std::string_view name, size_t id);

public:
  static Type create(
      bool isConst, bool isVolatile, std::string_view name, size_t id);

  [[nodiscard]] std::string_view getName() const {
    return m_name;
  }

  [[nodiscard]] bool isAnonymous() const {
    return m_name.empty();
  }

  [[nodiscard]] size_t getId() const {
    return m_id;
  }

  [[nodiscard]] uint64_t getSizeOf() const;
  [[nodiscard]] uint64_t getAlignOf() const;

  [[nodiscard]] bool operator==(const StructType &rhs) const {
    return m_id == rhs.m_id;
  }

  [[nodiscard]] bool operator!=(const StructType &rhs) const {
    return !(rhs == *this);
  }
};

class UnionType final {
private:
  std::string_view m_name;
  size_t m_id;

  UnionType(std::string_view name, size_t id);

public:
  static Type create(bool isConst, bool isVolatile, std::string_view name, size_t id);

  [[nodiscard]] std::string_view getName() const {
    return m_name;
  }

  [[nodiscard]] bool isAnonymous() const {
    return m_name.empty();
  }

  [[nodiscard]] size_t getId() const {
    return m_id;
  }

  [[nodiscard]] uint64_t getSizeOf() const;
  [[nodiscard]] uint64_t getAlignOf() const;

  [[nodiscard]] bool operator==(const UnionType& rhs) const {
    return m_id == rhs.m_id;
  }

  [[nodiscard]] bool operator!=(const UnionType& rhs) const {
    return !(rhs == *this);
  }
};

class EnumType final {
  std::string_view m_name;
  size_t m_id;

  EnumType(std::string_view name, size_t id);

public:
  static Type create(bool isConst, bool isVolatile, std::string_view name,
                     size_t id);

  [[nodiscard]] std::string_view getName() const {
    return m_name;
  }

  [[nodiscard]] bool isAnonymous() const {
    return m_name.empty();
  }

  [[nodiscard]] size_t getId() const {
    return m_id;
  }

  [[nodiscard]] uint64_t getSizeOf() const;

  [[nodiscard]] uint64_t getAlignOf() const;

  [[nodiscard]] bool operator==(const EnumType &rhs) const {
    return m_id == rhs.m_id;
  }

  [[nodiscard]] bool operator!=(const EnumType &rhs) const {
    return !(rhs == *this);
  }
};

class PointerType final {
  std::shared_ptr<const Type> m_elementType;
  bool m_restricted;

  PointerType(bool isRestricted, std::shared_ptr<Type> &&elementType);

public:
  static Type create(bool isConst, bool isVolatile, bool isRestricted,
                     Type elementType);

  [[nodiscard]] const Type &getElementType() const {
    return *m_elementType;
  }

  [[nodiscard]] bool isRestricted() const {
    return m_restricted;
  }

  [[nodiscard]] uint64_t getSizeOf() const;

  [[nodiscard]] uint64_t getAlignOf() const;


  bool operator==(const PointerType &rhs) const;

  bool operator!=(const PointerType &rhs) const;
};

class Type final {
  using variant = std::variant<std::monostate, PrimitiveType, ArrayType,
                               AbstractArrayType, ValArrayType, FunctionType,
                               StructType, UnionType, EnumType, PointerType>;

private:
  variant m_type;
  std::string_view m_name;
  bool m_isConst : 1;
  bool m_isVolatile : 1;
public:
  explicit Type(bool isConst = false, bool isVolatile = false,
                variant type = std::monostate{})
      : m_type(std::move(type)), m_isConst(isConst), m_isVolatile(isVolatile) {}

  [[nodiscard]] const variant &getVariant() const {
    return m_type;
  }

  [[nodiscard]] bool isConst() const {
    return m_isConst;
  }

  [[nodiscard]] bool isVolatile() const {
    return m_isVolatile;
  }

  [[nodiscard]] std::string_view getName() const {
    return m_name;
  }

  void setName(std::string_view name) {
    m_name = name;
  }

  [[nodiscard]] bool isTypedef() const {
    return !m_name.empty();
  }

  bool operator==(const Type &rhs) const;

  bool operator!=(const Type &rhs) const;

  [[nodiscard]] bool isUndefined() const {
    return std::holds_alternative<std::monostate>(m_type);
  }

  [[nodiscard]] std::uint64_t getSizeOf() const;

  [[nodiscard]] std::uint64_t getAlignOf() const;

  [[nodiscard]] bool isVoid();

  [[nodiscard]] bool isArray();

  [[nodiscard]] bool isCharArray();

  [[nodiscard]] bool isInteger();

  [[nodiscard]] bool isArithmetic();

  [[nodiscard]] bool isScalar();

  [[nodiscard]] bool isRecord();

  [[nodiscard]] bool isStruct();

  [[nodiscard]] bool isUnion();

  [[nodiscard]] bool isAnonymous();

  [[nodiscard]] bool isEnum();

  [[nodiscard]] bool isBool();

  [[nodiscard]] bool isCharType();

  [[nodiscard]] bool isAggregate();

  [[nodiscard]] bool isVariablyModified();

  [[nodiscard]] bool isVariableLengthArray();

  [[nodiscard]] bool isCharacterLikeType();

  [[nodiscard]] const Type& getArrayElementType();

  [[nodiscard]] Type adjustParameterType();

  [[nodiscard]] Type removeQualifiers();
};
} // namespace lcc::Sema
#endif // LCC_TYPE_H