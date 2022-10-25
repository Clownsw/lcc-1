/***********************************
 * File:     CodeGen.cc
 *
 * Author:   caipeng
 *
 * Email:    iiicp@outlook.com
 *
 * Date:     2022/10/21
 *
 * Sign:     enjoy life
 ***********************************/
#include "CodeGen.h"
namespace lcc::parser{
bool PrimaryType::IsSigned() const {
  return mSign;
}
bool PrimaryType::IsVoid() const {
  return mVoid;
}
LLVMTypePtr PrimaryType::TypeGen(CodeGenContext &context) {
  int size = mTypes.size();
  assert(size>0);
  int cur = 0;
  if (mTypes[cur] == lexer::TokenType::kw_const) {
    cur++;
  }
  assert(cur < size);
  if (mTypes[cur] == lexer::TokenType::kw_signed) {
    mSign = true;
    cur++;
  }else if (mTypes[cur] == lexer::TokenType::kw_unsigned) {
    mSign = false;
    cur++;
  }
  if (cur == size) {
    return context.mIrBuilder.getInt32Ty();
  }
  switch (mTypes[cur]) {
  case lexer::TokenType::kw_void:
    return context.mIrBuilder.getVoidTy();
  case lexer::TokenType::kw_char:
    return context.mIrBuilder.getInt8Ty();
  case lexer::TokenType::kw_short:
    return context.mIrBuilder.getInt16Ty();
  case lexer::TokenType::kw_int:
    return context.mIrBuilder.getInt32Ty();
  case lexer::TokenType::kw_long: {
    if (cur < size && mTypes[cur+1] == lexer::TokenType::kw_long) {
      ++cur;
      return context.mIrBuilder.getInt64Ty();
    } else {
      return context.mIrBuilder.getInt64Ty();
    }
  }
  case lexer::TokenType::kw_float:
    return context.mIrBuilder.getFloatTy();
  case lexer::TokenType::kw_double:
    return context.mIrBuilder.getDoubleTy();
  default:
    return nullptr;
  }
}
bool PointerType::IsSigned() const {
    return false;
}
bool PointerType::IsVoid() const {
  return false;
}
LLVMTypePtr PointerType::TypeGen(CodeGenContext &context) {
    return llvm::PointerType::getUnqual(mType->TypeGen(context));
}
LLVMValueSignPair Program::Codegen(CodeGenContext &context) const {
    context.mModule = std::make_unique<llvm::Module>("main", context.mContext);
    for (auto &ext : mExternalDecl) {
      ext->Codegen(context);
    }
    if (llvm::verifyModule(*context.mModule)) {
      context.mModule->print(llvm::errs(), nullptr);
      std::terminate();
    }else {
      context.mModule->print(llvm::outs(), nullptr);
    }
    return {nullptr, false};
}
LLVMValueSignPair GlobalDecl::Codegen(lcc::CodeGenContext &context) const {
  LLVMTypePtr type = mType->TypeGen(context);
  llvm::Value *value = nullptr;
  if (!mOptValue) {
    if (type->isIntegerTy()) {
      value = llvm::ConstantInt::get(type, 0);
    }else if (type->isFloatingPointTy()) {
      value = llvm::ConstantFP::get(type, 0);
    }
  }
  auto [constant, sign] = mOptValue ? mOptValue->Codegen(context) : LLVMValueSignPair{value, mType->IsSigned()};
  context.mModule->getOrInsertGlobal(mName, type);
  auto *globalVar = context.mModule->getGlobalVariable(mName);
  globalVar->setInitializer(llvm::cast<llvm::Constant>(constant));
  return {constant, sign};
}
LLVMValueSignPair Function::Codegen(lcc::CodeGenContext &context) const {
  LLVMTypePtr retType = mRetType->TypeGen(context);
  std::vector<LLVMTypePtr> paramType;
  std::vector<std::string> paramName;
  for (auto &p : mParam) {
    paramType.push_back(p.first->TypeGen(context));
    paramName.push_back(p.second);
  }
  auto *funType = llvm::FunctionType::get(retType, paramType, false);
  auto *func = llvm::Function::Create(funType, llvm::GlobalVariable::ExternalLinkage, mName, context.mModule.get());
  int i = -1;
  for (auto &iter : func->args()) {
    ++i;
    if (paramName[i].empty())
      continue;
    iter.setName(paramName[i]);
  }

  if (!mOptBlockStmt)
    return {func, false};

  context.mCurrentFunc = func;
  auto *entryBB = llvm::BasicBlock::Create(context.mContext, "entry", func);
  context.mIrBuilder.SetInsertPoint(entryBB);
  for (auto &iter : func->args()) {
    auto *address = context.mIrBuilder.CreateAlloca(iter.getType(), nullptr, iter.getName());
    context.mIrBuilder.CreateStore(&iter, address);
  }
  mOptBlockStmt->Codegen(context);

  auto &block = func->back();
  if (block.empty() || !block.back().isTerminator()) {
    if (retType->isVoidTy()) {
      context.mIrBuilder.CreateRetVoid();
    }else {
      assert(0);
    }
  }
  if (llvm::verifyFunction(*func, &llvm::errs())) {
    func->print(llvm::errs());
    std::terminate();
  }
  return {func, false};
}
LLVMValueSignPair BlockStmt::Codegen(lcc::CodeGenContext &context) const {
  for (auto &stmt : mStmts)
    stmt->Codegen(context);
  return {nullptr, false};
}
LLVMValueSignPair IfStmt::Codegen(lcc::CodeGenContext &context) const {
  auto[value, sign] = mExpr->Codegen(context);
//  auto [value, sign] = LLVMValueSignPair{context.mIrBuilder.getInt32(1), true};
  if (value->getType()->isIntegerTy()) {
      value = context.mIrBuilder.CreateICmpNE(value, context.mIrBuilder.getInt32(0));
  }else if (value->getType()->isFloatingPointTy()) {
    value = context.mIrBuilder.CreateFCmpUNE(value, llvm::ConstantFP::get(context.mIrBuilder.getFloatTy(), 0));
  }
  auto* function = context.mCurrentFunc;
  auto *thenBB = llvm::BasicBlock::Create(context.mContext, "", function);
  auto *elseBB =  mOptElseStmt ? llvm::BasicBlock::Create(context.mContext) : nullptr;
  auto *mergeBB = llvm::BasicBlock::Create(context.mContext);
  context.mIrBuilder.CreateCondBr(value, thenBB, elseBB?elseBB:mergeBB);
  context.mIrBuilder.SetInsertPoint(thenBB);
  mThenStmt->Codegen(context);
  if (!thenBB->back().isTerminator()) {
    context.mIrBuilder.CreateBr(mergeBB);
  }
  if (elseBB) {
    function->getBasicBlockList().push_back(elseBB);
    context.mIrBuilder.SetInsertPoint(elseBB);
    mOptElseStmt->Codegen(context);
    if (!elseBB->back().isTerminator()) {
      context.mIrBuilder.CreateBr(mergeBB);
    }
  }
  function->getBasicBlockList().push_back(mergeBB);
  context.mIrBuilder.SetInsertPoint(mergeBB);
  return {nullptr, false};
}
LLVMValueSignPair WhileStmt::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair DoWhileStmt::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair ForStmt::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair
ForDeclarationStmt::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair ExprStmt::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair ReturnStmt::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair BreakStmt::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair ContinueStmt::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair Declaration::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair Expr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair AssignExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair ConditionalExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair LogOrExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair LogAndExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair BitOrExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair BitXorExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair BitAndExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair EqualExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair RelationalExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair ShiftExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair AdditiveExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair MultiExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair CastExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair UnaryExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair PostFixExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair PrimaryExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
LLVMValueSignPair ConstantExpr::Codegen(lcc::CodeGenContext &context) const {
  return {};
}
}
namespace lcc::codegen {

}