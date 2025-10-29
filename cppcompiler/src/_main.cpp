#include "CodeGenerator/_codegenerator.hpp"
#include "Lexer/_lexer.hpp"
#include "LlvmInterface/_llvminterface.hpp"
#include "Parser/_parser.hpp"
#include <fstream>
#include <iostream>
#include <sstream>
int main() {
	std::ifstream     file{"code"};
	std::stringstream str;
	str << file.rdbuf();
	std::vector<Lexer::Token> tokens = Lexer::run(str.str().c_str());
	for (const Lexer::Token& token: tokens)
		std::cout << token << ' ';
	std::cout << '\n';
	std::unique_ptr<Parser::AST> ast = Parser::run(tokens);
	std::cout << *ast << std::flush;
	CodeGenerator::LlvmState llvm_state = CodeGenerator::run(*ast);
	llvm_state._module->print(llvm::outs(), nullptr);
	LlvmInterface::run(llvm_state);
}
