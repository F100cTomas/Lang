#ifdef __MINGW64__
#include <windows.h>
#include <memory>
#undef ERROR
#endif
#include "_llvminterface.hpp"
#include "../error.hpp"
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
void run(const CodeGenerator::LlvmState& llvm_state) {
	llvm::InitializeAllTargetInfos();
	llvm::InitializeAllTargets();
	llvm::InitializeAllTargetMCs();
	llvm::InitializeAllAsmParsers();
	llvm::InitializeAllAsmPrinters();
	std::string         triple_string = llvm::sys::getDefaultTargetTriple();
	std::string         error1;
	const llvm::Target* target = llvm::TargetRegistry::lookupTarget(triple_string, error1);
	if (!target)
		ERROR(error1);
	llvm::TargetOptions  opt;
	llvm::Triple         triple{triple_string};
	llvm::TargetMachine* machine = target->createTargetMachine(triple, "generic", "", opt, {});
	llvm_state._module->setDataLayout(machine->createDataLayout());
	std::error_code error2;
#ifdef __linux__
	char o_file_name[] = "/tmp/langoutputXXXXXX";
	mkstemp(o_file_name);
#elif defined(__MINGW64__)
	char o_file_name[MAX_PATH];
	{
		char tmp_path[MAX_PATH];
		// Note: The documentation advises GetTempPath2, but I can't seem to be able to link it.
		if (GetTempPathA(MAX_PATH, tmp_path) == 0) {
			// TODO: Add formatted Windows error message.
			ERROR("Could not get temporary path");
		}
		if (GetTempFileNameA(tmp_path, "", 0, o_file_name) == 0) {
			// TODO: Add formatted Windows error message.
			ERROR("Could not get temporary file name");
		}
	}
#endif
	std::cout << o_file_name << std::endl;
	llvm::raw_fd_ostream dest{o_file_name, error2, llvm::sys::fs::OF_None};
	if (error2)
		ERROR("Could not open file: ", error2.message());
	llvm::legacy::PassManager pass_manager;
	if (machine->addPassesToEmitFile(pass_manager, dest, nullptr, llvm::CodeGenFileType::ObjectFile))
		ERROR("LLVM error");
	pass_manager.run(*llvm_state._module);
	dest.flush();
#ifdef __linux__
	lld::lldMain({"ld.lld", o_file_name, "-o", "program"}, llvm::outs(), llvm::errs(), {{lld::Gnu, &lld::elf::link}});
#elif defined(__MINGW64__)
	std::string cmd = "lld-link.exe ";
	cmd += o_file_name;
	cmd += " /out:program.exe /SUBSYSTEM:CONSOLE kernel32.lib";
	std::cout << cmd << std::endl;
	system(cmd.c_str());
#endif
	std::remove(o_file_name);
}
} // namespace LlvmInterface