#include "../error.hpp"
#include "_codegenerator.hpp"
#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Alignment.h>
#include <memory>
#include <vector>
namespace CodeGenerator {
using namespace llvm;
using std::unique_ptr, std::make_unique;
#define I64_t        Type::getInt64Ty(context)
#define I64_val(val) ConstantInt::get(I64_t, val)
namespace {
Function* putch_fn{nullptr};
// only works with x64 for now
void add_start_symbol(LLVMContext& context, Module& module, Function* main_fn) {
#ifdef __linux__
#define START_SYMBOL "_start"
#elif defined(__MINGW64__)
#define START_SYMBOL "mainCRTStartup"
#endif
	FunctionType* start_type = FunctionType::get(Type::getVoidTy(context), false);
	Function*     start      = Function::Create(start_type, GlobalValue::ExternalLinkage, START_SYMBOL, module);
	start->setDoesNotReturn();
	start->setCallingConv(CallingConv::C);
	BasicBlock* entry = BasicBlock::Create(context, "", start);
	IRBuilder<> builder{entry};
	Value*      main_ret_val = builder.CreateCall(main_fn, {I64_val(0)});
#ifdef __linux__
	FunctionType* exit_type = FunctionType::get(Type::getVoidTy(context), {I64_t}, false);
	InlineAsm*    exit      = InlineAsm::get(exit_type, "mov $0, %rdi; mov $$60, %rax; syscall", "r", true);
	builder.CreateCall(exit, {main_ret_val});
#elif defined(__MINGW64__)
	FunctionType* exit_type = FunctionType::get(Type::getVoidTy(context), {Type::getInt32Ty(context)}, false);
	Function*     exit      = Function::Create(exit_type, GlobalValue::ExternalLinkage, "ExitProcess", module);
	exit->setDoesNotReturn();
	builder.CreateCall(exit, {builder.CreateTrunc(main_ret_val, Type::getInt32Ty(context))});
#endif
	builder.CreateUnreachable();
}
} // namespace
Function* function_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module) {
	if (node->_metadata == nullptr)
		ERROR("Unexpected nullptr.");
	Parser::FnMeta& meta = *reinterpret_cast<Parser::FnMeta*>(node->_metadata);
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
Value* let_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module, llvm::IRBuilder<>& builder) {
	if (node->_metadata == nullptr)
		ERROR("Unexpected nullptr.");
	// Parser::LetMeta& meta = *reinterpret_cast<Parser::LetMeta*>(node->_metadata);
	ExpressionResult res = codegen(node->_args.front(), context, module, &builder);
	if (res.index() == 0) {
		Value* ptr = builder.CreateAlloca(I64_t);
		builder.CreateStore(std::get<Value*>(res), ptr);
		return std::get<Value*>(res);
	}
	ERROR("Cannot assign a function to a variable");
	return nullptr;
}
ExpressionResult scope_gen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                           llvm::IRBuilder<>& builder) {
	for (size_t i = 0; i < node->_args.size() - 1; i++)
		codegen(node->_args[i], context, module, &builder);
	return codegen(node->_args.back(), context, module, &builder);
}
ExpressionResult codegen(Parser::ASTNode* node, llvm::LLVMContext& context, llvm::Module& module,
                         llvm::IRBuilder<>* builder) {
	if (node == nullptr)
		ERROR("Unexpected nullptr.");
	if (node->_name == "let")
		return let_gen(node, context, module, *builder);
	if (node->_name == "fn")
		return function_gen(node, context, module);
	if (node->_name == "{") {
		if (builder == nullptr)
			ERROR("Operator outside function");
		return scope_gen(node, context, module, *builder);
	}
	if (*node->_name.get() >= '0' && *node->_name.get() <= '9')
		return I64_val(atoi(node->_name));
	if (node->_args.size() == 2 || node->_args.size() == 1) {
		if (builder == nullptr)
			ERROR("Operator outside function");
		return Operators::operator_gen(node, context, module, *builder);
	}
	if (node->_name == "putchar") {
		if (builder == nullptr)
			ERROR("Function call outside function.");
		if (putch_fn == nullptr) {
			FunctionType* putch_fn_type = FunctionType::get(I64_t, {I64_t}, false);
			putch_fn          = Function::Create(putch_fn_type, llvm::GlobalValue::InternalLinkage, "putchar", module);
			BasicBlock* entry = BasicBlock::Create(context, "", putch_fn);
			IRBuilder<> builder(entry);
			Argument*   c_arg = putch_fn->getArg(0);
			AllocaInst* ptr   = builder.CreateAlloca(I64_t);
			ptr->setAlignment(Align(8));
			builder.CreateStore(c_arg, ptr)->setAlignment(Align(8));
#ifdef __linux__
			FunctionType* asm_type = FunctionType::get(Type::getVoidTy(context), {PointerType::get(context, 0)}, false);
			InlineAsm*    asm_fn   = InlineAsm::get(asm_type,
			                                        ("mov $$1, %rax; "
			                                         "mov $$1, %rdi; "
			                                         "mov $0, %rsi; "
			                                         "mov $$1, %rdx; "
			                                         "syscall"),
			                                        "r,~{rax},~{rdi},~{rsi},~{rdx},~{rcx},~{r11},~{memory}", true);
			builder.CreateCall(asm_fn, {ptr});
#elif defined(__MINGW64__)
			FunctionType* GetStdHandle_type =
			    FunctionType::get(PointerType::get(context, 0), {Type::getInt32Ty(context)}, false);
			Function* GetStdHandle =
			    Function::Create(GetStdHandle_type, GlobalValue::ExternalLinkage, "GetStdHandle", module);
			FunctionType* WriteConsoleA_type =
			    FunctionType::get(Type::getInt8Ty(context),
			                      {PointerType::get(context, 0), PointerType::get(context, 0), Type::getInt32Ty(context),
			                       PointerType::get(context, 0), PointerType::get(context, 0)},
			                      false);
			Function* WriteConsoleA =
			    Function::Create(WriteConsoleA_type, GlobalValue::ExternalLinkage, "WriteConsoleA", module);
			Value* null_ptr = ConstantPointerNull::get(PointerType::get(context, 0));
			builder.CreateCall(WriteConsoleA,
			                   {builder.CreateCall(GetStdHandle, {ConstantInt::get(Type::getInt32Ty(context), -11)}), ptr,
			                    ConstantInt::get(Type::getInt32Ty(context), 1), null_ptr, null_ptr});
#endif
			builder.CreateRet(I64_val(0));
		}
		return putch_fn;
	}
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
