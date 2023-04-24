/***********************************
 * File:     Syntax.h
 *
 * Author:   caipeng
 *
 * Email:    iiicp@outlook.com
 *
 * Date:     2022/10/13
 *
 * Sign:     enjoy life
 ***********************************/
#ifndef LCC_SYNTAX_H
#define LCC_SYNTAX_H
#include "Token.h"
#include "Box.h"
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace lcc::Syntax {
class PrimaryExprIdent;
class PrimaryExprConstant;
class PrimaryExprParentheses;

class PostFixExprSubscript;
class PostFixExprIncrement;
class PostFixExprDecrement;
class PostFixExprDot;
class PostFixExprArrow;
class PostFixExprFuncCall;
class PostFixExprPrimaryExpr;
class PostFixExprTypeInitializer;

class UnaryExprUnaryOperator;
class UnaryExprSizeOf;

class TypeName;
class CastExpr;
class MultiExpr;
class AdditiveExpr;
class ShiftExpr;
class RelationalExpr;
class EqualExpr;
class BitAndExpr;
class BitXorExpr;
class BitOrExpr;
class LogAndExpr;
class LogOrExpr;
class ConditionalExpr;
using ConstantExpr = ConditionalExpr;
class AssignExpr;
class Expr;

class ReturnStmt;
class ExprStmt;
class IfStmt;
class SwitchStmt;
class DefaultStmt;
class CaseStmt;
class BlockStmt;
class ForStmt;
class DoWhileStmt;
class WhileStmt;
class BreakStmt;
class ContinueStmt;
class GotoStmt;
class LabelStmt;

class TranslationUnit;
class FunctionDefinition;
class Declaration;
class DeclarationSpecifiers;
class SpecifierQualifiers;
class StorageClassSpecifier;
class TypeQualifier;
class TypeSpecifier;
class EnumSpecifier;
class EnumeratorList;
class StructOrUnionSpecifier;
class Declarator;
class DirectDeclaratorIdent;
class DirectDeclaratorParentheses;
class DirectDeclaratorAssignExpr;
class DirectDeclaratorAsterisk;
class DirectDeclaratorParamTypeList;
class AbstractDeclarator;
class DirectAbstractDeclaratorParentheses;
class DirectAbstractDeclaratorParamTypeList;
class DirectAbstractDeclaratorAssignExpr;
class DirectAbstractDeclaratorAsterisk;
class Pointer;
class ParamTypeList;
class ParameterDeclaration;
class ParamList;
class Initializer;
class InitializerList;

using ExprBox = box<Expr>;
using AssignExprBox = box<AssignExpr>;
using CastExprBox = box<CastExpr>;
using TypeNameBox = box<TypeName>;

using InitializerListBox = box<InitializerList>;
using AbstractDeclaratorBox = box<AbstractDeclarator>;

class Node {
private:
  TokIter beginTokLoc_;

public:
  Node(TokIter beginTokLoc) : beginTokLoc_(beginTokLoc) {}
  virtual ~Node() = default;
  TokIter getBeginLoc() const { return beginTokLoc_; }
};

/*
 * primary-expression:
 *    identifier
 */
class PrimaryExprIdent final : public Node {
private:
  std::string_view ident_;

public:
  PrimaryExprIdent(TokIter begin, std::string_view identifier)
      : Node(begin), ident_(identifier) {}
  [[nodiscard]] std::string_view getIdentifier() const { return ident_; }
};

/*
 * primary-expression:
 *    constant
 */
class PrimaryExprConstant final : public Node {
public:
  using Value = std::variant<int32_t, uint32_t, int64_t, uint64_t, float,
                             double, std::string>;

private:
  Value value_;

public:
  PrimaryExprConstant(TokIter begin, Value value)
      : Node(begin), value_(value) {}
  [[nodiscard]] const Value &getValue() const { return value_; }
};

/*
 * primary-expression:
 *    ( expression )
 */
class PrimaryExprParentheses final : public Node {
private:
  ExprBox expr_;

public:
  PrimaryExprParentheses(TokIter begin, ExprBox expr)
      : Node(begin), expr_(expr) {}
  [[nodiscard]] const Expr &getExpr() const { return *expr_; }
};

/*
 * primary-expression:
 *    identifier
 *    constant
 *    string-literal
 *    ( expression )
 */
using PrimaryExpr =
    std::variant<PrimaryExprIdent, PrimaryExprConstant, PrimaryExprParentheses>;

/**
 * postfix-expression:
 *    primary-expression
 *    postfix-expression [ expression ]
 *    postfix-expression ( argument-expression-list{opt} )
 *    postfix-expression . identifier
 *    postfix-expression -> identifier
 *    postfix-expression ++
 *    postfix-expression --
 *    ( type-name ) { initializer-list }
 *    ( type-name ) { initializer-list , }
 */
using PostFixExpr =
    std::variant<PrimaryExpr, box<PostFixExprSubscript>,
                 box<PostFixExprFuncCall>, box<PostFixExprDot>,
                 box<PostFixExprArrow>, box<PostFixExprIncrement>,
                 box<PostFixExprDecrement>, box<PostFixExprTypeInitializer>>;

/**
 * postfix-expression:
 *    postfix-expression [ expression ]
 */
class PostFixExprSubscript final : public Node {
private:
  PostFixExpr postFixExpr_;
  ExprBox expr_;

public:
  PostFixExprSubscript(TokIter begin, PostFixExpr postFixExpr, ExprBox expr)
      : Node(begin), postFixExpr_(postFixExpr), expr_(expr) {}
  [[nodiscard]] const PostFixExpr &getPostFixExpr() const {
    return postFixExpr_;
  }
  [[nodiscard]] const Expr &getExpr() const { return *expr_; }
};

/**
 * postfix-expression:
 *    postfix-expression ( argument-expression-list{opt} )
 *
 * argument-expression-list:
 *    assignment-expression
 *    argument-expression-list , assignment-expression
 */
class PostFixExprFuncCall final : public Node {
private:
  PostFixExpr postFixExpr_;
  std::vector<AssignExprBox> params_;

public:
  PostFixExprFuncCall(TokIter begin, PostFixExpr postFixExpr,
                      std::vector<AssignExprBox> params)
      : Node(begin), postFixExpr_(postFixExpr), params_(params) {}

  [[nodiscard]] const PostFixExpr &getPostFixExpr() const {
    return postFixExpr_;
  }
  [[nodiscard]] const std::vector<AssignExprBox> &
  getOptionalAssignExpressions() const {
    return params_;
  }
};

/**
 * postfix-expression:
 *    postfix-expression . identifier
 */
class PostFixExprDot final : public Node {
private:
  PostFixExpr postFixExpr_;
  std::string_view identifier_;

public:
  PostFixExprDot(TokIter begin, PostFixExpr postFixExpr,
                 std::string_view identifier)
      : Node(begin), postFixExpr_(postFixExpr), identifier_(identifier) {}

  [[nodiscard]] const PostFixExpr &getPostFixExpr() const {
    return postFixExpr_;
  }
  [[nodiscard]] std::string_view getIdentifier() const { return identifier_; }
};

/**
 * postfix-expression:
 *    postfix-expression -> identifier
 */
class PostFixExprArrow final : public Node {
private:
  PostFixExpr postFixExpr_;
  std::string_view identifier_;

public:
  PostFixExprArrow(TokIter begin, PostFixExpr postFixExpr,
                   std::string_view identifier)
      : Node(begin), postFixExpr_(postFixExpr), identifier_(identifier) {}
  [[nodiscard]] const PostFixExpr &getPostFixExpr() const {
    return postFixExpr_;
  }
  [[nodiscard]] std::string_view getIdentifier() const { return identifier_; }
};

/**
 * postfix-expression:
 *    postfix-expression ++
 */
class PostFixExprIncrement final : public Node {
private:
  PostFixExpr postFixExpr_;

public:
  PostFixExprIncrement(TokIter begin, PostFixExpr postFixExpr)
      : Node(begin), postFixExpr_(postFixExpr) {}
  [[nodiscard]] const PostFixExpr &getPostFixExpr() const {
    return postFixExpr_;
  }
};

/**
 * postfix-expression:
 *    postfix-expression --
 */
class PostFixExprDecrement final : public Node {
private:
  PostFixExpr postFixExpr_;

public:
  PostFixExprDecrement(TokIter begin, PostFixExpr postFixExpr)
      : Node(begin), postFixExpr_(postFixExpr) {}
  [[nodiscard]] const PostFixExpr &getPostFixExpr() const {
    return postFixExpr_;
  }
};

/**
 * postfix-expression:
 *   ( type-name ) { initializer-list }
 *   ( type-name ) { initializer-list , }
 */
class PostFixExprTypeInitializer final : public Node {
private:
  TypeNameBox typeName_;
  InitializerListBox initializerList_;

public:
  PostFixExprTypeInitializer(TokIter begin, TypeNameBox typeName,
                             InitializerListBox initializerList)
      : Node(begin), typeName_(typeName), initializerList_(initializerList) {}

  [[nodiscard]] const InitializerList &getInitializerList() const {
    return *initializerList_;
  }
  [[nodiscard]] const TypeName &getTypeName() const { return *typeName_; }
};

/**
 * unary-expression:
 *  postfix-expression
 *  ++ unary-expression
 *  -- unary-expression
 *  unary-operator cast-expression
 *  sizeof unary-expression
 *  sizeof ( type-name )
 *
 *  unary-operator: one of
 *      & * + - ~ !
 */
using UnaryExpr = std::variant<PostFixExpr, box<UnaryExprUnaryOperator>,
                               box<UnaryExprSizeOf>>;

/**
 * unary-expression:
 *  unary-operator cast-expression
 *  ++ unary-expression
 *  -- unary-expression
 *
 *  unary-operator: one of
 *  & * + - ~ !
 */
class UnaryExprUnaryOperator final : public Node {
public:
  enum class UnaryOperator : uint8_t {
    Increment,
    Decrement,
    Ampersand,
    Asterisk,
    Plus,
    Minus,
    BitNot,
    LogicalNot
  };
  using Variant = std::variant<UnaryExpr, CastExprBox>;

private:
  UnaryOperator operator_;
  Variant value_;

public:
  UnaryExprUnaryOperator(TokIter begin, UnaryOperator anOperator, Variant value)
      : Node(begin), operator_(anOperator), value_(value) {}

  [[nodiscard]] UnaryOperator getOperator() const { return operator_; }
  [[nodiscard]] const CastExpr *getCastExpr() const {
    if (std::holds_alternative<CastExprBox>(value_)) {
      return std::get<CastExprBox>(value_).get();
    }
    return nullptr;
  }
};

/**
 * unary-expression:
 *  sizeof unary-expression
 *  sizeof ( type-name )
 */
class UnaryExprSizeOf final : public Node {
  using Variant = std::variant<UnaryExpr, TypeNameBox>;
  Variant mValue;

public:
  UnaryExprSizeOf(TokIter begin, Variant variant)
      : Node(begin), mValue(variant) {}

  [[nodiscard]] const Variant &getVariant() const { return mValue; }
};

/**
 * type-specifier:
 *  void char short int long float double signed unsigned _Bool
 *  struct-or-union-specifier
 *  enum-specifier
 *  typedef-name
 */
class TypeSpecifier final : public Node {
public:
  enum PrimitiveTypeSpecifier {
    Void = 0b1,
    Char = 0b10,
    Short = 0b100,
    Int = 0b1000,
    Long = 0b10000,
    Float = 0b100000,
    Double = 0b1000000,
    Signed = 0b10000000,
    Unsigned = 0b100000000,
    Bool = 0b1000000000
  };
  using TypedefName = std::string_view;

private:
  using variant =
      std::variant<PrimitiveTypeSpecifier, box<StructOrUnionSpecifier>,
                   box<EnumSpecifier>, TypedefName>;

  variant mVariant;

public:
  TypeSpecifier(TokIter begin, variant variant)
      : Node(begin), mVariant(variant) {}

  [[nodiscard]] const variant &getVariant() const { return mVariant; }
};

/**
 * type-qualifier:
 *      const
 *      restrict
 *      volatile
 */
class TypeQualifier final : public Node {
public:
  enum Qualifier { Const, Restrict, Volatile };

private:
  Qualifier mQualifier;

public:
  TypeQualifier(TokIter begin, Qualifier qualifier)
      : Node(begin), mQualifier(qualifier) {}
  [[nodiscard]] Qualifier getQualifier() const { return mQualifier; }
};

/**
 * function-specifier:
 *      inline
 */
class FunctionSpecifier final : public Node {
public:
  FunctionSpecifier(TokIter begin) : Node(begin) {}
};

/**
 * storage-class-specifier:
 *      typedef
 *      extern
 *      static
 *      auto
 *      register
 */
class StorageClassSpecifier final : public Node {
public:
  enum Specifiers { Typedef, Extern, Static, Auto, Register };

private:
  Specifiers mSpecifier;

public:
  StorageClassSpecifier(TokIter begin, Specifiers specifier)
      : Node(begin), mSpecifier(specifier) {}
  [[nodiscard]] Specifiers getSpecifier() const { return mSpecifier; }
};

/**
 * declaration-specifiers:
        storage-class-specifier declaration-specifiers{opt}
        type-specifier declaration-specifiers{opt}
        type-qualifier declaration-specifiers{opt}
        function-specifier declaration-specifiers{opt}
 */
class DeclarationSpecifiers final : public Node {
private:
  std::vector<StorageClassSpecifier> mStorageClassSpecifiers;
  std::vector<TypeSpecifier> mTypeSpecifiers;
  std::vector<TypeQualifier> mTypeQualifiers;
  std::vector<FunctionSpecifier> mFunctionSpecifiers;

public:
  DeclarationSpecifiers(TokIter begin);
  void addStorageClassSpecifier(StorageClassSpecifier &&specifier);
  void addTypeSpecifier(TypeSpecifier &&specifier);
  void addTypeQualifier(TypeQualifier &&qualifier);
  void addFunctionSpecifier(FunctionSpecifier &&specifier);

  [[nodiscard]] const std::vector<StorageClassSpecifier> &
  getStorageClassSpecifiers() const;
  [[nodiscard]] const std::vector<TypeSpecifier> &getTypeSpecifiers() const;
  [[nodiscard]] const std::vector<TypeQualifier> &getTypeQualifiers() const;
  [[nodiscard]] const std::vector<FunctionSpecifier> &
  getFunctionSpecifiers() const;
  [[nodiscard]] bool isEmpty() const;
};

/**
 * SpecifierQualifier
 *      type-specifier
 *      type-qualifier
 */

/**
 * type-name:
 *  specifier-qualifier-list abstract-declarator{opt}
 */
class TypeName final : public Node {
private:
  DeclarationSpecifiers mSpecifierQualifiers;
  std::optional<AbstractDeclaratorBox> mAbstractDeclarator;

public:
  TypeName(
      TokIter begin, DeclarationSpecifiers specifierQualifiers,
      std::optional<AbstractDeclaratorBox> abstractDeclarator = {std::nullopt})
      : Node(begin), mSpecifierQualifiers(specifierQualifiers),
        mAbstractDeclarator(mAbstractDeclarator) {}
  [[nodiscard]] const DeclarationSpecifiers &getSpecifierQualifiers() const {
    return mSpecifierQualifiers;
  }
  [[nodiscard]] const AbstractDeclarator *getAbstractDeclarator() const {
    if (mAbstractDeclarator.has_value()) {
      return mAbstractDeclarator.value().get();
    }
    return nullptr;
  }
};

/**
 * cast-expression:
 *      unary-expression
 *      ( type-name ) cast-expression
 */
class CastExpr final : public Node {
public:
  using TypeNameCast = std::pair<TypeName, CastExprBox>;
  using Variant = std::variant<UnaryExpr, TypeNameCast>;

public:
  Variant mVariant;

public:
  CastExpr(TokIter begin, Variant unaryOrCast)
      : Node(begin), mVariant(unaryOrCast) {}
  [[nodiscard]] const Variant &getVariant() const { return mVariant; }
};

/**
 * multiplicative-expression:
 *  cast-expression
 *  multiplicative-expression * cast-expression
 *  multiplicative-expression / cast-expression
 *  multiplicative-expression % cast-expression
 */
class MultiExpr final : public Node {
public:
  enum MultiOperator { Multiply, Divide, Modulo };

private:
  CastExpr castExpr_;
  std::vector<std::pair<MultiOperator, CastExpr>> optionalCastExps_;

public:
  explicit MultiExpr(
      TokIter begin, CastExpr castExpr,
      std::vector<std::pair<MultiOperator, CastExpr>> optionalCastExps)
      : Node(begin), castExpr_(castExpr), optionalCastExps_(optionalCastExps) {}
  [[nodiscard]] const CastExpr &getCastExpr() const { return castExpr_; }
  [[nodiscard]] const std::vector<std::pair<MultiOperator, CastExpr>> &
  getOptionalCastExps() const {
    return optionalCastExps_;
  }
};

/**
 * additive-expression:
 * multiplicative-expression
 * additive-expression + multiplicative-expression
 * additive-expression - multiplicative-expression
 */
class AdditiveExpr final : public Node {
public:
  enum AdditiveOperator { Plus, Minus };

private:
  MultiExpr multiExpr_;
  std::vector<std::pair<AdditiveOperator, MultiExpr>> optionalMultiExps_;

public:
  AdditiveExpr(
      TokIter begin, MultiExpr multiExpr,
      std::vector<std::pair<AdditiveOperator, MultiExpr>> optionalMultiExps)
      : Node(begin), multiExpr_(multiExpr),
        optionalMultiExps_(optionalMultiExps) {}
  [[nodiscard]] const MultiExpr &getMultiExpr() const { return multiExpr_; }
  [[nodiscard]] const std::vector<std::pair<AdditiveOperator, MultiExpr>> &
  getOptionalMultiExps() const {
    return optionalMultiExps_;
  }
};

/**
 * shift-expression:
 *      additive-expression
 *      shift-expression << additive-expression
 *      shift-expression >> additive-expression
 */
class ShiftExpr final : public Node {
public:
  enum ShiftOperator { Right, Left };

private:
  AdditiveExpr additiveExpr_;
  std::vector<std::pair<ShiftOperator, AdditiveExpr>> optionalAdditiveExps_;

public:
  ShiftExpr(
      TokIter begin, AdditiveExpr additiveExpr,
      std::vector<std::pair<ShiftOperator, AdditiveExpr>> optionalAdditiveExps)
      : Node(begin), additiveExpr_(additiveExpr),
        optionalAdditiveExps_(optionalAdditiveExps) {}
  [[nodiscard]] const AdditiveExpr &getAdditiveExpr() const {
    return additiveExpr_;
  }
  [[nodiscard]] const std::vector<std::pair<ShiftOperator, AdditiveExpr>> &
  getOptAdditiveExps() const {
    return optionalAdditiveExps_;
  }
};

/**
 * relational-expression:
 *      shift-expression
 *      relational-expression < shift-expression
 *      relational-expression > shift-expression
 *      relational-expression <= shift-expression
 *      relational-expression >= shift-expression
 */
class RelationalExpr final : public Node {
public:
  enum RelationalOperator {
    LessThan,
    LessThanOrEqual,
    GreaterThan,
    GreaterThanOrEqual
  };

private:
  ShiftExpr shiftExpr_;
  std::vector<std::pair<RelationalOperator, ShiftExpr>> optionalShiftExps_;

public:
  RelationalExpr(
      TokIter begin, ShiftExpr shiftExpr,
      std::vector<std::pair<RelationalOperator, ShiftExpr>> optionalShiftExps)
      : Node(begin), shiftExpr_(shiftExpr),
        optionalShiftExps_(optionalShiftExps) {}
  [[nodiscard]] const ShiftExpr &getShiftExpr() const { return shiftExpr_; }
  [[nodiscard]] const std::vector<std::pair<RelationalOperator, ShiftExpr>> &
  getOptionalShiftExpressions() const {
    return optionalShiftExps_;
  }
};

/**
 * equality-expression:
 *      relational-expression
 *      equality-expression == relational-expression
 *      equality-expression != relational-expression
 */
class EqualExpr final : public Node {
public:
  enum EqualOperator { Equal, NotEqual };

private:
  RelationalExpr relationalExpr_;
  std::vector<std::pair<EqualOperator, RelationalExpr>> optionalRelationalExps_;

public:
  EqualExpr(TokIter begin, RelationalExpr relationalExpr,
            std::vector<std::pair<EqualOperator, RelationalExpr>>
                optionalRelationalExps)
      : Node(begin), relationalExpr_(relationalExpr),
        optionalRelationalExps_(optionalRelationalExps) {}
  [[nodiscard]] const RelationalExpr &getRelationalExpr() const {
    return relationalExpr_;
  }

  [[nodiscard]] const std::vector<std::pair<EqualOperator, RelationalExpr>> &
  getOptionalRelationalExpr() const {
    return optionalRelationalExps_;
  }
};

/**
 * AND-expression:
 *      equality-expression
 *      AND-expression & equality-expression
 */
class BitAndExpr final : public Node {
private:
  EqualExpr equalExpr_;
  std::vector<EqualExpr> optionalEqualExps_;

public:
  BitAndExpr(TokIter begin, EqualExpr equalExpr,
             std::vector<EqualExpr> optionalEqualExps)
      : Node(begin), equalExpr_(equalExpr),
        optionalEqualExps_(optionalEqualExps) {}
  [[nodiscard]] const EqualExpr &getEqualExpr() const { return equalExpr_; }

  [[nodiscard]] const std::vector<EqualExpr> &getOptionalEqualExpr() const {
    return optionalEqualExps_;
  }
};

/**
 * exclusive-OR-expression:
 *      AND-expression
 *      exclusive-OR-expression ^ AND-expression
 */
class BitXorExpr final : public Node {
private:
  BitAndExpr bitAndExpr_;
  std::vector<BitAndExpr> optionalBitAndExps_;

public:
  BitXorExpr(TokIter begin, BitAndExpr bitAndExpr,
             std::vector<BitAndExpr> optionalBitAndExps)
      : Node(begin), bitAndExpr_(bitAndExpr),
        optionalBitAndExps_(optionalBitAndExps) {}
  [[nodiscard]] const BitAndExpr &getBitAndExpr() const { return bitAndExpr_; }
  [[nodiscard]] const std::vector<BitAndExpr> &
  getOptionalBitAndExpressions() const {
    return optionalBitAndExps_;
  }
};

/**
 * inclusive-OR-expression:
 *      exclusive-OR-expression
 *      inclusive-OR-expression | exclusive-OR-expression
 */
class BitOrExpr final : public Node {
private:
  BitXorExpr bitXorExpr_;
  std::vector<BitXorExpr> optionalBitXorExps_;

public:
  BitOrExpr(TokIter begin, BitXorExpr bitXorExpr,
            std::vector<BitXorExpr> optionalBitXorExps)
      : Node(begin), bitXorExpr_(bitXorExpr),
        optionalBitXorExps_(optionalBitXorExps) {}
  [[nodiscard]] const BitXorExpr &getBitXorExpression() const {
    return bitXorExpr_;
  }

  [[nodiscard]] const std::vector<BitXorExpr> &
  getOptionalBitXorExpressions() const {
    return optionalBitXorExps_;
  }
};

/**
 * logical-AND-expression:
 *      inclusive-OR-expression
 *      logical-AND-expression && inclusive-OR-expression
 */
class LogAndExpr final : public Node {
private:
  BitOrExpr bitOrExpr_;
  std::vector<BitOrExpr> optionalBitOrExps_;

public:
  LogAndExpr(TokIter begin, BitOrExpr bitOrExpr,
             std::vector<BitOrExpr> optionalBitOrExps)
      : Node(begin), bitOrExpr_(bitOrExpr),
        optionalBitOrExps_(optionalBitOrExps) {}
  [[nodiscard]] const BitOrExpr &getBitOrExpression() const {
    return bitOrExpr_;
  }
  [[nodiscard]] const std::vector<BitOrExpr> &
  getOptionalBitOrExpressions() const {
    return optionalBitOrExps_;
  }
};

/**
 * logical-OR-expression:
 *      logical-AND-expression
 *      logical-OR-expression || logical-AND-expression
 */
class LogOrExpr final : public Node {
private:
  LogAndExpr logAndExpr_;
  std::vector<LogAndExpr> optionalLogAndExps;

public:
  LogOrExpr(TokIter begin, LogAndExpr logAndExpr,
            std::vector<LogAndExpr> optionalLogAndExps)
      : Node(begin), logAndExpr_(logAndExpr),
        optionalLogAndExps(optionalLogAndExps) {}
  [[nodiscard]] const LogAndExpr &getAndExpression() const {
    return logAndExpr_;
  }
  [[nodiscard]] const std::vector<LogAndExpr> &
  getOptionalAndExpressions() const {
    return optionalLogAndExps;
  }
};

/**
 * conditional-expression:
 *      logical-OR-expression
 *      logical-OR-expression ? expression : conditional-expression
 */
class ConditionalExpr final : public Node {
private:
  LogOrExpr logOrExpr_;
  std::optional<box<Expr>> optionalExpr_;
  std::optional<box<ConditionalExpr>> optionalCondExpr_;

public:
  explicit ConditionalExpr(
      TokIter begin, LogOrExpr logOrExpr,
      std::optional<box<Expr>> optionalExpr = {std::nullopt},
      std::optional<box<ConditionalExpr>> optionalCondExpr = {std::nullopt})
      : Node(begin), logOrExpr_(logOrExpr), optionalExpr_(optionalExpr),
        optionalCondExpr_(optionalCondExpr) {}
  [[nodiscard]] const LogOrExpr &getLogicalOrExpression() const {
    return logOrExpr_;
  }
  [[nodiscard]] const Expr *getOptionalExpression() const {
    if (optionalExpr_.has_value()) {
      return optionalExpr_.value().get();
    }
    return nullptr;
  }
  [[nodiscard]] const ConditionalExpr *
  getOptionalConditionalExpression() const {
    if (optionalCondExpr_.has_value()) {
      return optionalCondExpr_.value().get();
    }
    return nullptr;
  }
};

/**
 * assignment-expression:
 *      conditional-expression
 *      unary-expression assignment-operator assignment-expression
 *
 * assignment-operator: one of
 *      =  *=  /=  %=  +=  -=  <<=  >>=  &=  ^=  |=
 *
 * Instead we are doing something similar to clang here though:
 * We'll be using the grammar of the form:
 *
 * assignment-expression:
 *      conditional-expression
 *      conditional-expression assignment-operator assignment-expression
 */
class AssignExpr final : public Node {
public:
  enum AssignmentOperator {
    Assign,
    PlusAssign,
    MinusAssign,
    MultiplyAssign,
    DivideAssign,
    ModuloAssign,
    LeftShiftAssign,
    RightShiftAssign,
    BitAndAssign,
    BitOrAssign,
    BitXorAssign
  };

private:
  ConditionalExpr condExpr_;
  std::vector<std::pair<AssignmentOperator, ConditionalExpr>>
      optionalConditionExpr_;

public:
  AssignExpr(TokIter begin, ConditionalExpr conditionalExpression,
             std::vector<std::pair<AssignmentOperator, ConditionalExpr>>
                 optionalConditionExpr)
      : Node(begin), condExpr_(conditionalExpression),
        optionalConditionExpr_(optionalConditionExpr) {}

  [[nodiscard]] const ConditionalExpr &getConditionalExpr() const {
    return condExpr_;
  }
  [[nodiscard]] const std::vector<
      std::pair<AssignmentOperator, ConditionalExpr>> &
  getOptionalConditionalExpr() const {
    return optionalConditionExpr_;
  }
};

/**
 expression:
    assignment-expression
    expression , assignment-expression
 */
class Expr final : public Node {
private:
  std::vector<AssignExpr> assignExpressions_;

public:
  Expr(TokIter begin, std::vector<AssignExpr> assignExpressions)
      : Node(begin), assignExpressions_(assignExpressions) {}

  const std::vector<AssignExpr> &getAssignExpressions() const {
    return assignExpressions_;
  }
};

using Stmt =
    std::variant<box<ReturnStmt>, box<ExprStmt>, box<IfStmt>, box<BlockStmt>,
                 box<ForStmt>, box<WhileStmt>, box<DoWhileStmt>, box<BreakStmt>,
                 box<ContinueStmt>, box<SwitchStmt>, box<DefaultStmt>,
                 box<CaseStmt>, box<GotoStmt>, box<LabelStmt>>;

/**
 * expression-statement:
 *      expression{opt} ;
 */
class ExprStmt final : public Node {
private:
  std::optional<box<Expr>> optionalExpr_;

public:
  ExprStmt(TokIter begin,
           std::optional<box<Expr>> optionalExpr = {std::nullopt});
  [[nodiscard]] const Expr *getOptionalExpression() const {
    if (optionalExpr_.has_value()) {
      return optionalExpr_.value().get();
    }
    return nullptr;
  }
};

/**
 * if-statement:
 *      if ( expression ) statement
 *      if ( expression ) statement else statement
 */
class IfStmt final : public Node {
private:
  Expr expr_;
  Stmt thenStmt_;
  std::optional<Stmt> optionalElseStmt_;

public:
  IfStmt(TokIter begin, Expr expr, Stmt thenStmt,
         std::optional<Stmt> optionalElseStmt = {std::nullopt})
      : Node(begin), expr_(expr), thenStmt_(thenStmt),
        optionalElseStmt_(optionalElseStmt) {}

  [[nodiscard]] const Expr &getExpression() const { return expr_; }

  [[nodiscard]] const Stmt &getThenStmt() const { return thenStmt_; }

  [[nodiscard]] const Stmt *getElseStmt() const {
    if (optionalElseStmt_.has_value()) {
      return &optionalElseStmt_.value();
    }
    return nullptr;
  }
};

/**
 * switch-statement:
 *      switch ( expression ) statement
 */
class SwitchStmt final : public Node {
private:
  Expr expr_;
  Stmt stmt_;

public:
  SwitchStmt(TokIter begin, Expr expression, Stmt statement)
      : Node(begin), expr_(expression), stmt_(statement) {}

  [[nodiscard]] const Expr &getExpression() const { return expr_; }

  [[nodiscard]] const Stmt &getStatement() const { return stmt_; }
};

/**
 * default-statement:
 *      default : statement
 */
class DefaultStmt final : public Node {
private:
  Stmt stmt_;

public:
  DefaultStmt(TokIter begin, Stmt statement) : Node(begin), stmt_(statement) {}
  [[nodiscard]] const Stmt &getStatement() const { return stmt_; }
};

/**
 * case-statement:
 *      case constant-expression : statement
 */
class CaseStmt final : public Node {
private:
  ConstantExpr constantExpr_;
  Stmt stmt_;

public:
  CaseStmt(TokIter begin, ConstantExpr constantExpr, Stmt stmt)
      : Node(begin), constantExpr_(constantExpr), stmt_(stmt) {}

  [[nodiscard]] const ConstantExpr &getConstantExpr() const {
    return constantExpr_;
  }
  [[nodiscard]] const Stmt &getStatement() const { return stmt_; }
};

/**
 * label-statement:
 *      identifier : statement
 */
class LabelStmt final : public Node {
private:
  std::string_view mIdentifier;

public:
  LabelStmt(TokIter begin, std::string_view identifier)
      : Node(begin), mIdentifier(identifier) {}
  [[nodiscard]] std::string_view getIdentifier() const{ return mIdentifier; }
};

/**
 * goto-statement:
 *      goto identifier ;
 */
class GotoStmt final : public Node {
private:
  std::string_view mIdentifier;

public:
  GotoStmt(TokIter begin, std::string_view identifier)
      : Node(begin), mIdentifier(identifier) {}
  [[nodiscard]] std::string_view getIdentifier() const { return mIdentifier; }
};

/**
 * do-while-statement:
 *      do statement while ( expression ) ;
 */
class DoWhileStmt final : public Node {
private:
  Stmt stmt_;
  Expr expr_;

public:
  DoWhileStmt(TokIter begin, Stmt stmt, Expr expr)
      : Node(begin), stmt_(stmt), expr_(expr) {}
  [[nodiscard]] const Stmt &getStatement() const { return stmt_; }
  [[nodiscard]] const Expr &getExpression() const { return expr_; }
};

/**
 * while-statement:
 *      while ( expression ) statement
 */
class WhileStmt final : public Node {
private:
  Expr expr_;
  Stmt stmt_;

public:
  WhileStmt(TokIter begin, Expr expr, Stmt stmt)
      : Node(begin), expr_(expr), stmt_(stmt) {}
  [[nodiscard]] const Expr &getExpression() const { return expr_; }
  [[nodiscard]] const Stmt &getStatement() const { return stmt_; }
};

/**
 * for-statement:
 *      for ( expression{opt} ; expression{opt} ; expression{opt} ) statement
 *      for ( declaration expression{opt} ; expression{opt} ) statement
 */
class ForStmt final : public Node {
private:
  std::variant<box<Declaration>, std::optional<Expr>> initial_;
  std::optional<Expr> controlExpr_;
  std::optional<Expr> postExpr_;
  Stmt stmt_;

public:
  ForStmt(TokIter begin, Stmt stmt,
          std::variant<box<Declaration>, std::optional<Expr>> initial,
          std::optional<Expr> controlExpr = {std::nullopt},
          std::optional<Expr> postExpr = {std::nullopt});
  [[nodiscard]] const Stmt &getStatement() const { return stmt_; }

  [[nodiscard]] const std::variant<box<Declaration>, std::optional<Expr>> &
  getInitial() const {
    return initial_;
  }
  [[nodiscard]] const Expr *getControlling() const {
    if (controlExpr_) {
      return &controlExpr_.value();
    }
    return nullptr;
  }
  [[nodiscard]] const Expr *getPost() const {
    if (postExpr_) {
      return &postExpr_.value();
    }
    return nullptr;
  }
};

/**
 * break-statement:
 *      break ;
 */
class BreakStmt final : public Node {
public:
  BreakStmt(TokIter begin) : Node(begin) {}
};

/**
 * continue-statement:
 *      continue ;
 */
class ContinueStmt final : public Node {
public:
  ContinueStmt(TokIter begin) : Node(begin) {}
};

/**
 * return-statement:
 *      return expr{opt} ;
 */
class ReturnStmt final : public Node {
private:
  std::optional<Expr> optionalExpr_;

public:
  ReturnStmt(TokIter begin, std::optional<Expr> optionalExpr = {std::nullopt})
      : Node(begin), optionalExpr_(optionalExpr) {}
  [[nodiscard]] const Expr *getExpression() const {
    if (optionalExpr_) {
      return &optionalExpr_.value();
    }
    return nullptr;
  }
};

/**
 * initializer:
 *  assignment-expression
 *  { initializer-list }
 *  { initializer-list , }
 */
class Initializer final : public Node {
  using variant = std::variant<AssignExpr, box<InitializerList>>;
  variant variant_;

public:
  Initializer(TokIter begin, variant variant)
      : Node(begin), variant_(variant) {}

  [[nodiscard]] const variant &getVariant() const { return variant_; }
};

/**
 * initializer-list:
 *  designation{opt} initializer
 *  initializer-list , designation{opt} initializer
 *
 *  designation:
 *      designator-list =
 *
 *  designator-list:
 *      designator
 *      designator-list designator
 *
 *  designator:
 *      [ constant-expression ]
 *      . identifier
 *
 *  eg:
 *      struct { int a[3], b; } w[] = { [0].a = {1}, [1].a[0] = 2 };
 *  [0].a  meaning designator designator
 */
class InitializerList final : public Node {
public:
  using Identifier = std::string_view;
  using Designator = std::variant<ConstantExpr, Identifier>;
  using DesignatorList = std::vector<Designator>;
  using Designation = DesignatorList;
  using InitializerPair = std::pair<std::optional<Designation>, Initializer>;

private:
  std::vector<InitializerPair> initializerPairs_;

public:
  InitializerList(TokIter begin, std::vector<InitializerPair> initializerPairs)
      : Node(begin), initializerPairs_(initializerPairs) {}

  const std::vector<InitializerPair> &getInitializerList() const {
    return initializerPairs_;
  }
};

/**
 * declaration:
 *      declaration-specifiers init-declarator-list{opt} ;
 *
 *  init-declarator-list:
 *      init-declarator
 *      init-declarator-list , init-declarator
 *
 *  init-declarator:
 *      declarator
 *      declarator = initializer
 */
class Declaration final : public Node {
public:
  struct InitDeclarator {
    TokIter beginLoc_;
    box<Declarator> declarator_;
    std::optional<Initializer> optionalInitializer_;
  };

private:
  DeclarationSpecifiers declarationSpecifiers_;
  std::vector<InitDeclarator> initDeclarators_;

public:
  Declaration(TokIter begin, DeclarationSpecifiers declarationSpecifiers,
              std::vector<InitDeclarator> initDeclarators)
      : Node(begin), declarationSpecifiers_(declarationSpecifiers),
        initDeclarators_(initDeclarators) {}
  [[nodiscard]] const DeclarationSpecifiers &getDeclarationSpecifiers() const {
    return declarationSpecifiers_;
  }
  [[nodiscard]] const std::vector<InitDeclarator> &getInitDeclarators() const {
    return initDeclarators_;
  }
};

using BlockItem = std::variant<Stmt, Declaration>;

/**
 * compound-statement:
 *      { block-item-list{opt} }
 * block-item-list:
 *      block-item
 *      block-item-list block-item
 * block-item:
 *      declaration
 *      statement
 */
class BlockStmt final : public Node {
private:
  std::vector<BlockItem> blockItems_;

public:
  BlockStmt(TokIter begin, std::vector<BlockItem> blockItems)
      : Node(begin), blockItems_(blockItems) {}
  [[nodiscard]] const std::vector<BlockItem> &getBlockItems() const {
    return blockItems_;
  }
};

/**
 * direct-abstract-declarator:
 *      ( abstract-declarator )
 *      direct-abstract-declarator{opt} [ type-qualifier-list{opt}
 * assignment-expression{opt} ] direct-abstract-declarator{opt} [ static
 * type-qualifier-list{opt} assignment-expression ]
 *      direct-abstract-declarator{opt} [ type-qualifier-list static
 * assignment-expression ] direct-abstract-declarator{opt} [*]
 *      direct-abstract-declarator{opt} ( parameter-type-list{opt} )
 */
using DirectAbstractDeclarator =
    std::variant<box<DirectAbstractDeclaratorParentheses>,
                 box<DirectAbstractDeclaratorAssignExpr>,
                 box<DirectAbstractDeclaratorAsterisk>,
                 box<DirectAbstractDeclaratorParamTypeList>>;

/**
 * direct-declarator:
 *   identifier
 *   ( declarator )
 *   direct-declarator [ type-qualifier-list{opt} assignment-expression{opt} ]
 *   direct-declarator [ static type-qualifier-list{opt} assignment-expression ]
 *   direct-declarator [ type-qualifier-list static assignment-expression ]
 *   direct-declarator [ type-qualifier-list{opt} * ]
 *   direct-declarator ( parameter-type-list )
 *   direct-declarator ( identifier-list{opt} )
 */
using DirectDeclarator =
    std::variant<box<DirectDeclaratorIdent>, box<DirectDeclaratorParentheses>,
                 box<DirectDeclaratorAssignExpr>, box<DirectDeclaratorAsterisk>,
                 box<DirectDeclaratorParamTypeList>>;

/**
 * pointer:
 *  type-qualifier-list{opt}
 *  type-qualifier-list{opt} pointer
 */
class Pointer final : public Node {
  std::vector<TypeQualifier> typeQualifiers_;

public:
  Pointer(TokIter begin, std::vector<TypeQualifier> typeQualifiers)
      : Node(begin), typeQualifiers_(typeQualifiers) {}

  [[nodiscard]] const std::vector<TypeQualifier> &getTypeQualifiers() const {
    return typeQualifiers_;
  }
};

/**
 * abstract-declarator:
 *  pointer
 *  pointer{opt} direct-abstract-declarator
 */
class AbstractDeclarator final : public Node {
  std::vector<Pointer> pointers_;
  std::optional<DirectAbstractDeclarator> directAbstractDeclarator_;

public:
  AbstractDeclarator(TokIter begin, std::vector<Pointer> pointers,
                     std::optional<DirectAbstractDeclarator>
                         directAbstractDeclarator = {std::nullopt})
      : Node(begin), pointers_(pointers),
        directAbstractDeclarator_(directAbstractDeclarator) {}

  [[nodiscard]] const std::vector<Pointer> &getPointers() const {
    return pointers_;
  }

  [[nodiscard]] const DirectAbstractDeclarator *
  getDirectAbstractDeclarator() const {
    if (directAbstractDeclarator_) {
      return &directAbstractDeclarator_.value();
    }
    return nullptr;
  }
};

/**
 * declarator:
 *  pointer{opt} direct-declarator
 */
class Declarator final : public Node {
  std::vector<Pointer> pointers_;
  DirectDeclarator directDeclarator_;

public:
  Declarator(TokIter begin, std::vector<Pointer> pointers,
             DirectDeclarator directDeclarator)
      : Node(begin), pointers_(pointers), directDeclarator_(directDeclarator) {}

  [[nodiscard]] const std::vector<Pointer> &getPointers() const {
    return pointers_;
  }

  [[nodiscard]] const DirectDeclarator &getDirectDeclarator() const {
    return directDeclarator_;
  }
};

/**
 * parameter-declaration:
 *      declaration-specifiers declarator
 *      declaration-specifiers abstract-declarator{opt}
 */
struct ParameterDeclaration final : public Node {
public:
  DeclarationSpecifiers declarationSpecifiers_;
  using DeclaratorKind =
      std::variant<Declarator, std::optional<AbstractDeclarator>>;
  DeclaratorKind declaratorKind_;

public:
  ParameterDeclaration(
      TokIter begin, DeclarationSpecifiers declarationSpecifiers,
      std::variant<Declarator, std::optional<AbstractDeclarator>> variant =
          {std::nullopt})
      : Node(begin), declarationSpecifiers_(declarationSpecifiers),
        declaratorKind_(variant) {}
  [[nodiscard]] const DeclarationSpecifiers &getDeclarationSpecifiers() const {
    return declarationSpecifiers_;
  }
};

/**
 * parameter-list:
 *  parameter-declaration
 *  parameter-list , parameter-declaration
 */
class ParamList final : public Node {
private:
  std::vector<ParameterDeclaration> parameterList_;

public:
  ParamList(TokIter begin, std::vector<ParameterDeclaration> parameterList)
      : Node(begin), parameterList_(parameterList) {}

  [[nodiscard]] const std::vector<ParameterDeclaration> &
  getParameterDeclarations() const {
    return parameterList_;
  }
};

/**
 * parameter-type-list:
 *  parameter-list
 *  parameter-list , ...
 */
class ParamTypeList final : public Node {
  ParamList parameterList_;
  bool hasEllipse_;

public:
  ParamTypeList(TokIter begin, ParamList parameterList, bool hasEllipse)
      : Node(begin), parameterList_(parameterList), hasEllipse_(hasEllipse) {}

  [[nodiscard]] const ParamList &getParameterList() const {
    return parameterList_;
  }

  [[nodiscard]] bool hasEllipse() const { return hasEllipse_; }
};

/**
 * direct-abstract-declarator:
 *      ( abstract-declarator )
 */
class DirectAbstractDeclaratorParentheses final : public Node {
  AbstractDeclarator abstractDeclarator_;

public:
  DirectAbstractDeclaratorParentheses(TokIter begin,
                                      AbstractDeclarator abstractDeclarator)
      : Node(begin), abstractDeclarator_(abstractDeclarator) {}

  [[nodiscard]] const AbstractDeclarator &getAbstractDeclarator() const {
    return abstractDeclarator_;
  }
};

/**
 * direct-abstract-declarator:
 *      direct-abstract-declarator{opt} [ type-qualifier-list{opt}
 *                                                  assignment-expression{opt} ]
 *      direct-abstract-declarator{opt} [ static
 *                              type-qualifier-list{opt} assignment-expression ]
 *      direct-abstract-declarator{opt} [ type-qualifier-list static
 * assignment-expression ]
 */
class DirectAbstractDeclaratorAssignExpr final : public Node {
  std::optional<DirectAbstractDeclarator> optionalDirectAbstractDeclarator_;
  std::vector<TypeQualifier> typeQualifiers_;
  std::optional<AssignExpr> optionalAssignExpr_;
  bool hasStatic_{false};

public:
  DirectAbstractDeclaratorAssignExpr(
      TokIter begin, std::optional<DirectAbstractDeclarator> directAbstractDeclarator,
      std::vector<TypeQualifier> typeQualifiers, std::optional<AssignExpr> assignExpr,
      bool hasStatic)
      : Node(begin), optionalDirectAbstractDeclarator_(directAbstractDeclarator),
        typeQualifiers_(typeQualifiers), optionalAssignExpr_(assignExpr),
        hasStatic_(hasStatic) {}

  [[nodiscard]] const DirectAbstractDeclarator *
  getDirectAbstractDeclarator() const {
    if (optionalDirectAbstractDeclarator_) {
      return &optionalDirectAbstractDeclarator_.value();
    }
    return nullptr;
  }

  [[nodiscard]] const std::vector<TypeQualifier> &getTypeQualifiers() const {
    return typeQualifiers_;
  }

  [[nodiscard]] const AssignExpr *getAssignmentExpression() const {
    if (optionalAssignExpr_) {
      return &optionalAssignExpr_.value();
    }
    return nullptr;
  }

  [[nodiscard]] bool hasStatic() const { return hasStatic_; }
};

/// direct-abstract-declarator{opt} [*]
class DirectAbstractDeclaratorAsterisk final : public Node {
  std::optional<DirectAbstractDeclarator> optionalDirectAbstractDeclarator_;

public:
  DirectAbstractDeclaratorAsterisk(
      TokIter begin, std::optional<DirectAbstractDeclarator> directAbstractDeclarator)
      : Node(begin), optionalDirectAbstractDeclarator_(directAbstractDeclarator) {}

  [[nodiscard]] const DirectAbstractDeclarator * getDirectAbstractDeclarator() const {
    if (optionalDirectAbstractDeclarator_) {
      return &optionalDirectAbstractDeclarator_.value();
    }
    return nullptr;
  }
};

/**
 * direct-abstract-declarator:
 *  direct-abstract-declarator{opt} ( parameter-type-list{opt} )
 */
class DirectAbstractDeclaratorParamTypeList final : public Node {
  std::optional<DirectAbstractDeclarator> optionalDirectAbstractDeclarator_;
  std::optional<ParamTypeList> optionalParamTypeList_;

public:
  DirectAbstractDeclaratorParamTypeList(
      TokIter begin, std::optional<DirectAbstractDeclarator> directAbstractDeclarator,
      std::optional<ParamTypeList> paramTypeList)
      : Node(begin), optionalDirectAbstractDeclarator_(directAbstractDeclarator),
        optionalParamTypeList_(paramTypeList) {}

  [[nodiscard]] const DirectAbstractDeclarator *
  getDirectAbstractDeclarator() const {
    if (optionalDirectAbstractDeclarator_) {
      return &optionalDirectAbstractDeclarator_.value();
    }
    return nullptr;
  }

  [[nodiscard]] const ParamTypeList *getParameterTypeList() const {
    if (optionalParamTypeList_) {
      return &optionalParamTypeList_.value();
    }
    return nullptr;
  }
};

/**
 * direct-declarator:
 *  identifier
 */
class DirectDeclaratorIdent final : public Node {
  std::string_view mIdent;

public:
  DirectDeclaratorIdent(TokIter begin, std::string_view ident)
      : Node(begin), mIdent(ident) {}

  [[nodiscard]] const std::string_view &getIdent() const { return mIdent; }
};

/**
 * direct-declarator:
 *  ( declarator )
 */
class DirectDeclaratorParentheses final : public Node {
  Declarator declarator_;

public:
  DirectDeclaratorParentheses(TokIter begin, Declarator declarator)
      : Node(begin), declarator_(declarator) {}

  [[nodiscard]] const Declarator &getDeclarator() const { return declarator_; }
};

/**
 * direct-declarator:
 *  direct-declarator ( parameter-type-list )
 */
class DirectDeclaratorParamTypeList final : public Node {
  DirectDeclarator directDeclarator_;
  ParamTypeList paramTypeList_;

public:
  DirectDeclaratorParamTypeList(TokIter begin,
                                DirectDeclarator directDeclarator,
                                ParamTypeList paramTypeList)
      : Node(begin), directDeclarator_(directDeclarator),
        paramTypeList_(paramTypeList) {}

  [[nodiscard]] const DirectDeclarator &getDirectDeclarator() const {
    return directDeclarator_;
  }

  [[nodiscard]] const ParamTypeList &getParamTypeList() const {
    return paramTypeList_;
  }
};

/**
 * direct-declarator:
 *  direct-declarator [ type-qualifier-list{opt} assignment-expression{opt} ]
 *  direct-declarator [ static type-qualifier-list{opt} assignment-expression ]
 *  direct-declarator [ type-qualifier-list static assignment-expression ]
 */
class DirectDeclaratorAssignExpr final : public Node {
  DirectDeclarator directDeclarator_;
  std::optional<AssignExpr> optionalAssignExpr_;
  std::vector<TypeQualifier> typeQualifierList_;
  bool hasStatic_{false};

public:
  DirectDeclaratorAssignExpr(
      TokIter begin, DirectDeclarator directDeclarator,
      std::vector<TypeQualifier> typeQualifierList,
      std::optional<AssignExpr> assignExpr = {std::nullopt},
      bool hasStatic = false)
      : Node(begin), directDeclarator_(directDeclarator),
        optionalAssignExpr_(assignExpr), typeQualifierList_(typeQualifierList),
        hasStatic_(hasStatic) {}

  [[nodiscard]] const DirectDeclarator &getDirectDeclarator() const {
    return directDeclarator_;
  }

  [[nodiscard]] const std::vector<TypeQualifier> &getTypeQualifierList() const {
    return typeQualifierList_;
  }

  [[nodiscard]] const AssignExpr *getAssignmentExpression() const {
    if (optionalAssignExpr_) {
      return &optionalAssignExpr_.value();
    }
    return nullptr;
  }

  [[nodiscard]] bool hasStatic() const { return hasStatic_; }
};

/**
 * direct-declarator:
 *   direct-declarator [ type-qualifier-list{opt} * ]
 */
class DirectDeclaratorAsterisk final : public Node {
  DirectDeclarator directDeclarator_;
  std::vector<TypeQualifier> typeQualifierList_;

public:
  DirectDeclaratorAsterisk(TokIter begin, DirectDeclarator directDeclarator,
                           std::vector<TypeQualifier> typeQualifierList)
      : Node(begin), directDeclarator_(directDeclarator),
        typeQualifierList_(typeQualifierList) {}

  [[nodiscard]] const DirectDeclarator &getDirectDeclarator() const {
    return directDeclarator_;
  }

  [[nodiscard]] const std::vector<TypeQualifier> &getTypeQualifierList() const {
    return typeQualifierList_;
  }
};

/**
 * struct-or-union-specifier:
 *  struct-or-union identifier{opt} { struct-declaration-list }
 *  struct-or-union identifier
 *
 *  struct-declaration-list:
 *    struct-declaration
 *    struct-declaration-list struct-declaration
 *
 *  struct-declaration:
 *    specifier-qualifier-list struct-declarator-list ;
 *
 *  struct-declarator-list:
 *      struct-declarator
 *      struct-declarator-list , struct-declarator
 *
 *  struct-declarator:
 *      declarator
 *      declarator{opt} : constant-expression
 */
class StructOrUnionSpecifier final : public Node {
public:
  struct StructDeclarator {
    TokIter beginLoc_;
    std::optional<Declarator> optionalDeclarator_;
    std::optional<ConstantExpr> optionalBitfield_;
  };
  struct StructDeclaration {
    TokIter beginLoc_;
    DeclarationSpecifiers specifierQualifiers_;
    std::vector<StructDeclarator> structDeclarators_;
  };

private:
  std::string_view name_;
  bool isUnion_;
  std::vector<StructDeclaration> structDeclarations_;

public:
  StructOrUnionSpecifier(TokIter begin, bool isUnion,
                         std::string_view identifier,
                         std::vector<StructDeclaration> structDeclarations)
      : Node(begin), isUnion_(isUnion), name_(identifier),
        structDeclarations_(structDeclarations) {}

  [[nodiscard]] bool isUnion() const { return isUnion_; }

  [[nodiscard]] std::string_view getTag() const { return name_; }

  [[nodiscard]] const std::vector<StructDeclaration> &
  getStructDeclarations() const {
    return structDeclarations_;
  }
};

/**
 * enum-specifier:
 *  enum identifier{opt} { enumerator-list }
 *  enum identifier{opt} { enumerator-list , }
 *  enum identifier
 *
 * enumerator-list:
 *  enumerator
 *  enumerator-list , enumerator
 *
 * enumerator:
 *  enumeration-constant
 *  enumeration-constant = constant-expression
 *
 * enumeration-constant:
 *  identifier
 */
class EnumSpecifier final : public Node {
public:
  struct Enumerator {
    std::string_view name_;
    std::optional<ConstantExpr> optionalConstantExpr_;
  };

private:
  std::string_view tagName_;
  std::vector<Enumerator> enumerators_;

public:
  EnumSpecifier(TokIter begin, std::string_view tagName,
                std::vector<Enumerator> enumerators)
      : Node(begin), tagName_(tagName), enumerators_(enumerators) {}

  [[nodiscard]] std::string_view getName() const { return tagName_; }
  [[nodiscard]] const std::vector<Enumerator> &getEnumerators() const {
    return enumerators_;
  }
};

/**
 * function-definition:
 *  declaration-specifiers declarator declaration-list{opt} compound-statement
 */
class FunctionDefinition final : public Node {
  DeclarationSpecifiers declarationSpecifiers_;
  Declarator declarator_;
  BlockStmt compoundStmt_;

public:
  FunctionDefinition(TokIter begin, DeclarationSpecifiers declarationSpecifiers,
                     Declarator declarator, BlockStmt compoundStmt)
      : Node(begin), declarationSpecifiers_(declarationSpecifiers),
        declarator_(declarator), compoundStmt_(compoundStmt) {}

  [[nodiscard]] const DeclarationSpecifiers &getDeclarationSpecifiers() const {
    return declarationSpecifiers_;
  }

  [[nodiscard]] const Declarator &getDeclarator() const { return declarator_; }

  [[nodiscard]] const BlockStmt &getCompoundStatement() const {
    return compoundStmt_;
  }
};

/**
 * external-declaration:
 *  function-definition
 *  declaration
 */
using ExternalDeclaration = std::variant<Declaration, FunctionDefinition>;

/**
 * translation-unit:
 *  external-declaration
 *  translation-unit external-declaration
 */
class TranslationUnit final {
  std::vector<ExternalDeclaration> mGlobals;

public:
  explicit TranslationUnit(TokIter begin,
                           std::vector<ExternalDeclaration> globals) noexcept
      : mGlobals(globals) {}

  [[nodiscard]] const std::vector<ExternalDeclaration> &getGlobals() const {
    return mGlobals;
  }
};
} // namespace lcc::Syntax

#endif // LCC_SYNTAX_H