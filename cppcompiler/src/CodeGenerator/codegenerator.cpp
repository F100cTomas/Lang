#include "../error.hpp"
#include "_codegenerator.hpp"
#include <llvm/IR/Argument.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/CallingConv.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Alignment.h>
#include <llvm/Support/raw_ostream.h>
#include <memory>
#include <ostream>
#include <string>
#ifdef __linux__
#define START_SYMBOL "_start"
#elif defined(__MINGW64__)
#define START_SYMBOL "mainCRTStartup"
#endif
namespace CodeGenerator {
using namespace llvm;
using std::unique_ptr, std::make_unique;
namespace {
constexpr uint64_t hashfn(const char* str) {
	uint64_t hash = 14695981039346656037U;
	while (*str) {
		hash ^= static_cast<uint8_t>(*str++);
		hash *= 1099511628211U;
	}
	return hash;
}
LLVMNode* putchar{nullptr};
LLVMNode* get_putchar(LLVMState& state) {
	if (putchar != nullptr)
		return putchar;
	LLVMFunction* fn = new LLVMFunction(state, "putchar");;
	Argument*   c_arg = fn->_fn->getArg(0);
	AllocaInst* ptr   = fn->_builder->CreateAlloca(Type::getInt64Ty(state.context()));
	ptr->setAlignment(Align(8));
	fn->_builder->CreateStore(c_arg, ptr)->setAlignment(Align(8));
	#ifdef __linux__
	FunctionType* asm_type = FunctionType::get(Type::getVoidTy(state.context()), {PointerType::get(state.context(), 0)}, false);
	InlineAsm*    asm_fn   = InlineAsm::get(asm_type,
											("mov $$1, %rax; "
											"mov $$1, %rdi; "
											"mov $0, %rsi; "
											"mov $$1, %rdx; "
											"syscall"),
										 "r,~{rax},~{rdi},~{rsi},~{rdx},~{rcx},~{r11},~{memory}", true);
	fn->_builder->CreateCall(asm_fn, {ptr});
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
	fn->_builder->CreateRet(ConstantInt::get(Type::getInt64Ty(state.context()), 0));
	putchar = new LLVMNode(fn);
	return putchar;
}
} // namespace
LLVMFunction::LLVMFunction(LLVMState& state, const Lexer::Token& name) {
	FunctionType* type = FunctionType::get(Type::getInt64Ty(state.context()), {Type::getInt64Ty(state.context())}, false);
	_fn                = Function::Create(type, GlobalValue::ExternalLinkage, name.get(), state.module());
	_block             = BasicBlock::Create(state.context(), "", _fn);
	_alloca_builder    = new IRBuilder<>(_block, _block->begin());
	_builder           = new IRBuilder<>(_block);
}
std::ostream& operator<<(std::ostream& stream, const LLVMFunction& fn) {
	std::string        buffer{};
	raw_string_ostream llvm_stream{buffer};
	fn._fn->print(llvm_stream);
	return stream << buffer;
}
std::ostream& operator<<(std::ostream& stream, const LLVMValue& val) {
	std::string        buffer{};
	raw_string_ostream llvm_stream{buffer};
	val._value->print(llvm_stream);
	return stream << buffer;
}
LLVMState::LLVMState() : _context(new LLVMContext()) {
	_module = new Module("Program", *_context);
	_entry  = new LLVMFunction(*this, START_SYMBOL);
	_entry->_fn->setDoesNotReturn();
	_entry->_fn->setCallingConv(CallingConv::C);
	// Create exit syscall
}
void LLVMState::add_exit_syscall(llvm::Value* code) {
#ifdef __linux__
	FunctionType* exit_type = FunctionType::get(Type::getVoidTy(*_context), {Type::getInt64Ty(*_context)}, false);
	InlineAsm*    exit      = InlineAsm::get(exit_type, "mov $0, %rdi; mov $$60, %rax; syscall", "r", true);
	_entry->_builder->CreateCall(exit, {code});
#elif defined(__MINGW64__)
	FunctionType* exit_type = FunctionType::get(Type::getVoidTy(context), {Type::getInt32Ty(context)}, false);
	Function*     exit      = Function::Create(exit_type, GlobalValue::ExternalLinkage, "ExitProcess", module);
	exit->setDoesNotReturn();
	_entry->_builder.CreateCall(exit, {_entry->_builder->CreateTrunc(code, Type::getInt32Ty(*_context))});
#endif
	_entry->_builder->CreateUnreachable();
}
LLVMState::~LLVMState() {
	/*
	delete _module;
	delete _context;
	*/
}
LLVMNode* scope_gen(Symbol* symbol, LLVMState& state, LLVMFunction* function) {
	Parser::ASTNode& node = symbol->get_ast_node();
	for (Symbol* arg: node._args)
		arg->get_llvm_node(state, function);
	symbol->be_suceeded_by(node._args.back());
	return &node._args.back()->get_llvm_node(state, function);
}
LLVMFunction* function_gen(Symbol* symbol, LLVMState& state) {
	Parser::ASTNode& node = symbol->get_ast_node();
	if (node._metadata == nullptr)
		ERROR("Unexpected nullptr");
	const Parser::FnMeta& meta = *reinterpret_cast<const Parser::FnMeta*>(node._metadata);
	LLVMFunction*         fn   = new LLVMFunction(state, meta._name);
	fn->_builder->CreateRet(node._args.front()->get_llvm_node(state, fn).get_val()->_value);
	return fn;
}
LLVMNode* run(Symbol* symbol, LLVMState& state, LLVMFunction* function) {
	Parser::ASTNode& node = symbol->get_ast_node();
	switch (hashfn(node._name)) {
	case hashfn("putchar"): return get_putchar(state);
	case hashfn("fn"): return new LLVMNode(function_gen(symbol, state));
	case hashfn("{"): return scope_gen(symbol, state, function);
	case hashfn("+"):
		return node._args.size() <= 1 ? u_plus_gen(symbol, state, function) : add_gen(node, state, function);
	case hashfn("-"): return node._args.size() <= 1 ? u_minus_gen(node, state, function) : sub_gen(node, state, function);
	case hashfn(""): return blank_gen(node, state, function);
	case hashfn("*"): return mul_gen(node, state, function);
	case hashfn("/"): return div_gen(node, state, function);
	case hashfn("%"): return mod_gen(node, state, function);
	default: break;
	}
	if (*node._name.get() >= '0' && *node._name.get() <= '9')
		return new LLVMNode(new LLVMValue{ConstantInt::get(Type::getInt64Ty(state.context()), std::atoi(node._name))});
	return new LLVMNode(new LLVMValue{ConstantInt::get(Type::getInt64Ty(state.context()), 0), function});
}
} // namespace CodeGenerator
