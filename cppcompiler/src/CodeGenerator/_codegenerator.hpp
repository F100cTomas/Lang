#pragma once
#include "../Parser/_parser.hpp"
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
#include <variant>
namespace CodeGenerator {
using ExpressionResult = std::variant<llvm::Value*, llvm::Function*>;
llvm::Function*  function_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module);
ExpressionResult codegen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                         llvm::IRBuilder<>* builder);
struct LlvmState {
	std::unique_ptr<llvm::LLVMContext> _context;
	std::unique_ptr<llvm::Module>      _module;
};
LlvmState run(const Parser::AST& ast);
} // namespace CodeGenerator
