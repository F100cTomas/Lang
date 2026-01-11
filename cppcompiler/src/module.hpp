#pragma once
#include "CodeGenerator/_codegenerator.hpp"
#include "symboltable.hpp"
#include <iostream>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <string>
class IrFile;
class ObjFile;
class AsmFile {
	bool        _is_commited{false};
	std::string _filepath{};

public:
	AsmFile(IrFile& file);
	~AsmFile();
	inline const char* get_path() const {
		return _filepath.c_str();
	}
	void                 commit(const char* filepath);
	friend std::ostream& operator<<(std::ostream& stream, const AsmFile& asm_file);
};
class ExeFile {
	bool        _is_commited{false};
	std::string _filepath{};

public:
	ExeFile(ObjFile& file);
	~ExeFile();
	inline const char* get_path() const {
		return _filepath.c_str();
	}
	void commit(const char* filepath);
};
class ObjFile {
	bool        _is_commited{false};
	std::string _filepath{};

public:
	ObjFile(IrFile& file);
	~ObjFile();
	inline const char* get_path() const {
		return _filepath.c_str();
	}
	void    commit(const char* filepath);
	ExeFile link_executable();
};
class IrFile {
	bool               _is_commited{false};
	llvm::LLVMContext* _context{nullptr};
	llvm::Module*      _module{nullptr};

public:
	IrFile(CodeGenerator::LLVMState& state);
	~IrFile();
	inline llvm::LLVMContext& context() {
		return *_context;
	}
	inline llvm::Module& module() {
		return *_module;
	}
	AsmFile              make_asm();
	ObjFile              make_obj();
	friend std::ostream& operator<<(std::ostream& stream, const IrFile& ir_file);
};
class Module {
	SymbolTable              _st{nullptr};
	CodeGenerator::LLVMState _state{};

public:
	Module(const char* filepath);
	~Module();
	IrFile               build();
	IrFile               build(const std::vector<Lexer::Token>& symbol_names);
	friend std::ostream& operator<<(std::ostream& stream, const Module& module);
};
