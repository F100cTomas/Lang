#pragma once
#include "../error.hpp"
#include "../symboltable.hpp"
#include <iostream>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>
namespace CodeGenerator {
enum class NodeType : uint8_t { none, fn, let };
enum class ScopeType : uint8_t { empty, local, global };
class LLVMState;
class LLVMFunction {
	struct LLVMDefinition {
		llvm::IRBuilder<> _builder;
		llvm::BasicBlock* _block;
		inline LLVMDefinition(llvm::BasicBlock* block) : _builder(block), _block(block) {}
	};
	llvm::Function* _data{nullptr};
	LLVMState&      _state;
	LLVMDefinition* _definition{nullptr};

public:
	LLVMFunction(const char* name, LLVMState& _state);
	llvm::IRBuilder<>&     builder();
	inline llvm::Function* get() {
		return _data;
	}
	void finalize(llvm::Value* return_value);
};
class LLVMVariable {
	union VariableData {
		llvm::AllocaInst*     _local{nullptr};
		llvm::GlobalVariable* _global;
	};
	ScopeType    _type{ScopeType::empty};
	VariableData _data{};
	LLVMState&   _state;

public:
	LLVMVariable(const char* name, LLVMState& state, llvm::IRBuilder<>* builder);
	inline llvm::Value* get() {
		switch (_type) {
		case ScopeType::local: return _data._local;
		case ScopeType::global: return _data._global;
		default: break;
		}
		ERROR("Invalid memory.");
		return nullptr;
	}
	void finalize(llvm::IRBuilder<>& builder, llvm::Value* initializer);
};
class LLVMNode {
	union NodeData {
		llvm::Value*  _val{nullptr};
		LLVMFunction* _fn;
		LLVMVariable* _let;
	};
	NodeType   _type{NodeType::none};
	NodeData   _data{};
	Symbol*    _symbol{nullptr};
	LLVMNode*  _parent{nullptr};
	LLVMState& _state;
	bool       _is_finalized{false};

public:
	LLVMNode(Symbol* symbol, LLVMState& state, LLVMNode* parent);
	Symbol* symbol() {
		return _symbol;
	}
	void                 finalize();
	llvm::IRBuilder<>&   builder();
	void                 create_let();
	void                 create_fn();
	llvm::Value*         get_value();
	llvm::Function*      get_function();
	llvm::Value*         get_variable();
	friend std::ostream& operator<<(std::ostream& stream, const LLVMNode& node);
};
class LLVMState {
	llvm::LLVMContext* _context{nullptr};
	llvm::Module*      _module{nullptr};
	LLVMFunction*      _entry;
	uint64_t           _id{0};

public:
	LLVMState();
	LLVMState(const LLVMState&) = delete;
	~LLVMState();
	inline llvm::LLVMContext& context() {
		return *_context;
	}
	inline llvm::Module& module() {
		return *_module;
	}
	inline LLVMFunction& entry() {
		return *_entry;
	}
	inline uint64_t id() const {
		return _id;
	}
	void add_exit_syscall(llvm::Value* code);
};
llvm::Value* opgen(llvm::IRBuilder<>& builder, char op, LLVMNode* left, LLVMNode* right, LLVMState& state);
llvm::Value* opgen2(llvm::IRBuilder<>& builder, char op, LLVMNode* left, LLVMNode* right, LLVMState& state);
LLVMNode* run(Symbol* symbol, LLVMState& state, LLVMNode* parent);
} // namespace CodeGenerator
