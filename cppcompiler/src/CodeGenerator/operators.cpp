#include "../error.hpp"
#include "_codegenerator.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <memory>
namespace CodeGenerator {
using namespace llvm;
using std::unique_ptr, std::make_unique;
#define I64_t        Type::getInt64Ty(context)
#define I64_val(val) ConstantInt::get(I64_t, val)
namespace Operators {
namespace {
constexpr uint64_t hashfn(const char* str) {
	uint64_t hash = 14695981039346656037U;
	while (*str) {
		hash ^= static_cast<uint8_t>(*str++);
		hash *= 1099511628211U;
	}
	return hash;
}
} // namespace
ExpressionResult operator_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                              llvm::IRBuilder<>& builder) {
	if (node->_args.size() == 1)
		switch (hashfn(node->_name)) {
		case hashfn("+"): return u_plus_gen(node, context, module, builder);
		case hashfn("-"): return u_minus_gen(node, context, module, builder);
		default: {
			ERROR("Operator not implemented:", node->_name.get());
			return I64_val(0);
		}
		}
	switch (hashfn(node->_name)) {
	case hashfn(""): return blank_gen(node, context, module, builder);
	case hashfn("*"): return mul_gen(node, context, module, builder);
	case hashfn("/"): return div_gen(node, context, module, builder);
	case hashfn("%"): return mod_gen(node, context, module, builder);
	case hashfn("+"): return add_gen(node, context, module, builder);
	case hashfn("-"): return sub_gen(node, context, module, builder);
	default: break;
	}
	ERROR("Operator not implemented.");
	return I64_val(0);
}
ExpressionResult blank_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                           llvm::IRBuilder<>& builder) {
	ExpressionResult left  = codegen(node->_args[0], context, module, &builder);
	ExpressionResult right = codegen(node->_args[1], context, module, &builder);
	if (left.index() != 1 || right.index() != 0)
		ERROR("Invalid operands for function call operator.");
	return builder.CreateCall(std::get<Function*>(left), {std::get<Value*>(right)});
}
ExpressionResult mul_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                         llvm::IRBuilder<>& builder) {
	ExpressionResult left  = codegen(node->_args[0], context, module, &builder);
	ExpressionResult right = codegen(node->_args[1], context, module, &builder);
	if (left.index() != 0 || right.index() != 0)
		ERROR("Invalid operands for '*' operator.");
	return builder.CreateMul(std::get<Value*>(left), std::get<Value*>(right));
}
ExpressionResult div_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                         llvm::IRBuilder<>& builder) {
	ExpressionResult left  = codegen(node->_args[0], context, module, &builder);
	ExpressionResult right = codegen(node->_args[1], context, module, &builder);
	if (left.index() != 0 || right.index() != 0)
		ERROR("Invalid operands for '/' operator.");
	return builder.CreateSDiv(std::get<Value*>(left), std::get<Value*>(right));
}
ExpressionResult mod_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                         llvm::IRBuilder<>& builder) {
	ExpressionResult left  = codegen(node->_args[0], context, module, &builder);
	ExpressionResult right = codegen(node->_args[1], context, module, &builder);
	if (left.index() != 0 || right.index() != 0)
		ERROR("Invalid operands for '%' operator.");
	return builder.CreateSRem(std::get<Value*>(left), std::get<Value*>(right));
}
ExpressionResult add_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                         llvm::IRBuilder<>& builder) {
	ExpressionResult left  = codegen(node->_args[0], context, module, &builder);
	ExpressionResult right = codegen(node->_args[1], context, module, &builder);
	if (left.index() != 0 || right.index() != 0)
		ERROR("Invalid operands for '+' operator.");
	return builder.CreateAdd(std::get<Value*>(left), std::get<Value*>(right));
}
ExpressionResult sub_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                         llvm::IRBuilder<>& builder) {
	ExpressionResult left  = codegen(node->_args[0], context, module, &builder);
	ExpressionResult right = codegen(node->_args[1], context, module, &builder);
	if (left.index() != 0 || right.index() != 0)
		ERROR("Invalid operands for '-' operator.");
	return builder.CreateSub(std::get<Value*>(left), std::get<Value*>(right));
}
ExpressionResult u_plus_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                            llvm::IRBuilder<>& builder) {
	return codegen(node->_args.front(), context, module, &builder);
}
ExpressionResult u_minus_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                             llvm::IRBuilder<>& builder) {
	ExpressionResult arg = codegen(node->_args.front(), context, module, &builder);
	if (arg.index() != 0)
		ERROR("Invalid operands for '-' operator.");
	return builder.CreateNeg(std::get<Value*>(arg));
}
} // namespace Operators
} // namespace CodeGenerator
