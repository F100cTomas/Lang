#pragma once
#include "../Lexer/_lexer.hpp"
#include "../builtins.hpp"
#include "../symboltable.hpp"
#include <optional>
#include <iostream>
#include <vector>
namespace Parser {
struct ASTNode;
}
namespace Preparser {
using Parser::ASTNode;
struct ParsingNode;
class KeywordData {
public:
	inline virtual ~KeywordData() {};
	virtual ASTNode* parse_keyword(Symbol* symbol) const = 0;
};
class ParenData : public KeywordData {
	std::vector<Symbol*> _expression{};

public:
	ParenData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved);
	inline ~ParenData() override {}
	ASTNode* parse_keyword(Symbol* symbol) const override;
};
class ScopeData : public KeywordData {
	std::vector<std::vector<Symbol*>> _statements{};
	std::vector<SymbolTable*>         _scope{};

public:
	ScopeData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved);
	inline ~ScopeData() override {}
	ASTNode* parse_keyword(Symbol* symbol) const override;
};
class IfData : public KeywordData {
	std::vector<Symbol*> _cond{};
	std::vector<Symbol*> _then{};
	std::vector<Symbol*> _else{};
	SymbolTable*         _cond_scope{nullptr};
	SymbolTable*         _then_scope{nullptr};
	SymbolTable*         _else_scope{nullptr};

public:
	IfData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved);
	inline ~IfData() override {}
	ASTNode* parse_keyword(Symbol* symbol) const override;
};
class FnData : public KeywordData {
	Lexer::Token              _name;
	std::vector<Lexer::Token> _args;
	std::vector<Symbol*>      _body;
	SymbolTable&              _scope;

public:
	FnData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved, Symbol*& out_symbol);
	inline ~FnData() override {}
	ASTNode* parse_keyword(Symbol* symbol) const override;
};
class LetData : public KeywordData {
	Lexer::Token         _name;
	std::vector<Symbol*> _val;

public:
	LetData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved, Symbol*& out_symbol);
	inline ~LetData() override {}
	ASTNode* parse_keyword(Symbol* symbol) const override;
};
struct ParsingNode {
	Lexer::Token    _token{};
	Operators::Type _op_type{Operators::Type::none};
	KeywordData*    _keyword_data{nullptr};
	inline Symbol*  make_symbol(SymbolTable& symbol_table) {
    Symbol* symbol = new Symbol(*this);
    symbol_table.register_symbol(symbol);
    return symbol;
	}
	inline Symbol*  make_named_symbol(SymbolTable& symbol_table, const Lexer::Token& name) {
    Symbol* symbol = new Symbol(*this);
    symbol_table.register_named_symbol(symbol, name);
    return symbol;
	}
	std::optional<uint32_t> precedence() const;
	friend std::ostream& operator << (std::ostream& stream, const ParsingNode& node);
};
// Keeps the semicolon at the end around
std::vector<std::vector<Lexer::Token>> split_by_statements(const std::vector<Lexer::Token>& code);
// Initializes memory, use delete
Symbol* preparse_keyword(Symbol* const* begin_before, Symbol* const* end_before, const Lexer::Token* keyword,
                              const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved_before,
                              size_t& out_reserved_after);
// Expects no semicolon
std::vector<Symbol*> preparse(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols);
void                 run(const Lexer::Tokenized& code, SymbolTable& symbols);
} // namespace Preparser
