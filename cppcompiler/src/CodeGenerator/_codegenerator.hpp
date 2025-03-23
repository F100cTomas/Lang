#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <memory>

namespace CodeGenerator {
inline std::unique_ptr<llvm::LLVMContext> llvm_context{};
inline std::unique_ptr<llvm::IRBuilder<>> ir_builder;
inline std::unique_ptr<llvm::Module> module;

}
