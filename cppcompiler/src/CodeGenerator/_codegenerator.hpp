#pragma once
#include "../Parser/_parser.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <memory>
namespace CodeGenerator {
// llvm::Value* codegen();
struct LlvmState {
	std::unique_ptr<llvm::LLVMContext> _context;
	std::unique_ptr<llvm::Module>      _module;
};
LlvmState run(const Parser::AST& ast);
} // namespace CodeGenerator
