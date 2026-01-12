#include "symboltable.hpp"
#include "CodeGenerator/_codegenerator.hpp"
#include "Parser/_parser.hpp"
#include "Preparser/_preparser.hpp"
#include "error.hpp"
#include <cstdint>
#include <ostream>
namespace {
constexpr uint64_t hashfn(const char* str) {
	uint64_t hash = 14695981039346656037U;
	while (*str) {
		hash ^= (uint8_t)*str++;
		hash *= 1099511628211U;
	}
	return hash;
}
} // namespace
Symbol::Symbol(SymbolTable& table, Preparser::ParsingNode& parsing_node) :
    _table(&table), _parsing_node(&parsing_node) {
	table.register_symbol(this);
}
Symbol::Symbol(SymbolTable& table, Parser::ASTNode& ast_node) : _table(&table), _ast_node(&ast_node) {
	table.register_symbol(this);
}
Symbol::Symbol(SymbolTable& table, CodeGenerator::LLVMNode& llvm_node) : _table(&table), _llvm_node(&llvm_node) {
	table.register_symbol(this);
}
Symbol::~Symbol() {}
std::ostream& operator<<(std::ostream& stream, const Symbol& symbol) {
	if (symbol._parsing_node != nullptr)
		stream << *symbol._parsing_node << ' ';
	else
		stream << "[] ";
	if (symbol._ast_node != nullptr)
		stream << *symbol._ast_node << ' ';
	else
		stream << "[] ";
	if (symbol._llvm_node != nullptr)
		stream << *symbol._llvm_node << ' ';
	else
		stream << "[] ";
	return stream;
}
Preparser::ParsingNode& Symbol::get_parsing_node() const {
	if (_parsing_node == nullptr) {
		if (_successor != nullptr)
			return _successor->get_parsing_node();
		ERROR("Bad symbol");
	}
	return *_parsing_node;
}
Parser::ASTNode& Symbol::get_ast_node() {
	if (_ast_node == nullptr) {
		if (_successor != nullptr)
			return _successor->get_ast_node();
		Parser::ASTNode* result = Parser::run(this);
		if (_successor == nullptr) {
			_ast_node = result;
		}
		return *result;
	}
	return *_ast_node;
}
CodeGenerator::LLVMNode& Symbol::get_llvm_node(CodeGenerator::LLVMState&    llvm_state,
                                               CodeGenerator::LLVMFunction* function) {
	if (_llvm_node == nullptr || _llvm_function != function) {
		if (_successor != nullptr)
			return _successor->get_llvm_node(llvm_state, function);
		_llvm_node                      = nullptr;
		_llvm_function                  = function;
		CodeGenerator::LLVMNode* result = CodeGenerator::run(this, llvm_state, function);
		if (_successor == nullptr) {
			_llvm_node = result;
		}
		return *result;
	}
	return *_llvm_node;
}
CodeGenerator::LLVMNode& Symbol::get_llvm_node(CodeGenerator::LLVMState& llvm_state) {
	if (_llvm_node == nullptr || _llvm_function != llvm_state.entry()) {
		if (_successor != nullptr)
			return _successor->get_llvm_node(llvm_state);
		_llvm_node                      = nullptr;
		_llvm_function                  = llvm_state.entry();
		CodeGenerator::LLVMNode* result = CodeGenerator::run(this, llvm_state, llvm_state.entry());
		if (_successor == nullptr) {
			_llvm_node = result;
		}
		return *result;
	}
	return *_llvm_node;
}
SymbolTable::SymbolTable() {}
SymbolTable::SymbolTable(SymbolTable* upper) : _parent(upper) {
	if (upper == nullptr) {
		return;
	}
	upper->_children.push_back(this);
}
SymbolTable::~SymbolTable() {}
void SymbolTable::register_symbol(Symbol* symbol) {
	_values.push_back(symbol);
}
void SymbolTable::register_named_symbol(Symbol* symbol, const Lexer::Token& name) {
	_values.push_back(symbol);
	std::vector<TableValue>& row = _named_values[hashfn(name) % 16];
	row.emplace_back(name, symbol);
}
Symbol* SymbolTable::operator[](const Lexer::Token& name) {
	std::vector<TableValue>& row = _named_values[hashfn(name) % 16];
	for (const TableValue& item: row) {
		if (item._name == name.get()) {
			return item._symbol;
		}
	}
	if (_parent != nullptr)
		return (*_parent)[name];
	return nullptr;
}
std::ostream& operator<<(std::ostream& stream, const SymbolTable& st) {
	for (Symbol* value: st._values)
		stream << *value << '\n';
	stream << '\n';
	for (SymbolTable* child: st._children)
		stream << *child;
	return stream;
}
