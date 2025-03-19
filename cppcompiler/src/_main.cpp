#include "Lexer/_lexer.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int main(int agrc, const char* const argv[]) {
	std::ifstream     file{"code.ferro"};
	std::stringstream str;
	str << file.rdbuf();
	std::vector<const char*> tokens = Lexer::run(str.str().c_str());
	for (const char* const token : tokens)
		std::cout << token << '\n';
}
