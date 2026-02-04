#include "../error.hpp"
#include "_codegenerator.hpp"
#include <cstdint>
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
#include <vector>
namespace CodeGenerator {
using namespace llvm;
namespace {
constexpr uint64_t hashfn(const char* str) {
	uint64_t hash = 14695981039346656037U;
	while (*str) {
		hash ^= (uint8_t)*str++;
		hash *= 1099511628211U;
	}
	return hash;
}
LLVMFunction* putchar{nullptr};
LLVMFunction* get_putchar(LLVMState& state) {
	if (putchar != nullptr)
		return putchar;
	LLVMFunction* fn = new LLVMFunction("putchar", state);
	;
	Argument*   c_arg = fn->get()->getArg(0);
	AllocaInst* ptr   = fn->builder().CreateAlloca(Type::getInt64Ty(state.context()));
	ptr->setAlignment(Align(8));
	fn->builder().CreateStore(c_arg, ptr)->setAlignment(Align(8));
#ifdef __linux__
	FunctionType* asm_type =
	    FunctionType::get(Type::getVoidTy(state.context()), {PointerType::get(state.context(), 0)}, false);
	InlineAsm* asm_fn = InlineAsm::get(asm_type,
	                                   ("mov $$1, %rax; "
	                                    "mov $$1, %rdi; "
	                                    "mov $0, %rsi; "
	                                    "mov $$1, %rdx; "
	                                    "syscall"),
	                                   "r,~{rax},~{rdi},~{rsi},~{rdx},~{rcx},~{r11},~{memory}", true);
	fn->builder().CreateCall(asm_fn, {ptr});
#elif defined(__MINGW64__)
	FunctionType* GetStdHandle_type =
	    FunctionType::get(PointerType::get(state.context(), 0), {Type::getInt32Ty(state.context())}, false);
	Function* GetStdHandle =
	    Function::Create(GetStdHandle_type, GlobalValue::ExternalLinkage, "GetStdHandle", state.module());
	FunctionType* WriteConsoleA_type = FunctionType::get(
	    Type::getInt8Ty(state.context()),
	    {PointerType::get(state.context(), 0), PointerType::get(state.context(), 0), Type::getInt32Ty(state.context()),
	     PointerType::get(state.context(), 0), PointerType::get(state.context(), 0)},
	    false);
	Function* WriteConsoleA =
	    Function::Create(WriteConsoleA_type, GlobalValue::ExternalLinkage, "WriteConsoleA", state.module());
	Value* null_ptr = ConstantPointerNull::get(PointerType::get(state.context(), 0));
	fn->_builder->CreateCall(
	    WriteConsoleA,
	    {fn->_builder->CreateCall(GetStdHandle, {ConstantInt::get(Type::getInt32Ty(state.context()), -11)}), ptr,
	     ConstantInt::get(Type::getInt32Ty(state.context()), 1), null_ptr, null_ptr});
#endif
	fn->finalize(ConstantInt::get(Type::getInt64Ty(state.context()), 0));
	putchar = fn;
	return putchar;
}
LLVMFunction* getchar{nullptr};
LLVMFunction* get_getchar(LLVMState& state) {
	if (getchar != nullptr)
		return getchar;
	LLVMFunction* fn = new LLVMFunction("getchar", state);
	;
	Argument*   c_arg = fn->get()->getArg(0);
	AllocaInst* ptr   = fn->builder().CreateAlloca(Type::getInt64Ty(state.context()));
	ptr->setAlignment(Align(8));
	fn->builder().CreateStore(c_arg, ptr)->setAlignment(Align(8));
#ifdef __linux__
	FunctionType* asm_type =
	    FunctionType::get(Type::getVoidTy(state.context()), {PointerType::get(state.context(), 0)}, false);
	InlineAsm* asm_fn = InlineAsm::get(asm_type,
	                                   ("mov $$0, %rax; "
	                                    "mov $$1, %rdi; "
	                                    "mov $0, %rsi; "
	                                    "mov $$1, %rdx; "
	                                    "syscall"),
	                                   "r,~{rax},~{rdi},~{rsi},~{rdx},~{rcx},~{r11},~{memory}", true);
	fn->builder().CreateCall(asm_fn, {ptr});
	fn->finalize(fn->builder().CreateLoad(Type::getInt64Ty(state.context()), ptr));
#elif defined(__MINGW64__)
	FunctionType* GetStdHandle_type =
	    FunctionType::get(PointerType::get(state.context(), 0), {Type::getInt32Ty(state.context())}, false);
	Function* GetStdHandle =
	    Function::Create(GetStdHandle_type, GlobalValue::ExternalLinkage, "GetStdHandle", state.module());
	FunctionType* WriteConsoleA_type = FunctionType::get(
	    Type::getInt8Ty(state.context()),
	    {PointerType::get(state.context(), 0), PointerType::get(state.context(), 0), Type::getInt32Ty(state.context()),
	     PointerType::get(state.context(), 0), PointerType::get(state.context(), 0)},
	    false);
	Function* WriteConsoleA =
	    Function::Create(WriteConsoleA_type, GlobalValue::ExternalLinkage, "WriteConsoleA", state.module());
	Value* null_ptr = ConstantPointerNull::get(PointerType::get(state.context(), 0));
	fn->_builder->CreateCall(
	    WriteConsoleA,
	    {fn->_builder->CreateCall(GetStdHandle, {ConstantInt::get(Type::getInt32Ty(state.context()), -11)}), ptr,
	     ConstantInt::get(Type::getInt32Ty(state.context()), 1), null_ptr, null_ptr});
#endif
	getchar = fn;
	return getchar;
}
uint64_t new_llvm_state_id{0};
} // namespace
LLVMFunction::LLVMFunction(const char* name, LLVMState& state) : _data(nullptr), _state(state), _definition(nullptr) {
	FunctionType* fn_type =
	    FunctionType::get(Type::getInt64Ty(state.context()), {Type::getInt64Ty(state.context())}, false);
	_data = Function::Create(fn_type, GlobalValue::ExternalLinkage, name, _state.module());
}
IRBuilder<>& LLVMFunction::builder() {
	if (_definition != nullptr)
		return _definition->_builder;
	_definition = new LLVMDefinition(BasicBlock::Create(_state.context(), "", _data));
	return _definition->_builder;
}
BasicBlock* LLVMFunction::block() {
	if (_definition != nullptr)
		return _definition->_block;
	_definition = new LLVMDefinition(BasicBlock::Create(_state.context(), "", _data));
	return _definition->_block;
}
IRBuilder<>& LLVMFunction::succeed() {
	_definition = new LLVMDefinition(BasicBlock::Create(_state.context(), "", _data));
	return builder();
}
void LLVMFunction::finalize(Value* return_value) {
	builder().CreateRet(return_value);
}
LLVMVariable::LLVMVariable(const char* name, LLVMState& state, IRBuilder<>* builder) :
    _type(builder == nullptr ? ScopeType::global : ScopeType::local), _data(), _state(state) {
	if (builder == nullptr) {
		_data._global =
		    new GlobalVariable(_state.module(), Type::getInt64Ty(_state.context()), false, GlobalValue::ExternalLinkage,
		                       ConstantInt::get(Type::getInt64Ty(_state.context()), 0), name);
		return;
	}
	_data._local = builder->CreateAlloca(Type::getInt64Ty(_state.context()), nullptr, name);
}
void LLVMVariable::finalize(IRBuilder<>& builder, Value* initializer) {
	builder.CreateStore(initializer, get());
}
LLVMNode::LLVMNode(Symbol* symbol, LLVMState& state, LLVMNode* parent) :
    _type(NodeType::none), _data(), _symbol(symbol), _parent(parent), _state(state), _is_finalized(false) {}
void LLVMNode::finalize() {
	if (_is_finalized)
		return;
	_is_finalized             = true;
	Parser::ASTNode& ast_node = _symbol->get_ast_node();
	if (ast_node._name == "if") {
		LLVMNode&     node1    = ast_node._args[0]->get_llvm_node(_state, this);
		LLVMNode&     node2    = ast_node._args[1]->get_llvm_node(_state, this);
		LLVMNode&     node3    = ast_node._args[2]->get_llvm_node(_state, this);
		LLVMFunction& parent   = parent_function();
		IRBuilder<>&  builder1 = parent.builder();
		node1.finalize();
		IRBuilder<>& builder2 = parent.succeed();
		BasicBlock*  block2   = parent.block();
		node2.finalize();
		IRBuilder<>& builder3 = parent.succeed();
		BasicBlock*  block3   = parent.block();
		node3.finalize();
		IRBuilder<>& builder4 = parent.succeed();
		BasicBlock*  block4   = parent.block();
		builder1.CreateCondBr(
		    builder1.CreateICmpNE(node1.get_value(), ConstantInt::get(Type::getInt64Ty(_state.context()), 0)), block2,
		    block3);
		builder2.CreateBr(block4);
		builder3.CreateBr(block4);
		PHINode* phi = builder4.CreatePHI(Type::getInt64Ty(_state.context()), 2);
		phi->addIncoming(node2.get_value(), block2);
		phi->addIncoming(node3.get_value(), block3);
		_type      = NodeType::none;
		_data._val = phi;
		return;
	}
	if (ast_node._name == "while") {
		LLVMNode&     node2    = ast_node._args[0]->get_llvm_node(_state, this);
		LLVMNode&     node3    = ast_node._args[1]->get_llvm_node(_state, this);
		LLVMFunction& parent   = parent_function();
		IRBuilder<>&  builder1 = parent.builder();
		IRBuilder<>&  builder2 = parent.succeed();
		BasicBlock*   block2   = parent.block();
		builder1.CreateBr(block2);
		node2.finalize();
		IRBuilder<>& builder3 = parent.succeed();
		BasicBlock*  block3   = parent.block();
		node3.finalize();
		parent.succeed();
		BasicBlock* block4 = parent.block();
		builder2.CreateCondBr(
		    builder2.CreateICmpNE(node2.get_value(), ConstantInt::get(Type::getInt64Ty(_state.context()), 0)), block3,
		    block4);
		builder3.CreateBr(block2);
		_type      = NodeType::none;
		_data._val = ConstantInt::get(Type::getInt64Ty(_state.context()), 0);
		return;
	}
	std::vector<LLVMNode*> children{};
	children.reserve(ast_node._args.size());
	for (Symbol* child: ast_node._args)
		children.push_back(&child->get_llvm_node(_state, this));
	for (LLVMNode* child: children)
		child->finalize();
	if (_type == NodeType::fn) {
		_data._fn->finalize(children.front()->get_value());
		return;
	}
	if (_type == NodeType::let) {
		_data._let->finalize(builder(), children.front()->get_value());
		return;
	}
	if (*ast_node._name.get() >= '0' && *ast_node._name.get() <= '9') {
		_data._val = ConstantInt::get(Type::getInt64Ty(_state.context()), atoi(ast_node._name));
		return;
	}
	switch (hashfn(ast_node._name)) {
	case hashfn("putchar"): {
		_type     = NodeType::fn;
		_data._fn = get_putchar(_state);
	}
		return;
	case hashfn("getchar"): {
		_type     = NodeType::fn;
		_data._fn = get_getchar(_state);
	}
		return;
	case hashfn("{"): _data._val = children.back()->get_value(); return;
	default: break;
	}
	if (children.size() > 0) {
		if (strlen(ast_node._name) <= 1)
			_data._val = opgen(builder(), *ast_node._name.get(), children.size() >= 1 ? children[0] : nullptr,
			                   children.size() >= 2 ? children[1] : nullptr, _state);
		else if (ast_node._name.get()[1] == '=')
			_data._val = opgen2(builder(), *ast_node._name.get(), children.size() >= 1 ? children[0] : nullptr,
			                    children.size() >= 2 ? children[1] : nullptr, _state);
		else
			ERROR("Unknown operation: ", ast_node._name);
		return;
	}
	Symbol* symbol = _symbol->get_table()[ast_node._name];
	if (symbol == nullptr)
		ERROR("Symbol not found: ", ast_node._name);
	LLVMNode& node = symbol->get_llvm_node(_state, nullptr);
	if (node._type == NodeType::fn) {
		_type     = NodeType::fn;
		_data._fn = node._data._fn;
		return;
	}
	if (node._type == NodeType::let) {
		_type      = NodeType::let;
		_data._let = node._data._let;
		return;
	}
	_data._val = node.get_value();
}
IRBuilder<>& LLVMNode::builder() {
	if (_type == NodeType::fn)
		return _data._fn->builder();
	if (_parent == nullptr)
		return _state.entry().builder();
	return _parent->builder();
}
LLVMFunction& LLVMNode::parent_function() {
	if (_type == NodeType::fn)
		return *_data._fn;
	if (_parent == nullptr)
		return _state.entry();
	return _parent->parent_function();
}
void LLVMNode::create_let() {
	if (_type != NodeType::none)
		ERROR("Attempted to overwrite node.");
	_type                 = NodeType::let;
	Parser::LetMeta& meta = *reinterpret_cast<Parser::LetMeta*>(_symbol->get_ast_node()._metadata);
	_data._let            = new LLVMVariable(meta._name, _state, _parent == nullptr ? nullptr : &_parent->builder());
}
void LLVMNode::create_fn() {
	if (_type != NodeType::none)
		ERROR("Attempted to overwrite node.");
	_type                = NodeType::fn;
	Parser::FnMeta& meta = *reinterpret_cast<Parser::FnMeta*>(_symbol->get_ast_node()._metadata);
	_data._fn            = new LLVMFunction(meta._name, _state);
}
Value* LLVMNode::get_value() {
	if (_type == NodeType::fn)
		ERROR("Invalid call.");
	if (_type == NodeType::let)
		return builder().CreateLoad(Type::getInt64Ty(_state.context()), get_variable());
	if (_data._val == nullptr)
		finalize();
	return _data._val;
}
Function* LLVMNode::get_function() {
	if (_type != NodeType::fn)
		ERROR("Invalid call.");
	return _data._fn->get();
}
Value* LLVMNode::get_variable() {
	if (_type != NodeType::let)
		ERROR("Invalid call.");
	return _data._let->get();
}
std::ostream& operator<<(std::ostream& stream, const LLVMNode& node) {
	stream << "Not yet implemented";
	return stream;
}
LLVMState::LLVMState() : _context(new LLVMContext()), _module(nullptr), _entry(nullptr), _id(new_llvm_state_id++) {
	_module = new Module("Program", *_context);
	_entry  = new LLVMFunction("_start", *this);
	_entry->get()->setDoesNotReturn();
	_entry->get()->setCallingConv(CallingConv::C);
}
LLVMState::~LLVMState() {}
void LLVMState::add_exit_syscall(llvm::Value* code) {
#ifdef __linux__
	FunctionType* exit_type = FunctionType::get(Type::getVoidTy(*_context), {Type::getInt64Ty(*_context)}, false);
	InlineAsm*    exit      = InlineAsm::get(exit_type, "mov $0, %rdi; mov $$60, %rax; syscall", "r", true);
	_entry->builder().CreateCall(exit, {code});
#elif defined(__MINGW64__)
	FunctionType* exit_type = FunctionType::get(Type::getVoidTy(*_context), {Type::getInt32Ty(*_context)}, false);
	Function*     exit      = Function::Create(exit_type, GlobalValue::ExternalLinkage, "ExitProcess", *_module);
	exit->setDoesNotReturn();
	_entry->_builder->CreateCall(exit, {_entry->_builder->CreateTrunc(code, Type::getInt32Ty(*_context))});
#endif
	_entry->builder().CreateUnreachable();
}
LLVMNode* run(Symbol* symbol, LLVMState& state, LLVMNode* parent) {
	LLVMNode* result = new LLVMNode(symbol, state, parent);
	switch (hashfn(symbol->get_ast_node()._name.get())) {
	case hashfn("fn"): result->create_fn(); break;
	case hashfn("let"): result->create_let(); break;
	default: break;
	}
	return result;
}
} // namespace CodeGenerator
