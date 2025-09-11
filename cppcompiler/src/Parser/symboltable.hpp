#pragma once
#include "../Lexer/_lexer.hpp"
#include <vector>

namespace Parser {
struct ASTNode;
class SymbolTable {
	struct TablePair {
		Lexer::Token symbol;
		ASTNode* value;
	};
private:
	std::vector<TablePair> _data[16]{};
	SymbolTable* _upper_level{nullptr};
public:
	SymbolTable(SymbolTable* upper);
	~SymbolTable();
public:
	// void insert(const Lexer::Token& index, ASTNode* value);
};
}
