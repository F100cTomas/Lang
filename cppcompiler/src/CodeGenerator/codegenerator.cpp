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
#include <vector>
namespace CodeGenerator {
using namespace llvm;
using std::unique_ptr, std::make_unique;
#define I64_t        Type::getInt64Ty(context)
#define I64_val(val) ConstantInt::get(I64_t, val)
namespace {
// only works with x64 for now
void add_start_symbol(LLVMContext& context, Module& module, Function* main_fn) {
	FunctionType* start_type = FunctionType::get(Type::getVoidTy(context), false);
	Function*     start      = Function::Create(start_type, GlobalValue::ExternalLinkage, "_start", module);
	start->setDoesNotReturn();
	start->setCallingConv(CallingConv::C);
	BasicBlock*   entry = BasicBlock::Create(context, "", start);
	IRBuilder<>   builder{entry};
	Value*        main_ret_val = builder.CreateCall(main_fn, {ConstantInt::get(Type::getInt64Ty(context), 0)});
	FunctionType* exit_type    = FunctionType::get(Type::getVoidTy(context), {Type::getInt64Ty(context)}, false);
	InlineAsm*    exit         = InlineAsm::get(exit_type, "mov $0, %rdi; mov $$60, %rax; syscall", "r", true);
	builder.CreateCall(exit, {main_ret_val});
	builder.CreateUnreachable();
}
} // namespace
Function* function_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module) {
	if (node->_metadata == nullptr)
		ERROR("Unexpected nullptr.");
	Parser::FnMeta&          meta = *reinterpret_cast<Parser::FnMeta*>(node->_metadata);
	// one argument for now
	/*
	std::vector<llvm::Type*> args{};
	args.reserve(meta._args.size());
	for (size_t i = 0; i < meta._args.size(); i++)
		args.emplace_back(I64_t);
	*/
	FunctionType* fn_type  = FunctionType::get(I64_t, {I64_t}, false);
	Function*     fn       = Function::Create(fn_type, llvm::GlobalValue::InternalLinkage, meta._name.get(), module);
	BasicBlock*   fn_entry = BasicBlock::Create(context, "", fn);
	IRBuilder<>   fn_builder{fn_entry};
	if (node->_args.size() != 1)
		ERROR("Internal error.");
	ExpressionResult res = codegen(node->_args.front(), context, module, &fn_builder);
	if (res.index() != 0) {
		fn_builder.CreateRet(I64_val(0));
		return fn;
	}
	fn_builder.CreateRet(std::get<Value*>(res));
	return fn;
}
ExpressionResult codegen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                         llvm::IRBuilder<>* builder) {
	if (node == nullptr)
		ERROR("Unexpected nullptr.");
	if (node->_name == "fn")
		return function_gen(node, context, module);
	if (*node->_name.get() >= '0' && *node->_name.get() <= '9')
		return I64_val(atoi(node->_name));
	return I64_val(0);
}
LlvmState run(const Parser::AST& ast) {
	LlvmState state;
	state._context           = make_unique<LLVMContext>();
	state._module            = make_unique<Module>("Program", *state._context);
	LLVMContext&     context = *state._context;
	Module&          module  = *state._module;
	ExpressionResult res     = codegen(ast.get_symbol_table().get("main"), context, module, nullptr);
	if (res.index() != 1)
		ERROR("main is not a function");
	Function* main = std::get<Function*>(res);
	add_start_symbol(context, module, main);
	return state;
}
} // namespace CodeGenerator
