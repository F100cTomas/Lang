#pragma once
#include "Lexer/_lexer.hpp"
#include <iostream>
#include <vector>
namespace Preparser {
struct ParsingNode;
}
namespace Parser {
struct ASTNode;
}
namespace CodeGenerator {
struct LLVMFunction;
struct LLVMNode;
class LLVMState;
} // namespace CodeGenerator
class Symbol {
	Preparser::ParsingNode*      _parsing_node{nullptr};
	Parser::ASTNode*             _ast_node{nullptr};
	CodeGenerator::LLVMNode*     _llvm_node{nullptr};
	CodeGenerator::LLVMFunction* _llvm_function{nullptr};
	Symbol*                      _successor{nullptr};

public:
	inline Symbol(Preparser::ParsingNode& parsing_node) : _parsing_node(&parsing_node) {}
	inline Symbol(Parser::ASTNode& ast_node) : _ast_node(&ast_node) {}
	inline Symbol(CodeGenerator::LLVMNode& llvm_node) : _llvm_node(&llvm_node) {}
	inline void be_suceeded_by(Symbol* successor) {
		_successor = successor;
	}
	Symbol(const Symbol&) = delete;
	~Symbol();
	friend std::ostream& operator<<(std::ostream& stream, const Symbol& symbol);

public:
	Preparser::ParsingNode&  get_parsing_node() const;
	Parser::ASTNode&         get_ast_node();
	CodeGenerator::LLVMNode& get_llvm_node(CodeGenerator::LLVMState& llvm_state, CodeGenerator::LLVMFunction* function);
	CodeGenerator::LLVMNode& get_llvm_node(CodeGenerator::LLVMState& llvm_state);
};
class SymbolTable {
	struct TableValue {
		Lexer::Token _name;
		Symbol*      _symbol;
		inline TableValue(const Lexer::Token& name, Symbol* symbol) : _name(name), _symbol(symbol) {}
	};
	std::vector<Symbol*>      _values{};
	std::vector<TableValue>   _named_values[16]{};
	SymbolTable*              _parent{nullptr};
	std::vector<SymbolTable*> _children{};

public:
	SymbolTable();
	SymbolTable(SymbolTable* upper);
	SymbolTable(const SymbolTable&) = delete;
	~SymbolTable();

public:
	void        register_symbol(Symbol* symbol);
	void        register_named_symbol(Symbol* symbol, const Lexer::Token& name);
	Symbol*     operator[](const Lexer::Token& name);
	inline auto begin() {
		return _values.begin();
	}
	inline auto end() {
		return _values.end();
	}
	friend std::ostream& operator<<(std::ostream& stream, const SymbolTable& st);
};
#include "CodeGenerator/_codegenerator.hpp"
#include "Parser/_parser.hpp"
#include "Preparser/_preparser.hpp"
