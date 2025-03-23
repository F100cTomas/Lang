#include "_parser.hpp"
#include <iostream>
#include <vector>
namespace Parser {
namespace {
std::vector<std::vector<const char*>> partition_code(const std::vector<const char*>& code) {
	std::vector<std::vector<const char*>> out{{}};
	std::vector<char>                     stack{};
	for (const char* token : code) {
		out.back().push_back(token);
		switch (*token) {
		case '(':
		case '[':
		case '{':
			stack.push_back(*token);
			break;
		case ')':
			if (stack.back() == '(')
				stack.pop_back();
			break;
		case ']':
			if (stack.back() == '[')
				stack.pop_back();
			break;
		case '}':
			if (stack.back() == '{')
				stack.pop_back();
			break;
		case ';':
			if (stack.size() == 0) {
				out.push_back({});
			}
			break;
		default:
			break;
		}
	}
	out.pop_back();
	return out;
}
} // namespace
SymbolTable run(const std::vector<const char*>& code) {
	std::vector<std::vector<const char*>> lines = partition_code(code);
	for (const std::vector<const char*>& line : lines) {
		for (const char* token : line)
			std::cout << token << " ";
		std::cout << '\n';
	}
	return {};
}
}; // namespace Parser
