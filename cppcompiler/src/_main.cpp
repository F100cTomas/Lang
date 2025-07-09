#include "Lexer/_lexer.hpp"
#include "Parser/_parser.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int main() {
	std::ifstream     file{"code.ferro"};
	std::stringstream str;
	str << file.rdbuf();
	std::vector<const char*> tokens = Lexer::run(str.str().c_str());
	Parser::run(tokens);
}
