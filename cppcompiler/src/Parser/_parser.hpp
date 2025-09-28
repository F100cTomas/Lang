#pragma once
#include "../Lexer/_lexer.hpp"
#include "preparser.hpp"
#include "symboltable.hpp"
#include <iostream>
#include <memory>
#include <vector>
namespace Parser {
struct ASTNode;
struct FnMeta {
	Lexer::Token              _name;
	std::vector<Lexer::Token> _args;
	SymbolTable&              _symbols;
	inline FnMeta(const Lexer::Token& name, const std::vector<Lexer::Token> args, SymbolTable& symbols) :
	    _name(name), _args(args), _symbols(symbols) {}
};
struct ReferenceMeta {
	Lexer::Token _name;
	SymbolTable& _symbols;
	inline ReferenceMeta(const Lexer::Token& name, SymbolTable& symbols) : _name(name), _symbols(symbols) {}
};
struct ASTNode {
	Lexer::Token         _name;
	void*                _metadata{nullptr};
	std::vector<ASTNode> _args{};
	inline ASTNode(const Lexer::Token& op) : _name(op), _args() {}
	inline ASTNode(ASTNode&& operation) :
	    _name(operation._name), _metadata(operation._metadata), _args(std::move(operation._args)) {}
	inline ~ASTNode() {}
	friend std::ostream& operator<<(std::ostream& stream, const ASTNode& op);
};
class AST {
	std::vector<ASTNode> _statements{};
	SymbolTable          _symbols{nullptr};

public:
	AST(size_t statement_amount);
	void                 add_statement(const std::vector<ParsingNode>& expression);
	friend std::ostream& operator<<(std::ostream& stream, const AST& ast);
	inline SymbolTable&  get_symbol_table() {
    return _symbols;
	}
};
ASTNode parse(const ParsingNode* begin, const ParsingNode* end);
// Main function of the parser
std::unique_ptr<AST> run(const std::vector<Lexer::Token>& code);
} // namespace Parser
