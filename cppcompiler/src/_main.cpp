#include "CodeGenerator/_codegenerator.hpp"
#include "Lexer/_lexer.hpp"
#include "Parser/_parser.hpp"
#include "error.hpp"
#include <fstream>
#include <iostream>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <optional>
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
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();
	std::string         triple = llvm::sys::getDefaultTargetTriple();
	std::string         error1;
	const llvm::Target* target = llvm::TargetRegistry::lookupTarget(triple, error1);
	if (!target)
		ERROR(error1);
	llvm::TargetOptions  opt;
	llvm::TargetMachine* machine = target->createTargetMachine(triple, "generic", "", opt, {});
	llvm_state._module->setDataLayout(machine->createDataLayout());
	std::error_code      error2;
	llvm::raw_fd_ostream dest{"program.o", error2, llvm::sys::fs::OF_None};
	if (error2)
		ERROR("Could not open file: ", error2.message());
	llvm::legacy::PassManager pass_manager;
	if (machine->addPassesToEmitFile(pass_manager, dest, nullptr, llvm::CodeGenFileType::ObjectFile))
		ERROR("LLVM error");
	pass_manager.run(*llvm_state._module);
	dest.flush();
}
