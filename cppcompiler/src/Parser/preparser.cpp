#include "_parser.hpp"
#include "../error.hpp"
#include <vector>
namespace Parser {
	std::vector<std::vector<Lexer::Token>> split_by_statements(const std::vector<Lexer::Token>& code) {
		std::vector<std::vector<Lexer::Token>> out{{}};
		size_t                                 indentation{0};
		for (const Lexer::Token& token : code) {
			out.back().push_back(token);
			if (token == "{") {
				indentation++;
				continue;
			}
			if (token == "}") {
				if (indentation == 0)
					ERROR("Inconsistent { }");
				indentation--;
				continue;
			}
			if (indentation == 0 && token == ";") {
				out.push_back({});
				continue;
			}
		}
		if (!out.back().empty())
			ERROR("Missing semicolon");
		out.pop_back();
		return out;
	}
	std::vector<ParsingNode> run_preparser(const std::vector<Lexer::Token>& expression) {
		using Operators::infix_operator_precedence, Operators::prefix_operator_precedence, Operators::postfix_operator_precedence, Operators::Type;
		size_t                    size = expression.size();
		std::vector<ParsingNode>  result{};
		result.reserve(size);
		size_t i = 0;
		for (; i < size; i++) {
			const auto   infix = infix_operator_precedence(expression[i]);
			const auto  prefix = prefix_operator_precedence(expression[i]);
			const auto postfix = postfix_operator_precedence(expression[i]);
			if (!prefix.has_value()) {
				if (infix.has_value() || postfix.has_value())
					ERROR("Unexpected non-prefix operator: ", expression[i]);
				result.emplace_back(expression[i], Type::none, nullptr);
				i++;
				break;
			}
			result.emplace_back(expression[i], Type::prefix, nullptr);
		}
		struct OperatorInfo {
			Lexer::Token _token;
			bool _is_infix, _is_prefix, _is_postfix;
			inline OperatorInfo(const Lexer::Token& token, bool is_infix, bool is_prefix, bool is_postfix) : _token(token), _is_infix(is_infix), _is_prefix(is_prefix), _is_postfix(is_postfix) {}
		};
		std::vector<OperatorInfo> stack{};
		for (; i < size; i++) {
			const Lexer::Token& token = expression[i];
			const auto   infix = infix_operator_precedence(token);
			const auto  prefix = prefix_operator_precedence(token);
			const auto postfix = postfix_operator_precedence(token);
			if (infix.has_value() || prefix.has_value() || postfix.has_value()) {
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
				result.emplace_back(token, Type::none, nullptr);
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
