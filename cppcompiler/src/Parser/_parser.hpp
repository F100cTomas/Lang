#pragma once
#include "../Lexer/_lexer.hpp"
#include "../operators.hpp"
#include <iostream>
#include <vector>
#include <optional>
#include <variant>
#include <tuple>

namespace Parser {
struct ParsingNode;
class ASTNode;
struct ScopeData {
	std::vector<std::vector<ParsingNode>> _statements{};
	std::vector<ParsingNode> _expression{};
public:
	inline ScopeData() {}
	inline ScopeData(ScopeData&& scope_data) : _statements(std::move(scope_data._statements)), _expression(std::move(scope_data._expression)) {}
public:
	ASTNode parse_keyword() const;
};
struct IfData {
	std::vector<ParsingNode> _cond{};
	std::vector<ParsingNode> _then{};
	std::vector<ParsingNode> _else{};
public:
	inline IfData() {}
	inline IfData(IfData&& if_data) : _cond(std::move(if_data._cond)), _then(std::move(if_data._then)), _else(std::move(if_data._else)) {}
public:
	ASTNode parse_keyword() const;
};
struct FnData {};
class KeywordData {
public:
	std::variant<ScopeData, IfData, FnData> _data;
private:
	inline KeywordData(ScopeData&& scope_data) : _data(std::move(scope_data)) {}
	inline KeywordData(IfData&& if_data) : _data(std::move(if_data)) {}
	inline KeywordData(FnData&& fn_data) : _data(std::move(fn_data)) {}
public:
	static std::tuple<KeywordData*, size_t> scope(const Lexer::Token* begin, const Lexer::Token* end);
	static std::tuple<KeywordData*, size_t> if_keyword(const Lexer::Token* begin, const Lexer::Token* end);
public:
	ASTNode parse_keyword() const;
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
// Keeps the semicolon at the end around
std::vector<std::vector<Lexer::Token>> split_by_statements(const std::vector<Lexer::Token>& code);
// Expects no semicolon
std::vector<ParsingNode> run_preparser(const Lexer::Token* begin, const Lexer::Token* end);
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
struct Operation {
	Lexer::Token _operator;
	std::vector<ASTNode> _args{};
	inline Operation(const Lexer::Token& op) : _operator(op), _args() {}
	inline Operation(Operation&& operation) : _operator(operation._operator), _args(std::move(operation._args)) {}
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
public:
	AST(size_t statement_amount);
	void add_statement(const std::vector<ParsingNode>& expression);
	friend std::ostream& operator<<(std::ostream& stream, const AST& ast);
private:
};
// For internal use
ASTNode parse(const ParsingNode* begin, const ParsingNode* end);
// Main function of the parser
AST run(const std::vector<Lexer::Token>& code);
} // namespace Parser
