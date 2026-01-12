#pragma once
#include "../Parser/_parser.hpp"
#include "../error.hpp"
#include <iostream>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
#include <variant>
namespace CodeGenerator {
using ExpressionResult = std::variant<llvm::Value*, llvm::Function*>;
enum class NodeType : uint8_t {
	empty,
	val,
	fn,
};
class LLVMState;
struct LLVMFunction {
	llvm::Function*    _fn{nullptr};
	llvm::BasicBlock*  _block{nullptr};
	llvm::IRBuilder<>* _alloca_builder{nullptr};
	llvm::IRBuilder<>* _builder{nullptr};

public:
	LLVMFunction(LLVMState& state, const Lexer::Token& name);
	friend std::ostream& operator<<(std::ostream& stream, const LLVMFunction& fn);
};
struct LLVMValue {
	llvm::Value*  _value{nullptr};
	LLVMFunction* _parent{nullptr};

public:
	friend std::ostream& operator<<(std::ostream& stream, const LLVMValue& val);
};
struct LLVMNode {
	union NodeData {
		LLVMValue*    _val{nullptr};
		LLVMFunction* _fn;
	};
	NodeData _value{};
	NodeType _type{NodeType::empty};
	inline LLVMNode(LLVMValue* value) : _type(NodeType::val) {
		_value._val = value;
	}
	inline LLVMNode(LLVMFunction* fn) : _type(NodeType::fn) {
		_value._fn = fn;
	}
	inline LLVMValue* get_val() {
		if (_type != NodeType::val)
			ERROR("Tried to access invalid data");
		return _value._val;
	}
	inline LLVMFunction* get_fn() {
		if (_type != NodeType::fn)
			ERROR("Tried to access invalid data");
		return _value._fn;
	}
	friend inline std::ostream& operator<<(std::ostream& stream, const LLVMNode& node) {
		switch (node._type) {
		case NodeType::val: return stream << *node._value._val;
		case NodeType::fn: return stream << *node._value._fn;
		default: break;
		}
		ERROR("Tried to access invalid data");
		return stream;
	}
};
class LLVMState {
	llvm::LLVMContext* _context{nullptr};
	llvm::Module*      _module{nullptr};
	LLVMFunction*      _entry{nullptr};

public:
	LLVMState();
	LLVMState(const LLVMState&) = delete;
	~LLVMState();
	llvm::LLVMContext& context() {
		return *_context;
	}
	llvm::Module& module() {
		return *_module;
	}
	LLVMFunction* entry() {
		return _entry;
	}
	void add_exit_syscall(llvm::Value* code);
};
LLVMNode*     blank_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function);
LLVMNode*     mul_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function);
LLVMNode*     div_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function);
LLVMNode*     mod_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function);
LLVMNode*     add_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function);
LLVMNode*     sub_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function);
LLVMNode*     u_plus_gen(Symbol* symbol, LLVMState& state, LLVMFunction* function);
LLVMNode*     u_minus_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function);
LLVMNode*     let_gen(Symbol* symbol, LLVMState& state, LLVMFunction* function);
LLVMNode*     scope_gen(Symbol* symbol, LLVMState& state, LLVMFunction* function);
LLVMFunction* function_gen(Symbol* symbol, LLVMState& state);
LLVMNode*     run(Symbol* symbol, LLVMState& state, LLVMFunction* function);
} // namespace CodeGenerator
