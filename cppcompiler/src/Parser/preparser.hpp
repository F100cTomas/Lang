#pragma once
#include "../Lexer/_lexer.hpp"
#include "../builtins.hpp"
#include "symboltable.hpp"
#include <iostream>
#include <optional>
#include <vector>
namespace Parser {
struct ParsingNode;
struct ASTNode;
class KeywordData {
public:
	inline virtual ~KeywordData() {};
	virtual ASTNode parse_keyword() const = 0;
};
class ParenData : public KeywordData {
	std::vector<ParsingNode> _expression;

public:
	ParenData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved);
	inline ~ParenData() override {}
	ASTNode parse_keyword() const override;
};
class ScopeData : public KeywordData {
	std::vector<std::vector<ParsingNode>> _statements;
	std::vector<ParsingNode>              _expression;

public:
	ScopeData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved);
	inline ~ScopeData() override {}
	ASTNode parse_keyword() const override;
};
class IfData : public KeywordData {
	std::vector<ParsingNode> _cond;
	std::vector<ParsingNode> _then;
	std::vector<ParsingNode> _else;

public:
	IfData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved);
	inline ~IfData() override {}
	ASTNode parse_keyword() const override;
};
class FnData : public KeywordData {
	Lexer::Token              _name;
	std::vector<Lexer::Token> _args;
	std::vector<ParsingNode>  _body;
	SymbolTable&              _symbols;

public:
	FnData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved);
	inline ~FnData() override {}
	ASTNode parse_keyword() const override;
};
struct ParsingNode {
	Lexer::Token    _token{nullptr};
	Operators::Type _op_type{Operators::Type::none};
	KeywordData*    _keyword_data{nullptr};
	SymbolTable*    _symbols{nullptr};

public:
	constexpr ParsingNode(const Lexer::Token& token, Operators::Type op_type, KeywordData* keyword_data,
	                      SymbolTable* symbols) :
	    _token(token), _op_type(op_type), _keyword_data(keyword_data), _symbols(symbols) {}
	constexpr ParsingNode(const ParsingNode& node) :
	    _token(node._token), _op_type(node._op_type), _keyword_data(node._keyword_data), _symbols(node._symbols) {}
	inline ~ParsingNode() {
		if (_keyword_data != nullptr)
			delete _keyword_data;
	}
	std::optional<uint32_t> precedence() const;
	friend std::ostream&    operator<<(std::ostream& stream, const ParsingNode& node);
};
// Keeps the semicolon at the end around
std::vector<std::vector<Lexer::Token>> split_by_statements(const std::vector<Lexer::Token>& code);
// Initializes memory, use delete
KeywordData* preparse_keyword(const Lexer::Token& keyword, const ::Lexer::Token* begin, const Lexer::Token* end,
                              SymbolTable& symbols, size_t& out_reserved);
// Expects no semicolon
std::vector<ParsingNode> preparse(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols);
} // namespace Parser
