#pragma once
#include "../Lexer/_lexer.hpp"
#include <vector>

namespace Parser {
class SymbolTable {
	struct TablePair {
		Lexer::Token symbol;
		Lexer::Token value;
	};
private:
	std::vector<TablePair> _data[256]{};
	SymbolTable* _upper_level{nullptr};
public:
	SymbolTable();
	~SymbolTable();
public:

};
}
