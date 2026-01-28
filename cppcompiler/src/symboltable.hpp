#pragma once
#include "Lexer/_lexer.hpp"
#include <cstdint>
#include <iostream>
#include <unordered_map>
#include <vector>
class SymbolTable;
namespace Preparser {
struct ParsingNode;
}
namespace Parser {
struct ASTNode;
}
namespace CodeGenerator {
class LLVMNode;
class LLVMState;
} // namespace CodeGenerator
class Symbol {
	SymbolTable*                                           _table{nullptr};
	Preparser::ParsingNode*                                _parsing_node{nullptr};
	Parser::ASTNode*                                       _ast_node{nullptr};
	std::unordered_map<uint64_t, CodeGenerator::LLVMNode*> _llvm_nodes{};
	Symbol*                                                _successor{nullptr};

public:
	Symbol(SymbolTable& table, Preparser::ParsingNode& parsing_node);
	Symbol(SymbolTable& table, Parser::ASTNode& ast_node);
	Symbol(SymbolTable& table, CodeGenerator::LLVMState& state, CodeGenerator::LLVMNode& llvm_node);
	inline SymbolTable& get_table() {
		return *_table;
	}
	void be_suceeded_by(Symbol* successor);
	Symbol(const Symbol&) = delete;
	~Symbol();
	friend std::ostream& operator<<(std::ostream& stream, const Symbol& symbol);

public:
	Preparser::ParsingNode&  get_parsing_node() const;
	Parser::ASTNode&         get_ast_node();
	CodeGenerator::LLVMNode& get_llvm_node(CodeGenerator::LLVMState& llvm_state, CodeGenerator::LLVMNode* parent);
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
