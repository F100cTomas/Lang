#include "_codegenerator.hpp"
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/InlineAsm.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Value.h>
namespace CodeGenerator {
using namespace llvm;
LLVMNode* blank_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function) {
	Function* left  = node._args[0]->get_llvm_node(state, function).get_fn()->_fn;
	Value*    right = node._args[1]->get_llvm_node(state, function).get_val()->_value;
	return new LLVMNode(new LLVMValue{function->_builder->CreateCall(left, {right}), function});
}
LLVMNode* mul_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function) {
	Value* left  = node._args[0]->get_llvm_node(state, function).get_val()->_value;
	Value* right = node._args[1]->get_llvm_node(state, function).get_val()->_value;
	return new LLVMNode(new LLVMValue{function->_builder->CreateMul(left, right), function});
}
LLVMNode* div_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function) {
	Value* left  = node._args[0]->get_llvm_node(state, function).get_val()->_value;
	Value* right = node._args[1]->get_llvm_node(state, function).get_val()->_value;
	return new LLVMNode(new LLVMValue{function->_builder->CreateSDiv(left, right), function});
}
LLVMNode* mod_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function) {
	Value* left  = node._args[0]->get_llvm_node(state, function).get_val()->_value;
	Value* right = node._args[1]->get_llvm_node(state, function).get_val()->_value;
	return new LLVMNode(new LLVMValue{function->_builder->CreateSRem(left, right), function});
}
LLVMNode* add_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function) {
	Value* left  = node._args[0]->get_llvm_node(state, function).get_val()->_value;
	Value* right = node._args[1]->get_llvm_node(state, function).get_val()->_value;
	return new LLVMNode(new LLVMValue{function->_builder->CreateAdd(left, right), function});
}
LLVMNode* sub_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function) {
	Value* left  = node._args[0]->get_llvm_node(state, function).get_val()->_value;
	Value* right = node._args[1]->get_llvm_node(state, function).get_val()->_value;
	return new LLVMNode(new LLVMValue{function->_builder->CreateSub(left, right), function});
}
LLVMNode* u_plus_gen(Symbol* symbol, LLVMState& state, LLVMFunction* function) {
	Parser::ASTNode& node = symbol->get_ast_node();
	if (node._args.size() != 1)
		ERROR("Wrong numer of arguments ", node._args.size());
	Symbol* val = node._args.front();
	symbol->be_suceeded_by(val);
	return &val->get_llvm_node(state, function);
}
LLVMNode* u_minus_gen(Parser::ASTNode& node, LLVMState& state, LLVMFunction* function) {
	Value* val = node._args.front()->get_llvm_node(state, function).get_val()->_value;
	return new LLVMNode(new LLVMValue{function->_builder->CreateNeg(val), function});
}
} // namespace CodeGenerator
