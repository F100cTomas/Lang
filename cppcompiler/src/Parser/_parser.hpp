#pragma once
#include "../Lexer/_lexer.hpp"
#include "../builtins.hpp"
#include "symboltable.hpp"
#include <iostream>
#include <vector>
#include <optional>
#include <variant>

namespace Parser {
struct ParsingNode;
class ASTNode;
class KeywordData {
public:
	inline virtual ~KeywordData() {};
	virtual ASTNode parse_keyword() const = 0;
};
class ParenData : public KeywordData {
	std::vector<ParsingNode> _expression;
public:
	ParenData(const Lexer::Token* begin, const Lexer::Token* end, size_t& out_reserved);
	inline ~ParenData() override {}
	ASTNode parse_keyword() const override;
};
class ScopeData : public KeywordData {
	std::vector<std::vector<ParsingNode>> _statements;
	std::vector<ParsingNode> _expression;
public:
	ScopeData(const Lexer::Token* begin, const Lexer::Token* end, size_t& out_reserved);
	inline ~ScopeData() override {}
	ASTNode parse_keyword() const override;
};
class IfData : public KeywordData {
	std::vector<ParsingNode> _cond;
	std::vector<ParsingNode> _then;
	std::vector<ParsingNode> _else;
public:
	IfData(const Lexer::Token* begin, const Lexer::Token* end, size_t& out_reserved);
	inline ~IfData() override {}
	ASTNode parse_keyword() const override;
};
class FnData : public KeywordData {
	Lexer::Token _name;
	std::vector<Lexer::Token> _args;
	std::vector<ParsingNode> _body;
public:
	FnData(const Lexer::Token* begin, const Lexer::Token* end, size_t& out_reserved);
	inline ~FnData() override {}
	ASTNode parse_keyword() const override;
};
struct ParsingNode {
	Lexer::Token    _token{nullptr};
	Operators::Type _op_type{Operators::Type::none};
	KeywordData*    _keyword_data{nullptr};
public:
	constexpr ParsingNode(const Lexer::Token& token, Operators::Type op_type, KeywordData* keyword_data) : _token(token), _op_type(op_type), _keyword_data(keyword_data) {}
	constexpr ParsingNode(const ParsingNode& node) : _token(node._token), _op_type(node._op_type), _keyword_data(node._keyword_data) {}
	inline   ~ParsingNode() {
		if (_keyword_data != nullptr)
			delete _keyword_data;
	}
	std::optional<uint32_t> precedence() const;
	friend std::ostream& operator<<(std::ostream& stream, const ParsingNode& node);
};
struct Value {
	uint64_t _value;
	inline ~Value() {}
	inline static Value integer(int64_t value) {
		Value result;
		result._value = value;
		return result;
	}
	friend std::ostream& operator<<(std::ostream& stream, const Value& val);
};
struct FnMeta {
	Lexer::Token _name;
	std::vector<Lexer::Token> _args;
	inline FnMeta(Lexer::Token name, const std::vector<Lexer::Token> args): _name(name), _args(args) {}
};
struct Operation {
	Lexer::Token _operator;
	void*        _metadata{nullptr};
	std::vector<ASTNode> _args{};
	inline Operation(const Lexer::Token& op) : _operator(op), _args() {}
	inline Operation(Operation&& operation) : _operator(operation._operator), _metadata(operation._metadata), _args(std::move(operation._args)) {}
	inline ~Operation() {}
	friend std::ostream& operator<<(std::ostream& stream, const Operation& op);
};
class ASTNode {
	std::variant<Value, Operation> _data;
public:
	inline ASTNode(Value&& value) : _data(std::move(value)) {}
	inline ASTNode(Operation&& operation) : _data(std::move(operation)) {}
	inline ASTNode(ASTNode&& node) : _data(std::move(node._data)) {}
	friend std::ostream& operator<<(std::ostream& stream, const ASTNode& node);
};
class AST {
	std::vector<ASTNode> _statements {};
	SymbolTable          _symbols{};
public:
	AST(size_t statement_amount);
	void add_statement(const std::vector<ParsingNode>& expression);
	friend std::ostream& operator<<(std::ostream& stream, const AST& ast);
private:
};
// Keeps the semicolon at the end around
std::vector<std::vector<Lexer::Token>> split_by_statements(const std::vector<Lexer::Token>& code);
// Initializes memory, use delete
KeywordData* preparse_keyword(const Lexer::Token& keyword, const::Lexer::Token* begin, const Lexer::Token* end, size_t& out_reserved);
// Expects no semicolon
std::vector<ParsingNode> preparse(const Lexer::Token* begin, const Lexer::Token* end);
// For internal use
ASTNode parse(const ParsingNode* begin, const ParsingNode* end);
// Main function of the parser
AST run(const std::vector<Lexer::Token>& code);
} // namespace Parser
