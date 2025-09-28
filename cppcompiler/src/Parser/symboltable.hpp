#pragma once
#include "../Lexer/_lexer.hpp"
#include <iostream>
#include <vector>
namespace Parser {
struct ASTNode;
struct TableValue {
	Lexer::Token _symbol;
	ASTNode*     _data;
	inline TableValue(Lexer::Token symbol, ASTNode* data) : _symbol(symbol), _data(data) {}
};
class SymbolTable {
	std::vector<TableValue>   _data[16]{};
	SymbolTable*              _upper_level{nullptr};
	std::vector<SymbolTable*> _lower_level{};

public:
	SymbolTable(SymbolTable* upper);
	SymbolTable(const SymbolTable&) = delete;
	~SymbolTable();

public:
	void                 insert(const Lexer::Token& index, ASTNode* value);
	ASTNode*             get(const Lexer::Token& index) const;
	bool                 is_registered(const Lexer::Token& index) const;
	friend std::ostream& operator<<(std::ostream& stream, const SymbolTable& symbols);
};
} // namespace Parser
