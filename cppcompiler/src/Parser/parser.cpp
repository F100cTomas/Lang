#include "../error.hpp"
#include "_parser.hpp"
#include "symboltable.hpp"
#include <cstdint>
#include <cstring>
#include <vector>
namespace Parser {
ASTNode* ParenData::parse_keyword() const {
	return parse(_expression.data(), _expression.data() + _expression.size());
}
ASTNode* ScopeData::parse_keyword() const {
	ASTNode* op = new ASTNode("{");
	op->_args.reserve(op->_args.size() + 1);
	for (const std::vector<ParsingNode>& node: _statements)
		op->_args.emplace_back(parse(node.data(), node.data() + node.size()));
	op->_args.emplace_back(parse(_expression.data(), _expression.data() + _expression.size()));
	return op;
}
ASTNode* IfData::parse_keyword() const {
	ASTNode* op = new ASTNode("if");
	op->_args.reserve(3);
	op->_args.emplace_back(parse(_cond.data(), _cond.data() + _cond.size()));
	op->_args.emplace_back(parse(_then.data(), _then.data() + _then.size()));
	op->_args.emplace_back(parse(_else.data(), _else.data() + _else.size()));
	return op;
}
ASTNode* FnData::parse_keyword() const {
	ASTNode* op = new ASTNode("fn");
	op->_metadata = new FnMeta(_name, _args, _symbols);
	op->_args.emplace_back(parse(_body.data(), _body.data() + _body.size()));
	_symbols.define_symbol(_name, op);
	return op;
}
std::optional<uint32_t> ParsingNode::precedence() const {
	switch (_op_type) {
	case Operators::Type::prefix: return Operators::prefix_operator_precedence(_token);
	case Operators::Type::infix: return Operators::infix_operator_precedence(_token);
	case Operators::Type::postfix: return Operators::postfix_operator_precedence(_token);
	case Operators::Type::none:
	default: break;
	}
	return {};
}
std::ostream& operator<<(std::ostream& stream, const ParsingNode& node) {
	using Operators::Type;
	switch (node._op_type) {
	case Type::infix: stream << "\x1b[32m"; break;
	case Type::prefix: stream << "\x1b[33m"; break;
	case Type::postfix: stream << "\x1b[35m"; break;
	case Type::none: stream << "\x1b[31m"; break;
	case Type::keyword:
		if (node._keyword_data == nullptr)
			stream << "\x1b[36m";
		else
			stream << "\x1b[34m";
		break;
	}
	stream << node._token.get() << "\x1b[0m";
	return stream;
}
std::ostream& operator<<(std::ostream& stream, const ASTNode& op) {
	if (op._name == "{") {
		stream << "{ ";
		for (size_t i = 0; i < op._args.size() - 1; i++)
			stream << op._args[i] << " ; ";
		stream << op._args.back() << " }";
		return stream;
	}
	for (const ASTNode* arg: op._args)
		stream << *arg << ' ';
	if (op._args.size() == 1 && (op._name == "+" || op._name == "-"))
		stream << 'u';
	stream << (op._name == "" ? "()" : op._name.get());
	if (op._name == "fn") {
		FnMeta* meta = reinterpret_cast<FnMeta*>(op._metadata);
		if (meta == nullptr)
			ERROR("Unexpected nullptr");
		stream << ' ' << meta->_name.get() << '(';
		if (meta->_args.empty()) {
			stream << ')';
		} else {
			stream << meta->_args.front().get();
			for (size_t i = 1; i < meta->_args.size(); i++)
				stream << ", " << meta->_args[i].get();
			stream << ')';
		}
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
	for (const ASTNode* root: ast._statements)
		stream << *root << '\n';
	stream << ast._symbols;
	return stream;
}
ASTNode::~ASTNode() {
for (ASTNode* node : _args)
	delete node;
}
AST::~AST() {
for (ASTNode* node : _statements)
	delete node;
}
ASTNode* parse(const ParsingNode* begin, const ParsingNode* end) {
	using Operators::Type;
	if (begin >= end) {
		return new ASTNode("0");
	}
	if (begin == end - 1) {
		if (begin->_op_type == Type::none)
			return new ASTNode(begin->_token);
		;
		if (begin->_op_type == Type::keyword) {
			if (begin->_keyword_data == nullptr)
				ERROR("Unexpected nullptr");
			return begin->_keyword_data->parse_keyword();
		}
		ERROR("Syntax Error");
	}
	const ParsingNode* max = begin;
	for (const ParsingNode* current = begin; current < end; current++) {
		const bool is_max_invalid{max->_op_type == Type::none || max->_op_type == Type::keyword
		                          || (max->_op_type == Type::prefix && max != begin)
		                          || (max->_op_type == Type::postfix && max != end - 1)};
		const bool is_current_valid{current->_op_type == Type::infix
		                            || (current->_op_type == Type::prefix && current == begin)
		                            || (current->_op_type == Type::postfix && current == end - 1)};
		const bool is_current_better{current->precedence().value_or(0) > max->precedence().value_or(0)
		                             || (current->precedence().value_or(0) == max->precedence().value_or(0)
		                                 && current->precedence().value_or(0) % 2 == 1)};
		if (is_max_invalid || (is_current_valid && is_current_better))
			max = current;
	}
	ASTNode* op = new ASTNode(max->_token);
	if (max->_op_type == Type::infix || max->_op_type == Type::postfix)
		op->_args.emplace_back(parse(begin, max));
	if (max->_op_type == Type::infix || max->_op_type == Type::prefix)
		op->_args.emplace_back(parse(max + 1, end));
	return op;
}
std::unique_ptr<AST> run(const std::vector<Lexer::Token>& code) {
	std::vector<std::vector<Lexer::Token>> lines = split_by_statements(code);
	std::unique_ptr<AST>                   ast   = std::make_unique<AST>(lines.size());
	for (std::vector<Lexer::Token>& line: lines) {
		if (line.back() != ";")
			ERROR("Missing semicolon");
		line.pop_back();
		std::vector<ParsingNode> preparsed = preparse(line.data(), line.data() + line.size(), ast->get_symbol_table());
		/*
		for (const ParsingNode& node : preparsed)
		  std::cout << node;
		std::cout << '\n';
		*/
		ast->add_statement(preparsed);
	}
	return ast;
}
}; // namespace Parser
