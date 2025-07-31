#include "_parser.hpp"
#include "../error.hpp"
#include <vector>
namespace Parser {
	std::tuple<KeywordData*, size_t> KeywordData::scope(const Lexer::Token* begin, const Lexer::Token* end) {
		std::vector<Lexer::Token> stack{};
		ScopeData out{};
		size_t    layer{0};
		size_t    reserved{0};
		for (const Lexer::Token* current = begin + 1; current < end; current++) {
			if (layer == 0) {
				if (*current == ";") {
					out._statements.emplace_back(run_preparser(stack.data(), stack.data() + stack.size()));
					stack.clear();
					continue;
				}
				if (*current == "}") {
					reserved = current - begin;
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
		if (reserved == 0)
			ERROR("Inconsistent { }");
		out._expression = run_preparser(stack.data(), stack.data() + stack.size());
		return {new KeywordData(std::move(out)), reserved};
	}
	std::tuple<KeywordData*, size_t> KeywordData::if_keyword(const Lexer::Token* begin, const Lexer::Token* end) {
		if (end == begin + 1)
			ERROR("Syntax Error");
		{
			size_t layer{0};
			for (const Lexer::Token* current = begin; current < end; current++) {
				if (*current == "{" || *current == "(" || *current == "[") {
					layer++;
					continue;
				}
				if (*current == "}" || *current == ")" || *current == "]") {
					if (layer == 0) {
						end = current;
						break;
					}
					layer--;
				}
			}
		}
		const Lexer::Token* cond_begin{nullptr};
		const Lexer::Token* cond_end{nullptr};
		const Lexer::Token* then_begin{nullptr};
		const Lexer::Token* then_end{nullptr};
		bool is_if_else = false;
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
						cond_end = current;
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
					cond_end = current;
					then_begin = current;
					break;
				}
				if (*current == "(" ||  *current == "[") {
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
					then_end = current;
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
			if (*(then_end + 1) == "{") {
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
		IfData if_data;
		if_data._cond = run_preparser(cond_begin, cond_end);
		if_data._then = run_preparser(then_begin, then_end);
		if (is_if_else)
			if_data._else = run_preparser(then_end + 1, end);
		else {
			if_data._else = {};
			end = then_end;
		}
		return {new KeywordData(std::move(if_data)), end - begin - 1};
	}
	std::vector<std::vector<Lexer::Token>> split_by_statements(const std::vector<Lexer::Token>& code) {
		std::vector<std::vector<Lexer::Token>> out{{}};
		size_t                                 layer{0};
		for (const Lexer::Token& token : code) {
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
	std::vector<ParsingNode> run_preparser(const Lexer::Token* begin, const Lexer::Token* end) {
		using Operators::infix_operator_precedence, Operators::prefix_operator_precedence, Operators::postfix_operator_precedence, Operators::is_keyword, Operators::Type;
		size_t                    size = end - begin;
		std::vector<ParsingNode>  result{};
		result.reserve(size);
		size_t i = 0;
		for (; i < size; i++) {
			const Lexer::Token& token = begin[i];
			if (is_keyword(token)) {
				KeywordData* keyword_data{nullptr};
				size_t       reserved_tokens{0};
				if (token == "{") {
					const auto result = KeywordData::scope(begin + i, end);
					keyword_data = std::get<0>(result);
					reserved_tokens = std::get<1>(result);
				} else if (token == "if") {
					const auto result = KeywordData::if_keyword(begin + i, end);
					keyword_data = std::get<0>(result);
					reserved_tokens = std::get<1>(result);
				}
				result.emplace_back(token, Type::keyword, keyword_data);
				i += reserved_tokens + 1;
				break;
			}
			const auto   infix = infix_operator_precedence(token);
			const auto  prefix = prefix_operator_precedence(token);
			const auto postfix = postfix_operator_precedence(token);
			if (!prefix.has_value()) {
				if (infix.has_value() || postfix.has_value())
					ERROR("Unexpected non-prefix operator: ", token);
				result.emplace_back(token, Type::none, nullptr);
				i++;
				break;
			}
			result.emplace_back(token, Type::prefix, nullptr);
		}
		struct OperatorInfo {
			Lexer::Token _token;
			bool _is_infix, _is_prefix, _is_postfix;
			inline OperatorInfo(const Lexer::Token& token, bool is_infix, bool is_prefix, bool is_postfix) : _token(token), _is_infix(is_infix), _is_prefix(is_prefix), _is_postfix(is_postfix) {}
		};
		std::vector<OperatorInfo> stack{};
		for (; i < size; i++) {
			const Lexer::Token& token = begin[i];
			KeywordData* keyword_data{nullptr};
			size_t reserved_tokens{0};
			if (is_keyword(token)) {
				if (token == "{") {
					const auto result = KeywordData::scope(begin + i, end);
					keyword_data = std::get<0>(result);
					reserved_tokens = std::get<1>(result);
				} else if (token == "if") {
					const auto result = KeywordData::if_keyword(begin + i, end);
					keyword_data = std::get<0>(result);
					reserved_tokens = std::get<1>(result);
				}
			}
			const auto   infix = infix_operator_precedence(token);
			const auto  prefix = prefix_operator_precedence(token);
			const auto postfix = postfix_operator_precedence(token);
			if (reserved_tokens != 0 || infix.has_value() || prefix.has_value() || postfix.has_value()) {
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
						result.emplace_back(stack[j]._token, Type::postfix, nullptr);
					result.emplace_back(stack[j]._token, Type::infix, nullptr);
					for (j++; j < stack.size(); j++)
						if (!stack[j]._is_prefix)
							ERROR("Invalid operator order at operator: ", stack[j]._token);
						else
							result.emplace_back(stack[j]._token, Type::prefix, nullptr);
				} else {
					// implicit infix operator
					size_t j = 0;
					for (; j < stack.size() && stack[j]._is_postfix; j++)
						result.emplace_back(stack[j]._token, Type::postfix, nullptr);
					result.emplace_back("", Type::infix, nullptr);
					for (; j < stack.size(); j++)
						if (!stack[j]._is_prefix)
							ERROR("Invalid operator order at operator: ", stack[j]._token);
						else
							result.emplace_back(stack[j]._token, Type::prefix, nullptr);
				}
				stack.clear();
				if (keyword_data == nullptr)
					result.emplace_back(token, Type::none, nullptr);
				else {
					result.emplace_back(token, Type::keyword, keyword_data);
					i += reserved_tokens;
				}
			}
		}
		for (const OperatorInfo& info : stack)
			if (!info._is_postfix)
				ERROR("Unexpected non-postfix operator: ", info._token);
			else
				result.emplace_back(info._token, Type::postfix, nullptr);
		return result;
	}
}
