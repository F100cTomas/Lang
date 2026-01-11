#include "LlvmInterface/_llvminterface.hpp"
#include <filesystem>
#ifdef __MINGW64__
#include <windows.h>
#undef ERROR
#endif
#include "CodeGenerator/_codegenerator.hpp"
#include "Lexer/_lexer.hpp"
#include "Preparser/_preparser.hpp"
#include "module.hpp"
#include <fstream>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Type.h>
#include <llvm/Support/raw_ostream.h>
#include <sstream>
namespace {
std::string tmpfilename() {
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
	return o_file_name;
}
} // namespace
AsmFile::AsmFile(IrFile& file) : _filepath(tmpfilename()) {
	LlvmInterface::assemble(_filepath.c_str(), file);
}
AsmFile::~AsmFile() {
	if (_is_commited)
		return;
	std::remove(_filepath.c_str());
}
void AsmFile::commit(const char* filepath) {
	std::filesystem::copy_file(_filepath, filepath, std::filesystem::copy_options::overwrite_existing);
	_filepath    = filepath;
	_is_commited = true;
}
std::ostream& operator<<(std::ostream& stream, const AsmFile& asm_file) {
	stream << "operator<< not implemented";
	return stream;
}
ExeFile::ExeFile(ObjFile& file) : _filepath(tmpfilename()) {
	LlvmInterface::link(_filepath.c_str(), file);
}
ExeFile::~ExeFile() {
	if (_is_commited)
		return;
	std::remove(_filepath.c_str());
}
void ExeFile::commit(const char* filepath) {
	std::filesystem::copy_file(_filepath, filepath, std::filesystem::copy_options::overwrite_existing);
	_filepath    = filepath;
	_is_commited = true;
}
ObjFile::ObjFile(IrFile& file) : _filepath(tmpfilename()) {
	LlvmInterface::compile(_filepath.c_str(), file);
}
ObjFile::~ObjFile() {
	if (_is_commited)
		return;
	std::remove(_filepath.c_str());
}
void ObjFile::commit(const char* filepath) {
	std::filesystem::copy_file(_filepath, filepath, std::filesystem::copy_options::overwrite_existing);
	_filepath    = filepath;
	_is_commited = true;
}
ExeFile ObjFile::link_executable() {
	return *this;
}
IrFile::IrFile(CodeGenerator::LLVMState& state) :
    _is_commited(false), _context(&state.context()), _module(&state.module()) {}
IrFile::~IrFile() {
	if (_is_commited)
		return;
}
AsmFile IrFile::make_asm() {
	return *this;
}
ObjFile IrFile::make_obj() {
	return *this;
}
std::ostream& operator<<(std::ostream& stream, const IrFile& ir_file) {
	std::string              buffer{};
	llvm::raw_string_ostream llvm_stream{buffer};
	ir_file._module->print(llvm_stream, nullptr);
	stream << buffer;
	return stream;
}
Module::Module(const char* filepath) {
	std::ifstream     file{filepath};
	std::stringstream str{};
	str << file.rdbuf();
	Lexer::Tokenized tokens = Lexer::run(str.str().c_str());
	Preparser::run(tokens, _st);
}
Module::~Module() {}
IrFile Module::build() {
	for (Symbol* symbol: _st) {
		symbol->get_llvm_node(_state);
	}
	Symbol* main = _st["main"];
	if (main != nullptr)
		_state.add_exit_syscall(
		    _state.entry()->_builder->CreateCall(main->get_llvm_node(_state).get_fn()->_fn,
		                                         {llvm::ConstantInt::get(llvm::Type::getInt64Ty(_state.context()), 0)}));
	return _state;
}
IrFile Module::build(const std::vector<Lexer::Token>& symbol_names) {
	for (const Lexer::Token& name: symbol_names) {
		(void)_st[name]->get_llvm_node(_state);
	}
	_state.add_exit_syscall(llvm::ConstantInt::get(llvm::Type::getInt64Ty(_state.context()), 0));
	return _state;
}
std::ostream& operator<<(std::ostream& stream, const Module& module) {
	return stream << module._st;
}
