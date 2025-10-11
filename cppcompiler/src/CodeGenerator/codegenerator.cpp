#include "_codegenerator.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
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
	FunctionType* main_type = FunctionType::get(Type::getVoidTy(context), false);
	Function*     main      = Function::Create(main_type, GlobalValue::ExternalLinkage, "_start", module);
	main->setDoesNotReturn();
	main->setCallingConv(CallingConv::C);
	BasicBlock*   entry     = BasicBlock::Create(context, "", main);
	IRBuilder<>   builder{entry};
	FunctionType* exit_type = FunctionType::get(Type::getVoidTy(context), {Type::getInt64Ty(context)}, false);
	InlineAsm*    exit      = InlineAsm::get(exit_type, "mov $0, %rdi; mov $$60, %rax; syscall", "r", true);
	builder.CreateCall(exit, {ConstantInt::get(Type::getInt64Ty(context), 42)});
	builder.CreateUnreachable();
	return state;
}
} // namespace CodeGenerator
