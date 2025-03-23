#include "../Lexer/_lexer.hpp"
#include <vector>

namespace Parser {
class Variable {
	const char* name;
};
class Function {
	const char* name;
};
class Expression {};
class SymbolTable {
	std::vector<Variable>   _variables{};
	std::vector<Function>   _functions{};
	std::vector<Expression> _expressions{};
};
SymbolTable run(const std::vector<const char*>& code);
} // namespace Parser
