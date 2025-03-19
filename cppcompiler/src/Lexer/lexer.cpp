#include "_lexer.hpp"
#include <cstdint>
#include <cstring>

namespace Lexer {
namespace {
struct MapMember {
	MapMember* next = 0;
	char*      data = 0;
};
static uint64_t hashfn(const char* str) {
	uint64_t hash = 14695981039346656037U;
	while (*str) {
		hash ^= (uint8_t)*str++;
		hash *= 1099511628211U;
	}
	return hash;
}
MapMember token_map[256]{};
} // namespace
std::vector<const char*> run(const char* code) {
	std::vector<const char*> out{};
	for (const char* current = code; *current != '\0'; current++) {
		char token[2] = {*current, '\0'};
		out.push_back(make_token(token));
	}
	return out;
}
const char* make_token(const char* name) {
	uint64_t   hash    = hashfn(name);
	MapMember* current = token_map + (hash % 256);
	while (true) {
		if (current->data == nullptr) {
			size_t length = strlen(name);
			char*  buff   = new char[length + 1];
			strcpy(buff, name);
			current->data = buff;
			current->next = new MapMember();
			return buff;
			;
		}
		if (strcmp(current->data, name) == 0)
			return current->data;
		current = current->next;
	}
}
void free_token(const char* token) {
	uint64_t   hash    = hashfn(token);
	MapMember* current = token_map + (hash % 256);
	if (current->next == nullptr)
		return;
	if (strcmp(current->data, token) == 0) {
		MapMember* next = current->next;
		delete[] current->data;
		current->next = next->next;
		current->data = next->data;
		delete next;
		return;
	}
	while (current->next != nullptr) {
		if (strcmp(current->next->data, token) == 0) {
			MapMember* next = current->next;
			delete[] next->data;
			current->next = next->next;
			delete next;
			return;
		}
		current = current->next;
	}
}
void free_all_tokens() {
	for (size_t i = 0; i < 256; i++) {
		MapMember* const root           = token_map + i;
		while(root->next != nullptr) {
			delete[] root->data;
			root->data = root->next->data;
			MapMember* next = root->next->next;
			delete root->next;
			root->next = next;
		}
	}
}
} // namespace Lexer
