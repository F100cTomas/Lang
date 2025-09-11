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
} // namespace Parser
