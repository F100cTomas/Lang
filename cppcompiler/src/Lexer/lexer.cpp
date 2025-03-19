#include "_lexer.hpp"
#include <cctype>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_set>
#include <vector>
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
namespace {
enum class CharType : uint8_t { empty, alnum, special };
CharType classify(char c) {
	if ((uint8_t)c > 128)
		return CharType::alnum;
	if (c <= ' ')
		return CharType::empty;
	if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z') || (c == '_'))
		return CharType::alnum;
	return CharType::special;
}
std::unordered_set<std::string> two_ch_ops{
    "!=", "%=", "&=", "*=", "+=", "-=", "->", "/=", "<<", "<=", "==", "=>", ">=", ">>", "^=", "|="};
std::unordered_set<std::string> three_ch_ops{"<<=", "<=>", ">>="};
} // namespace
std::vector<const char*> run(const char* code) {
	std::vector<const char*> out{};
	std::string              partial{};
	for (const char* current = code; *current != '\0'; current++) {
		const char &c = current[0], n = current[1];
		switch (classify(c)) {
		case CharType::special:
			switch (c) {
			case '\"':
			case '\'': {
				char beginning = c;
				partial.push_back(c);
				current++;
				while (*current != '\0' && *current != beginning) {
					const char &c = current[0], n = current[1];
					if (c == '\\' && n == beginning) {
						partial.push_back(c);
						partial.push_back(n);
						current += 2;
						continue;
					}
					partial.push_back(c);
					current++;
				}
				partial.push_back(beginning);
				out.push_back(make_token(partial.c_str()));
				partial.erase();
			} break;
			case '$': {
				while (classify(*current) != CharType::empty) {
					partial.push_back(*current);
					current++;
				}
				out.push_back(make_token(partial.c_str()));
				partial.erase();
			} break;
			case '&':
			case '+':
			case '-':
			case '|':
				if (c == n) {
					while (*(current++) == c)
						partial.push_back(c);
					current--;
					out.push_back(make_token(partial.c_str()));
					partial.erase();
					break;
				}
			default: {
				partial.push_back(c);
				partial.push_back(n);
				if (n != '\0') {
					partial.push_back(current[2]);
					if (three_ch_ops.count(partial)) {
						out.push_back(make_token(partial.c_str()));
						partial.erase();
						current += 2;
						break;
					}
					partial.pop_back();
				}
				if (two_ch_ops.count(partial)) {
					out.push_back(make_token(partial.c_str()));
					partial.erase();
					current++;
					break;
				}
				partial.erase();
				const char new_token[2]{c, '\0'};
				out.push_back(make_token(new_token));
			} break;
			}
			break;
		case CharType::alnum: {
			partial.push_back(c);
			if (classify(n) != CharType::alnum) {
				out.push_back(make_token(partial.c_str()));
				partial.erase();
			}
		} break;
		case CharType::empty:
			break;
		}
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
		MapMember* const root = token_map + i;
		while (root->next != nullptr) {
			delete[] root->data;
			root->data      = root->next->data;
			MapMember* next = root->next->next;
			delete root->next;
			root->next = next;
		}
	}
}
} // namespace Lexer
