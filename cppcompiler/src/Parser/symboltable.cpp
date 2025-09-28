#include "symboltable.hpp"
#include <cstdint>
namespace Parser {
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
SymbolTable::SymbolTable(SymbolTable* upper) : _upper_level(upper) {}
SymbolTable::~SymbolTable() {}
void SymbolTable::insert(const Lexer::Token& index, ASTNode* value) {
	uint8_t                  i   = hashfn(index) % 16;
	std::vector<TableValue>& row = _data[i];
	for (TableValue& item: row) {
		if (item._symbol == index.get()) {
			item._data = value;
			return;
		}
	}
	row.emplace_back(index, value);
}
ASTNode* SymbolTable::get(const Lexer::Token& index) const {
	uint8_t                        i   = hashfn(index) % 16;
	const std::vector<TableValue>& row = _data[i];
	for (const TableValue& item: row) {
		if (item._symbol == index.get())
			return item._data;
	}
	if (_upper_level == nullptr)
		return nullptr;
	return _upper_level->get(index);
}
bool SymbolTable::is_registered(const Lexer::Token& index) const {
	uint8_t                        i   = hashfn(index) % 16;
	const std::vector<TableValue>& row = _data[i];
	for (const TableValue& item: row) {
		if (item._symbol == index.get())
			return true;
	}
	if (_upper_level == nullptr)
		return false;
	return _upper_level->is_registered(index);
}
} // namespace Parser
