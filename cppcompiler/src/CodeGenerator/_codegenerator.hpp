#pragma once
#include "../Parser/_parser.hpp"
#include <memory>
#include "llvm/IR/Module.h"

namespace CodeGenerator {
	// llvm::Value* codegen();
	std::unique_ptr<llvm::Module> run(const Parser::AST& ast);
}
