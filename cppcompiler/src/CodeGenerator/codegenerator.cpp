#include "_codegenerator.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <memory>
namespace CodeGenerator {
using namespace llvm;
using std::unique_ptr, std::make_unique;
LlvmState run(const Parser::AST& ast) {
	LlvmState state;
	state._context          = make_unique<LLVMContext>();
	state._module           = make_unique<Module>("Program", *state._context);
	LLVMContext&  context   = *state._context;
	Module&       module    = *state._module;
	FunctionType* main_type = FunctionType::get(Type::getInt32Ty(context), false);
	Function*     main      = Function::Create(main_type, GlobalValue::ExternalLinkage, "main", module);
	BasicBlock*   entry     = BasicBlock::Create(context, "", main);
	IRBuilder<>   builder{entry};
	builder.CreateRet(ConstantInt::get(Type::getInt32Ty(context), 42));
	return state;
}
} // namespace CodeGenerator
