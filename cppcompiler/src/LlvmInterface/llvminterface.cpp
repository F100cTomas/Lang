#ifdef __MINGW64__
#include <memory>
#include <windows.h>
#undef ERROR
#endif
#include "../error.hpp"
#include "_llvminterface.hpp"
#include <lld/Common/Driver.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/raw_ostream.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>
LLD_HAS_DRIVER(elf)
LLD_HAS_DRIVER(coff)
namespace LlvmInterface {
static bool initialized = false;
void init() {
	if (initialized)
		return;
	initialized = true;
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();
}
void assemble(const char* filepath, IrFile& file) {
	init();
	std::string         triple_string = llvm::sys::getDefaultTargetTriple();
	std::string         error1;
	const llvm::Target* target = llvm::TargetRegistry::lookupTarget(triple_string, error1);
	if (!target)
		ERROR(error1);
	llvm::TargetOptions  opt;
#ifdef __MINGW64__
	llvm::Triple         triple{"x86_64-pc-windows-msvc"};
#else
	llvm::Triple         triple{triple_string};
#endif
	llvm::TargetMachine* machine = target->createTargetMachine(triple, "generic", "", opt, {});
	file.module().setDataLayout(machine->createDataLayout());
	std::error_code      error2;
	llvm::raw_fd_ostream dest{filepath, error2, llvm::sys::fs::OF_None};
	if (error2)
		ERROR("Could not open file: ", error2.message());
	llvm::legacy::PassManager pass_manager;
	if (machine->addPassesToEmitFile(pass_manager, dest, nullptr, llvm::CodeGenFileType::AssemblyFile))
		ERROR("LLVM error");
	pass_manager.run(file.module());
	dest.flush();
}
void compile(const char* filepath, IrFile& file) {
	init();
	std::string         triple_string = llvm::sys::getDefaultTargetTriple();
	std::string         error1;
	const llvm::Target* target = llvm::TargetRegistry::lookupTarget(triple_string, error1);
	if (!target)
		ERROR(error1);
	llvm::TargetOptions  opt;
#ifdef __MINGW64__
	llvm::Triple         triple{"x86_64-pc-windows-msvc"};
#else
	llvm::Triple         triple{triple_string};
#endif
	llvm::TargetMachine* machine = target->createTargetMachine(triple, "generic", "", opt, {});
	file.module().setDataLayout(machine->createDataLayout());
	std::error_code      error2;
	llvm::raw_fd_ostream dest{filepath, error2, llvm::sys::fs::OF_None};
	if (error2)
		ERROR("Could not open file: ", error2.message());
	llvm::legacy::PassManager pass_manager;
	if (machine->addPassesToEmitFile(pass_manager, dest, nullptr, llvm::CodeGenFileType::ObjectFile))
		ERROR("LLVM error");
	pass_manager.run(file.module());
	dest.flush();
}
void link(const char* filepath, ObjFile& file) {
#ifdef __linux__
	lld::lldMain({"ld.lld", file.get_path(), "-o", filepath}, llvm::outs(), llvm::errs(), {{lld::Gnu, &lld::elf::link}});
#elif defined(__MINGW64__)
	std::string cmd = "lld-link.exe ";
	cmd += file.get_path();
	cmd += " /out:\"";
	cmd += filepath;
	cmd += "\" /NODEFAULTLIB /ENTRY:_start /SUBSYSTEM:CONSOLE kernel32.lib";
	std::cout << cmd << std::endl;
	system(cmd.c_str());
#endif
}
} // namespace LlvmInterface
