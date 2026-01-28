#include "../error.hpp"
#include "../symboltable.hpp"
#include "_parser.hpp"
#include <cstdint>
#include <ostream>
#include <vector>
namespace Preparser {
using Parser::parse, Parser::FnMeta, Parser::LetMeta;
ASTNode* ParenData::parse_keyword(Symbol* symbol) const {
	Symbol* inside = parse(_expression.data(), _expression.data() + _expression.size());
	symbol->be_suceeded_by(inside);
	return &inside->get_ast_node();
}
ASTNode* ScopeData::parse_keyword(Symbol* symbol) const {
	ASTNode* op = new ASTNode("{");
	op->_args.reserve(_statements.size());
	for (const std::vector<Symbol*>& node: _statements)
		op->_args.push_back(parse(node.data(), node.data() + node.size()));
	return op;
}
ASTNode* IfData::parse_keyword(Symbol* symbol) const {
	ASTNode* op = new ASTNode("if");
	op->_args.reserve(3);
	op->_args.push_back(parse(_cond.data(), _cond.data() + _cond.size()));
	op->_args.push_back(parse(_then.data(), _then.data() + _then.size()));
	op->_args.push_back(parse(_else.data(), _else.data() + _else.size()));
	return op;
}
ASTNode* FnData::parse_keyword(Symbol* symbol) const {
	ASTNode* op   = new ASTNode("fn");
	op->_metadata = new FnMeta{_name, _args};
	op->_args.push_back(parse(_body.data(), _body.data() + _body.size()));
	return op;
}
ASTNode* LetData::parse_keyword(Symbol* symbol) const {
	ASTNode* op   = new ASTNode("let");
	op->_metadata = new LetMeta{_name};
	op->_args.push_back(parse(_val.data(), _val.data() + _val.size()));
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
} // namespace Preparser
namespace Parser {
std::ostream& operator<<(std::ostream& stream, const ASTNode& node) {
	stream << node._name.get() << '(';
	stream << node._args.size();
	return stream << ')';
}
Symbol* parse(Symbol* const* begin, Symbol* const* end) {
	using Operators::Type;
	if (begin >= end) {
		ERROR("Missing expression");
		return nullptr;
	}
	if (begin == end - 1) {
		return *begin;
	}
	Symbol* const* max = begin;
	for (Symbol* const* current = begin; current < end; current++) {
		ParsingNode& max_node     = (*max)->get_parsing_node();
		ParsingNode& current_node = (*current)->get_parsing_node();
		const bool   is_max_invalid{max_node._op_type == Type::none || (max_node._op_type == Type::prefix && max != begin)
                              || (max_node._op_type == Type::postfix && max != end - 1)};
		const bool   is_current_valid{current_node._op_type == Type::infix
                                || (current_node._op_type == Type::prefix && current == begin)
                                || (current_node._op_type == Type::postfix && current == end - 1)};
		const bool   is_current_better{current_node.precedence().value_or(0) > max_node.precedence().value_or(0)
                                 || (current_node.precedence().value_or(0) == max_node.precedence().value_or(0)
                                     && current_node.precedence().value_or(0) % 2 == 1)};
		if (is_max_invalid || (is_current_valid && is_current_better))
			max = current;
	}
	ASTNode& op = (*max)->get_ast_node();
	if ((*max)->get_parsing_node()._op_type == Type::infix || (*max)->get_parsing_node()._op_type == Type::postfix) {
		op._args.push_back(parse(begin, max));
	}
	if ((*max)->get_parsing_node()._op_type == Type::infix || (*max)->get_parsing_node()._op_type == Type::prefix) {
		op._args.push_back(parse(max + 1, end));
	}
	return *max;
}
ASTNode* run(Symbol* symbol) {
	ParsingNode& node = symbol->get_parsing_node();
	if (node._keyword_data != nullptr)
		return node._keyword_data->parse_keyword(symbol);
	return new ASTNode(node._token);
}
}; // namespace Parser
