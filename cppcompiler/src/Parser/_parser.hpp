#pragma once
#include "../Lexer/_lexer.hpp"
#include "../operators.hpp"
#include <iostream>
#include <vector>
#include <optional>
#include <variant>

namespace Parser {
typedef void KeywordData;
struct ParsingNode {
	Lexer::Token    _token{nullptr};
	Operators::Type _op_type{Operators::Type::none};
	KeywordData*    _keyword_data{nullptr};
public:
	constexpr ParsingNode(const Lexer::Token& token, Operators::Type op_type, KeywordData* keyword_data) : _token(token), _op_type(op_type), _keyword_data(keyword_data) {}
	constexpr ParsingNode(const ParsingNode& node) : _token(node._token), _op_type(node._op_type), _keyword_data(node._keyword_data) {}
	std::optional<uint32_t> precedence() const;
};
// Keeps the semicolon at the end around
std::vector<std::vector<Lexer::Token>> split_by_statements(const std::vector<Lexer::Token>& code);
// Expects no semicolon
std::vector<ParsingNode> run_preparser(const std::vector<Lexer::Token>& expression);
class ASTNode;
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
	inline ASTNode(Value&& value) : _data(value) {}
	inline ASTNode(Operation&& operation) : _data(std::move(operation)) {}
	// inline ASTNode(const ASTNode& node) : _data(node._data) {}
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
	ASTNode parse(const ParsingNode* begin, const ParsingNode* end) const;
};
// Main function of the parser
AST run(const std::vector<Lexer::Token>& code);
} // namespace Parser
