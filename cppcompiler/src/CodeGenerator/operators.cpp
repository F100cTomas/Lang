#include "_codegenerator.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/InstrTypes.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
namespace CodeGenerator {
using namespace llvm;
Value* opgen(IRBuilder<>& builder, char op, LLVMNode* left, LLVMNode* right, LLVMState& state) {
	switch (op) {
	case '\0': return builder.CreateCall(left->get_function(), {right->get_value()});
	case '*': return builder.CreateMul(left->get_value(), right->get_value());
	case '/': return builder.CreateSDiv(left->get_value(), right->get_value());
	case '%': return builder.CreateSRem(left->get_value(), right->get_value());
	case '+':
		if (right == nullptr)
			return left->get_value();
		return builder.CreateAdd(left->get_value(), right->get_value());
	case '-':
		if (right == nullptr)
			return builder.CreateNeg(left->get_value());
		return builder.CreateSub(left->get_value(), right->get_value());
	case '=': return builder.CreateStore(right->get_value(), left->get_variable());
	case '>':
		return builder.CreateZExt(builder.CreateICmpSGT(left->get_value(), right->get_value()),
		                          Type::getInt64Ty(state.context()));
	case '<':
		return builder.CreateZExt(builder.CreateICmpSLT(left->get_value(), right->get_value()),
		                          Type::getInt64Ty(state.context()));
	default: break;
	}
	ERROR("Unknown operation.");
	return nullptr;
}
Value* opgen2(IRBuilder<>& builder, char op, LLVMNode* left, LLVMNode* right, LLVMState& state) {
	ICmpInst::Predicate p;
	switch (op) {
	case '=': p = ICmpInst::ICMP_EQ; break;
	case '!': p = ICmpInst::ICMP_NE; break;
	case '>': p = ICmpInst::ICMP_SGE; break;
	case '<': p = ICmpInst::ICMP_SLE; break;
	default: ERROR("Unknown operation."); break;
	}
	return builder.CreateZExt(builder.CreateICmp(p, left->get_value(), right->get_value()),
	                          Type::getInt64Ty(state.context()));
}
} // namespace CodeGenerator
