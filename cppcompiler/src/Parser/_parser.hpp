#pragma once
#include "../Lexer/_lexer.hpp"
#include "../Preparser/_preparser.hpp"
#include "../symboltable.hpp"
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Value.h>
#include <iostream>
#include <ostream>
#include <vector>
namespace Parser {
using Preparser::ParsingNode;
struct FnMeta {
	Lexer::Token              _name{};
	std::vector<Lexer::Token> _args{};
};
struct LetMeta {
	Lexer::Token _name{};
};
struct ASTNode {
	Lexer::Token         _name{};
	std::vector<Symbol*> _args{};
	void*                _metadata{nullptr};
	inline ASTNode(const Lexer::Token& op) : _name(op), _args(), _metadata(nullptr) {}
	inline ASTNode(ASTNode&& operation) :
	    _name(operation._name), _args(std::move(operation._args)), _metadata(operation._metadata) {}

	friend std::ostream& operator<<(std::ostream& stream, const ASTNode& node);
};
Symbol* parse(Symbol* const* begin, Symbol* const* end);
// Main function of the parser
ASTNode* run(Symbol* symbol);
} // namespace Parser
