#include "../error.hpp"
#include "_preparser.hpp"
#include <vector>
namespace Preparser {
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
LetData::LetData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved) :
    _symbols(symbols) {
	out_reserved = end - begin - 1;
	if (out_reserved == 0)
		ERROR("Syntax Error");
	_name = begin[1];
	symbols.create_symbol(_name);
	if (out_reserved == 1) {
		_val = {};
		return;
	} else if (begin[2] != "=")
		ERROR("Syntax Error");
	_val = preparse(begin + 3, end, symbols);
	/*
	for (const ParsingNode& node: _val)
		std::cout << node << ' ';
	std::cout << std::endl;
	*/
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
KeywordData* preparse_keyword(const ParsingNode* begin_before, const ParsingNode* end_before,
                              const Lexer::Token* keyword, const Lexer::Token* end, SymbolTable& symbols,
                              size_t& out_reserved_before, size_t& out_reserved_after) {
	out_reserved_before = 0;
	if (*keyword == "(")
		return new ParenData(keyword, end, symbols, out_reserved_after);
	if (*keyword == "{")
		return new ScopeData(keyword, end, symbols, out_reserved_after);
	if (*keyword == "if")
		return new IfData(keyword, end, symbols, out_reserved_after);
	if (*keyword == "fn")
		return new FnData(keyword, end, symbols, out_reserved_after);
	if (*keyword == "let")
		return new LetData(keyword, end, symbols, out_reserved_after);
	ERROR("Unknown keyword: ", keyword->get());
	return nullptr;
}
std::vector<ParsingNode> preparse(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols) {
	using Operators::OperatorInfo, Operators::decide_token, Operators::is_keyword, Operators::Type;
	/*
	for (const Lexer::Token* c = begin; c < end; c++)
	  std::cout << c->get() << ' ';
	std::cout << std::endl;
	*/
	size_t                   size = end - begin;
	std::vector<ParsingNode> result{};
	result.reserve(size);
	for (const Lexer::Token* ptr = begin; ptr < end; ptr++) {
		const Lexer::Token& token{*ptr};
		if (!is_keyword(token)) {
			result.emplace_back(token, Type::undecided, nullptr, &symbols);
			continue;
		}
		size_t       out_reserved_before{0}, out_reserved_after{0};
		KeywordData* keyword_data = preparse_keyword(result.data(), result.data() + result.size(), ptr, end, symbols,
		                                             out_reserved_before, out_reserved_after);
		while (out_reserved_before > 0) {
			result.pop_back();
			out_reserved_before--;
		}
		result.emplace_back(token, Type::undecided, keyword_data, &symbols);
		ptr += out_reserved_after;
	}
	std::vector<OperatorInfo> possibilities{};
	possibilities.reserve(size);
	bool charge{false}; /*
	  TODO: come up with a better name
	  true  means after Type::none  and before Type::infix.
	  false means after Type::infix and before Type::none.
	*/
	for (const ParsingNode* ptr = result.data(); ptr < result.data() + result.size(); ptr++) {
		OperatorInfo info = decide_token(ptr->_token);
		info._postfix     = info._postfix && charge;
		info._infix       = info._infix && charge;
		if (charge && !info._postfix && !info._infix) {
			OperatorInfo& prev_info = possibilities.back();
			if (prev_info._infix) {
				prev_info = {false, true, false, false};
			} else if (prev_info._none && !prev_info._postfix) {
				info._postfix = info._postfix && charge;
				info._infix   = info._infix && charge;
			}
		}
		possibilities.push_back(info);
		charge = info._none || info._postfix;
	}
	if (possibilities.size() != 0 && (possibilities.back()._infix || possibilities.back()._prefix)) {
		ERROR("Invalid expression");
	}
	Type                last_type{Type::prefix};
	const OperatorInfo* info = possibilities.data();
	for (size_t i = 0; i < result.size(); i++, info++) {
		if ((last_type == Type::prefix || last_type == Type::infix) && info->_none) {
			last_type          = Type::none;
			result[i]._op_type = Type::none;
			continue;
		}
		if ((last_type == Type::prefix || last_type == Type::infix) && info->_prefix) {
			last_type          = Type::prefix;
			result[i]._op_type = Type::prefix;
			continue;
		}
		if ((last_type == Type::postfix || last_type == Type::none) && info->_postfix) {
			last_type          = Type::postfix;
			result[i]._op_type = Type::postfix;
			continue;
		}
		if ((last_type == Type::postfix || last_type == Type::none) && info->_infix) {
			last_type          = Type::infix;
			result[i]._op_type = Type::infix;
			continue;
		}
		if (info->_none) {
			last_type          = Type::none;
			result[i]._op_type = Type::none;
			result.insert(result.begin() + i, ParsingNode(Lexer::Token(""), Type::infix, nullptr, &symbols));
			i++;
			continue;
		}
		if (info->_prefix) {
			last_type          = Type::prefix;
			result[i]._op_type = Type::prefix;
			result.insert(result.begin() + i, ParsingNode(Lexer::Token(""), Type::infix, nullptr, &symbols));
			i++;
			continue;
		}
		ERROR("Invalid expression token = \"", result[i]._token, '\"', (info->_none ? "none = true " : "none = false "),
		      (info->_infix ? "infix = true " : "infix = false "), (info->_prefix ? "prefix = true " : "prefix = false "),
		      (info->_postfix ? "postfix = true" : "postfix = false"));
	}
	for (const ParsingNode& node: result)
		if (node._op_type == Type::undecided)
			ERROR("Preparser failed");
	return result;
}
} // namespace Preparser
