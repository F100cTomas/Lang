#include "../error.hpp"
#include "_preparser.hpp"
#include <ostream>
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
	SymbolTable*              scope_stack{&symbols};
	out_reserved = 0;
	for (const Lexer::Token* current = begin + 1; current < end; current++) {
		if (layer == 0) {
			if (*current == ";") {
				scope_stack = new SymbolTable(scope_stack);
				_scope.push_back(scope_stack);
				_statements.emplace_back(preparse(stack.data(), stack.data() + stack.size(), *scope_stack));
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
	scope_stack = new SymbolTable(scope_stack);
	_scope.push_back(scope_stack);
	_statements.emplace_back(preparse(stack.data(), stack.data() + stack.size(), *scope_stack));
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
	_cond_scope = new SymbolTable(&symbols);
	_cond       = preparse(cond_begin, cond_end, *_cond_scope);
	_then_scope = new SymbolTable(&symbols);
	_then       = preparse(then_begin, then_end, *_then_scope);
	if (is_if_else) {
		_else_scope = new SymbolTable(&symbols);
		_else       = preparse(then_end + 1, end, *_else_scope);
	} else {
		_else_scope = nullptr;
		_else       = {};
		end         = then_end;
	}
	out_reserved = end - begin - 1;
}
FnData::FnData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved,
               Symbol*& out_symbol) : _scope(*new SymbolTable(&symbols)) {
	out_reserved = end - begin - 1;
	if (out_reserved < 3)
		ERROR("Syntax Error");
	_name      = begin[1];
	out_symbol = (new ParsingNode{*begin, Operators::Type::undecided, this})->make_named_symbol(symbols, _name);
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
					_body = preparse(current, c + 1, _scope);
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
			_body = preparse(current + 1, end, _scope);
			return;
		}
	}
}
LetData::LetData(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved,
                 Symbol*& out_symbol) {
	out_reserved = end - begin - 1;
	if (out_reserved == 0)
		ERROR("Syntax Error");
	_name      = begin[1];
	out_symbol = (new ParsingNode{*begin, Operators::Type::undecided, this})->make_named_symbol(symbols, _name);
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
std::ostream& operator<<(std::ostream& stream, const ParsingNode& node) {
	stream << node._token;
	return stream;
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
Symbol* preparse_keyword(Symbol* const* begin_before, Symbol* const* end_before, const Lexer::Token* keyword,
                         const Lexer::Token* end, SymbolTable& symbols, size_t& out_reserved_before,
                         size_t& out_reserved_after) {
	using Operators::Type;
	out_reserved_before = 0;
	Symbol* symbol{nullptr};
	if (*keyword == "(")
		return (new ParsingNode{*keyword, Type::undecided, new ParenData(keyword, end, symbols, out_reserved_after)})
		    ->make_symbol(symbols);
	if (*keyword == "{")
		return (new ParsingNode{*keyword, Type::undecided, new ScopeData(keyword, end, symbols, out_reserved_after)})
		    ->make_symbol(symbols);
	if (*keyword == "if")
		return (new ParsingNode{*keyword, Type::undecided, new IfData(keyword, end, symbols, out_reserved_after)})
		    ->make_symbol(symbols);
	if (*keyword == "fn")
		return new FnData(keyword, end, symbols, out_reserved_after, symbol), symbol;
	if (*keyword == "let")
		return new LetData(keyword, end, symbols, out_reserved_after, symbol), symbol;
	ERROR("Unknown keyword: ", keyword->get());
	return nullptr;
}
std::vector<Symbol*> preparse(const Lexer::Token* begin, const Lexer::Token* end, SymbolTable& symbols) {
	using Operators::OperatorInfo, Operators::decide_token, Operators::is_keyword, Operators::Type;
	/*
	for (const Lexer::Token* c = begin; c < end; c++)
	  std::cout << c->get() << ' ';
	std::cout << std::endl;
	*/
	size_t               size = end - begin;
	std::vector<Symbol*> result{};
	result.reserve(size);
	for (const Lexer::Token* ptr = begin; ptr < end; ptr++) {
		const Lexer::Token& token{*ptr};
		if (!is_keyword(token)) {
			result.push_back((new ParsingNode{token, Type::undecided, nullptr})->make_symbol(symbols));
			continue;
		}
		size_t  out_reserved_before{0}, out_reserved_after{0};
		Symbol* keyword = preparse_keyword(result.data(), result.data() + result.size(), ptr, end, symbols,
		                                   out_reserved_before, out_reserved_after);
		while (out_reserved_before > 0) {
			result.pop_back();
			out_reserved_before--;
		}
		result.push_back(keyword);
		ptr += out_reserved_after;
	}
	std::vector<OperatorInfo> possibilities{};
	possibilities.reserve(size);
	bool charge{false}; /*
	  TODO: come up with a better name
	  true  means after Type::none  and before Type::infix.
	  false means after Type::infix and before Type::none.
	*/
	for (Symbol* const* ptr = result.data(); ptr < result.data() + result.size(); ptr++) {
		ParsingNode& node = (*ptr)->get_parsing_node();
		OperatorInfo info = decide_token(node._token);
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
			last_type                              = Type::none;
			result[i]->get_parsing_node()._op_type = Type::none;
			continue;
		}
		if ((last_type == Type::prefix || last_type == Type::infix) && info->_prefix) {
			last_type                              = Type::prefix;
			result[i]->get_parsing_node()._op_type = Type::prefix;
			continue;
		}
		if ((last_type == Type::postfix || last_type == Type::none) && info->_postfix) {
			last_type                              = Type::postfix;
			result[i]->get_parsing_node()._op_type = Type::postfix;
			continue;
		}
		if ((last_type == Type::postfix || last_type == Type::none) && info->_infix) {
			last_type                              = Type::infix;
			result[i]->get_parsing_node()._op_type = Type::infix;
			continue;
		}
		if (info->_none) {
			last_type                              = Type::none;
			result[i]->get_parsing_node()._op_type = Type::none;
			result.insert(result.begin() + i,
			              (new ParsingNode{Lexer::Token(""), Type::infix, nullptr})->make_symbol(symbols));
			i++;
			continue;
		}
		if (info->_prefix) {
			last_type                              = Type::prefix;
			result[i]->get_parsing_node()._op_type = Type::prefix;
			result.insert(result.begin() + i,
			              (new ParsingNode{Lexer::Token(""), Type::infix, nullptr})->make_symbol(symbols));
			i++;
			continue;
		}
		ERROR("Invalid expression token = \"", result[i]->get_parsing_node()._token, '\"',
		      (info->_none ? "none = true " : "none = false "), (info->_infix ? "infix = true " : "infix = false "),
		      (info->_prefix ? "prefix = true " : "prefix = false "),
		      (info->_postfix ? "postfix = true" : "postfix = false"));
	}
	for (Symbol* symbol: result) {
		if (symbol->get_parsing_node()._op_type == Type::undecided)
			ERROR("Preparser failed");
	}
	return result;
}
void run(const Lexer::Tokenized& code, SymbolTable& symbols) {
	std::vector<std::vector<Lexer::Token>> statements = Preparser::split_by_statements(code);
	for (std::vector<Lexer::Token>& statement: statements) {
		if (statement.back() != ";")
			ERROR("Missing semicolon");
		statement.pop_back();
		Preparser::preparse(statement.data(), statement.data() + statement.size(), symbols);
	}
}
} // namespace Preparser
