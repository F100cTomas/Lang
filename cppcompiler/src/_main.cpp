#include "Lexer/_lexer.hpp"
#include "Parser/_parser.hpp"
#include "CodeGenerator/_codegenerator.hpp"
#include <fstream>
#include <iostream>
#include <sstream>

int main() {
	std::ifstream     file{"code.ferro"};
	std::stringstream str;
	str << file.rdbuf();
	std::vector<Lexer::Token> tokens = Lexer::run(str.str().c_str());
	/*
	for (const Lexer::Token& token: tokens)
	  std::cout << token << ' ';
	std::cout << '\n';
	*/
	Parser::AST ast = Parser::run(tokens);
	// std::cout << ast;
	std::unique_ptr<llvm::Module> module = CodeGenerator::run(ast);
	module->print(llvm::outs(), nullptr);
}
