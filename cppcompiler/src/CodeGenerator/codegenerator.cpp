#include "_codegenerator.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <memory>

namespace CodeGenerator {
	using namespace llvm;
	using std::unique_ptr, std::make_unique;
	unique_ptr<Module> run(const Parser::AST& ast) {
		unique_ptr<LLVMContext> context = make_unique<LLVMContext>();
		unique_ptr<Module> module = make_unique<Module>("Ferro Program", *context);
		unique_ptr<IRBuilder<>> builder = make_unique<IRBuilder<>>(*context);
		return {};
	}
}
