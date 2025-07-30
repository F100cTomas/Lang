#include "_parser.hpp"
#include "../error.hpp"
#include <cstdint>
#include <cstring>
#include <variant>
#include <vector>
namespace Parser {
std::optional<uint32_t> ParsingNode::precedence() const {
	switch (_op_type) {
		case Operators::Type::prefix:
		return Operators::prefix_operator_precedence(_token);
		case Operators::Type::infix:
		return Operators::infix_operator_precedence(_token);
		case Operators::Type::postfix:
		return Operators::postfix_operator_precedence(_token);
		case Operators::Type::none:
	default:
		break;
	}
	return {};
}
std::ostream& operator<<(std::ostream& stream, const Value& val) {
	stream << static_cast<int64_t>(val._value);
	return stream;
}
std::ostream& operator<<(std::ostream& stream, const Operation& op) {
	for (const ASTNode& arg : op._args)
		stream << arg << ' ';
	if (op._args.size() == 1)
		stream << 'u';
	stream << (op._operator == "" ? "()" : op._operator.get());
	return stream;
}
std::ostream& operator<<(std::ostream& stream, const ASTNode& node) {
	if (std::holds_alternative<Value>(node._data)) {
		stream << std::get<Value>(node._data);
		return stream;
	}
	if (std::holds_alternative<Operation>(node._data)) {
		stream << std::get<Operation>(node._data);
		return stream;
	}
	return stream;
}
AST::AST(size_t statement_amount) : _statements() {
	_statements.reserve(statement_amount);
}
void AST::add_statement(const std::vector<ParsingNode>& expression) {
	_statements.emplace_back(parse(expression.data(), expression.data() + expression.size()));
}
std::ostream& operator<<(std::ostream& stream, const AST& ast) {
	for (const ASTNode& root : ast._statements)
		stream << root << '\n';
	return stream;
}
ASTNode AST::parse(const ParsingNode* begin, const ParsingNode* end) const {
	using Operators::Type;
	if (begin >= end) {
		return ASTNode(Value::integer(0));
	}
	if (begin == end - 1) {
		return ASTNode(Value::integer(atoi(begin->_token)));;
	}
	const ParsingNode* max   = begin;
	size_t             layer = 0;
	for (const ParsingNode* current = begin; current < end; current++) {
		if (current->_token == "(") {
			layer++;
			continue;
		}
		if (current->_token == ")") {
			if (layer == 0)
				ERROR("Inconsistent ( )");
			else
				layer--;
			continue;
		}
		const bool is_max_invalid{max->_op_type == Type::none || (max->_op_type == Type::prefix && max != begin) || (max->_op_type == Type::postfix && max != end - 1)};
		const bool is_current_valid{current->_op_type == Type::infix || (current->_op_type == Type::prefix && current == begin) || (current->_op_type == Type::postfix && current == end - 1)};
		const bool is_current_better{current->precedence().value_or(0) > max->precedence().value_or(0) || (current->precedence().value_or(0) == max->precedence().value_or(0) && current->precedence().value_or(0) % 2 == 1)};
		if (layer == 0 && (is_max_invalid || (is_current_valid && is_current_better)))
			max = current;
	}
	if (layer != 0)
		ERROR("Inconsistent ( )");
	if (max == begin && max->_token == "(") {
		// verify braces match
		layer = 1;
		for (const ParsingNode* current = begin + 1; current < end - 1; current++) {
			if (current->_token == "(") {
				layer++;
			} else if (current->_token == ")") {
				layer--;
				if (layer == 0) {
					ERROR("Inconsistent ( )");
				}
			}
		}
		return parse(begin + 1, end - 1);
	}
	Operation op = Operation(max->_token);
	if (max->_op_type == Type::infix || max->_op_type == Type::postfix)
		op._args.emplace_back(parse(begin, max));
	if (max->_op_type == Type::infix || max->_op_type == Type::prefix)
		op._args.emplace_back(parse(max + 1, end));
	return ASTNode(std::move(op));
}
AST run(const std::vector<Lexer::Token>& code) {
	std::vector<std::vector<Lexer::Token>> lines = split_by_statements(code);
	AST                                    ast{lines.size()};
	for (std::vector<Lexer::Token>& line : lines) {
		if (line.back() != ";")
			ERROR("Missing semicolon");
		line.pop_back();
		ast.add_statement(run_preparser(line));
	}
	return ast;
}
}; // namespace Parser
