#include "preparser.hpp"
#include "../error.hpp"
#include "symboltable.hpp"
#include <vector>
namespace Parser {
ParenData::ParenData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved) {
	std::vector<Lexer::Token> stack{};
	size_t                    layer{0};
	out_reserved = 0;
	for (const Lexer::Token* current = begin + 1; current < end; current++) {
		if (layer == 0) {
			if (*current == ")") {
				out_reserved = current - begin;
				break;
			}
		}
		stack.push_back(*current);
		if (*current == "(") {
			layer++;
			continue;
		}
		if (*current == ")") {
			layer--;
			continue;
		}
	}
	if (out_reserved == 0)
		ERROR("Inconsistent ( )");
	_expression = preparse(stack.data(), stack.data() + stack.size(), symbols);
}
ScopeData::ScopeData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved) {
	std::vector<Lexer::Token> stack{};
	size_t                    layer{0};
	out_reserved = 0;
	for (const Lexer::Token* current = begin + 1; current < end; current++) {
		if (layer == 0) {
			if (*current == ";") {
				_statements.emplace_back(preparse(stack.data(), stack.data() + stack.size(), symbols));
				stack.clear();
				continue;
			}
			if (*current == "}") {
				out_reserved = current - begin;
				break;
			}
		}
		stack.push_back(*current);
		if (*current == "{") {
			layer++;
			continue;
		}
		if (*current == "}") {
			layer--;
			continue;
		}
	}
	if (out_reserved == 0)
		ERROR("Inconsistent { }");
	_expression = preparse(stack.data(), stack.data() + stack.size(), symbols);
}
IfData::IfData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved) {
	if (end == begin + 1)
		ERROR("Syntax Error");
	const Lexer::Token* cond_begin{nullptr};
	const Lexer::Token* cond_end{nullptr};
	const Lexer::Token* then_begin{nullptr};
	const Lexer::Token* then_end{nullptr};
	bool                is_if_else = false;
	if (*(begin + 1) == "(") {
		// if (<cond>) ...
		size_t layer{0};
		cond_begin = begin + 2;
		for (const Lexer::Token* current = cond_begin; current < end; current++) {
			if (*current == "(") {
				layer++;
				continue;
			}
			if (*current == ")") {
				if (layer == 0) {
					cond_end   = current;
					then_begin = current + 1;
					break;
				}
				layer--;
			}
		}
	} else {
		// if <cond> {<then>} ...
		cond_begin = begin + 1;
		size_t layer{0};
		for (const Lexer::Token* current = cond_begin; current < end; current++) {
			if (layer == 0 && *current == "{") {
				cond_end   = current;
				then_begin = current;
				break;
			}
			if (*current == "(" || *current == "[") {
				layer++;
				continue;
			}
			if (*current == ")" || *current == "]") {
				layer--;
			}
		}
	}
	if (then_begin == nullptr)
		ERROR("Syntax Error");
	if (*then_begin == "{") {
		// if ... {<then>} ...
		size_t layer{0};
		for (const Lexer::Token* current = then_begin + 1; current < end; current++) {
			if (*current == "{") {
				layer++;
				continue;
			}
			if (*current == "}") {
				if (layer == 0) {
					then_end = current + 1;
					break;
				}
				layer--;
			}
		}
		if (then_end == nullptr)
			ERROR("Syntax Error");
		if (*then_end == "else")
			is_if_else = true;
	} else {
		// if ... <then> ...
		size_t layer{0};
		for (const Lexer::Token* current = begin; current < end; current++) {
			if (layer == 0 && *current == "else") {
				then_end   = current;
				is_if_else = true;
				break;
			}
			if (*current == "{" || *current == "(" || *current == "[") {
				layer++;
				continue;
			}
			if (*current == "}" || *current == ")" || *current == "]") {
				layer--;
			}
		}
		if (then_end == nullptr)
			then_end = end;
	}
	if (is_if_else) {
		if (then_end[1] == "{") {
			// if ... {<else>}
			size_t layer{0};
			for (const Lexer::Token* current = then_end + 2; current < end; current++) {
				if (*current == "{") {
					layer++;
					continue;
				}
				if (*current == "}") {
					if (layer == 0) {
						end = current + 1;
						break;
					}
					layer--;
				}
			}
		} else {
			// if ... <else>
		}
	}
	_cond = preparse(cond_begin, cond_end, symbols);
	_then = preparse(then_begin, then_end, symbols);
	if (is_if_else)
		_else = preparse(then_end + 1, end, symbols);
	else {
		_else = {};
		end   = then_end;
	}
	out_reserved = end - begin - 1;
}
FnData::FnData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved) :
    _symbols(symbols) {
	out_reserved = end - begin - 1;
	if (out_reserved < 3)
		ERROR("Syntax Error");
	_name = begin[1];
	symbols.create_symbol(_name);
	if (begin[2] != "(")
		ERROR("Missing ( )");
	const Lexer::Token* current = begin + 3;
	for (; *current != ")"; current++) {
		if (current == end)
			ERROR("Syntax Error");
		_args.push_back(*current);
		current++;
		if (*current != ",") {
			if (*current == ")")
				break;
			else
				ERROR("Syntax Error");
		}
	}
	for (; current < end; current++) {
		if (*current == "{") {
			// fn ... { <body }
			size_t layer{0};
			for (const Lexer::Token* c = current + 1; c < end; c++) {
				if (layer == 0 && *c == "}") {
					_body = preparse(current, c + 1, symbols);
					return;
				}
				if (*c == "{") {
					layer++;
					continue;
				}
				if (*c == "}") {
					layer--;
					continue;
				}
			}
			ERROR("Inconsistent { }");
		}
		if (*current == ":") {
			// fn ... : <body>
			_body = preparse(current + 1, end, symbols);
			return;
		}
	}
}
std::vector<std::vector<Lexer::Token>> split_by_statements(const std::vector<Lexer::Token>& code) {
	std::vector<std::vector<Lexer::Token>> out{{}};
	size_t                                 layer{0};
	for (const Lexer::Token& token: code) {
		out.back().push_back(token);
		if (token == "{") {
			layer++;
			continue;
		}
		if (token == "}") {
			if (layer == 0)
				ERROR("Inconsistent { }");
			layer--;
			continue;
		}
		if (layer == 0 && token == ";") {
			out.push_back({});
			continue;
		}
	}
	if (!out.back().empty())
		ERROR("Missing semicolon");
	out.pop_back();
	return out;
}
KeywordData* preparse_keyword(const Lexer::Token& keyword, const ::Lexer::Token* begin, const Lexer::Token* end,
                              SymbolTable& symbols, size_t& out_reserved) {
	if (keyword == "(")
		return new ParenData(begin, end, symbols, out_reserved);
	if (keyword == "{")
		return new ScopeData(begin, end, symbols, out_reserved);
	if (keyword == "if")
		return new IfData(begin, end, symbols, out_reserved);
	if (keyword == "fn")
		return new FnData(begin, end, symbols, out_reserved);
	ERROR("Unknown keyword: ", keyword.get());
	return nullptr;
}
std::vector<ParsingNode> preparse(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols) {
	using Operators::infix_operator_precedence, Operators::prefix_operator_precedence,
	    Operators::postfix_operator_precedence, Operators::is_keyword, Operators::Type;
	size_t                   size = end - begin;
	std::vector<ParsingNode> result{};
	result.reserve(size);
	size_t i = 0;
	for (; i < size; i++) {
		const Lexer::Token& token = begin[i];
		if (is_keyword(token)) {
			size_t       reserved_tokens{0};
			KeywordData* keyword_data = preparse_keyword(token, begin + i, end, symbols, reserved_tokens);
			result.emplace_back(token, Type::keyword, keyword_data, &symbols);
			i += reserved_tokens + 1;
			break;
		}
		const auto infix   = infix_operator_precedence(token);
		const auto prefix  = prefix_operator_precedence(token);
		const auto postfix = postfix_operator_precedence(token);
		if (!prefix.has_value()) {
			if (infix.has_value() || postfix.has_value())
				ERROR("Unexpected non-prefix operator: ", token.get());
			result.emplace_back(token, Type::none, nullptr, &symbols);
			i++;
			break;
		}
		result.emplace_back(token, Type::prefix, nullptr, &symbols);
	}
	struct OperatorInfo {
		Lexer::Token _token;
		bool         _is_infix, _is_prefix, _is_postfix;
		inline OperatorInfo(const Lexer::Token& token, bool is_infix, bool is_prefix, bool is_postfix) :
		    _token(token), _is_infix(is_infix), _is_prefix(is_prefix), _is_postfix(is_postfix) {}
	};
	std::vector<OperatorInfo> stack{};
	for (; i < size; i++) {
		const Lexer::Token& token = begin[i];
		KeywordData*        keyword_data{nullptr};
		size_t              reserved_tokens{0};
		if (is_keyword(token)) {
			if (token == "(")
				keyword_data = preparse_keyword(token, begin + i, end, symbols, reserved_tokens);
		}
		const auto infix   = infix_operator_precedence(token);
		const auto prefix  = prefix_operator_precedence(token);
		const auto postfix = postfix_operator_precedence(token);
		if (keyword_data == nullptr && (infix.has_value() || prefix.has_value() || postfix.has_value())) {
			stack.emplace_back(token, infix.has_value(), prefix.has_value(), postfix.has_value());
		} else {
			int64_t infix_candidate = -1;
			for (size_t j = 0; j < stack.size(); j++)
				if (!stack[j]._is_postfix) {
					if (stack[j]._is_infix)
						infix_candidate = j;
					break;
				} else if (stack[j]._is_infix)
					infix_candidate = j;
			if (infix_candidate != -1) {
				// explicit infix operator
				size_t j = 0;
				for (; j < infix_candidate; j++)
					result.emplace_back(stack[j]._token, Type::postfix, nullptr, &symbols);
				result.emplace_back(stack[j]._token, Type::infix, nullptr, &symbols);
				for (j++; j < stack.size(); j++)
					if (!stack[j]._is_prefix)
						ERROR("Invalid operator order at operator: ", stack[j]._token);
					else
						result.emplace_back(stack[j]._token, Type::prefix, nullptr, &symbols);
			} else {
				// implicit infix operator
				size_t j = 0;
				for (; j < stack.size() && stack[j]._is_postfix; j++)
					result.emplace_back(stack[j]._token, Type::postfix, nullptr, &symbols);
				result.emplace_back("", Type::infix, nullptr, &symbols);
				for (; j < stack.size(); j++)
					if (!stack[j]._is_prefix)
						ERROR("Invalid operator order at operator: ", stack[j]._token);
					else
						result.emplace_back(stack[j]._token, Type::prefix, nullptr, &symbols);
			}
			stack.clear();
			if (keyword_data == nullptr)
				result.emplace_back(token, Type::none, nullptr, &symbols);
			else {
				result.emplace_back(token, Type::keyword, keyword_data, &symbols);
				i += reserved_tokens;
			}
		}
	}
	for (const OperatorInfo& info: stack)
		if (!info._is_postfix)
			ERROR("Unexpected non-postfix operator: ", info._token);
		else
			result.emplace_back(info._token, Type::postfix, nullptr, &symbols);
	return result;
}
} // namespace Parser
