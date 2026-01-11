#include "../builtins.hpp"
#include "_lexer.hpp"
#include <cstdint>
#include <cstring>
#include <string>
#include <vector>
namespace Lexer {
namespace {
/*
 Lexer uses a hashmap to store the values of all tokens,
 this means that a token are just pointers to strings.
 */
constexpr uint64_t hashfn(const char* str) {
	uint64_t hash = 14695981039346656037U;
	while (*str) {
		hash ^= (uint8_t)*str++;
		hash *= 1099511628211U;
	}
	return hash;
}
struct MapMember {
	MapMember* next = 0;
	char*      data = 0;
};
MapMember   token_map[1024]{};
const char* make_token(const char* name) {
	uint64_t   hash    = hashfn(name);
	MapMember* current = token_map + (hash % 1024);
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
} // namespace
// Token class
Token::Token(const char* name) : _data(name == nullptr ? nullptr : make_token(name)) {}
// Main function of the lexer
Tokenized run(const char* code) {
	std::vector<Token> out{};
	std::string        partial{};
	for (const char* ptr = code; *ptr != '\0'; ptr++) {
		const char &c = ptr[0], &n = ptr[1];
		switch (classify(c)) {
		case CharType::alnum: {
			// Add to partial, if last alnum in row => flush
			partial.push_back(c);
			if (classify(n) != CharType::alnum) {
				out.emplace_back(partial.c_str());
				partial.clear();
			}
		} break;
		case CharType::empty:
			// do nothing
			break;
		case CharType::special:
			// different based on the character
			switch (c) {
				// strings
			case '\"':
			case '\'': {
				char beginning = c;
				partial.push_back(c);
				ptr++;
				while (*ptr != '\0' && *ptr != beginning) {
					const char &c = ptr[0], &n = ptr[1];
					if (c == '\\' && n == beginning) {
						partial.push_back(c);
						partial.push_back(n);
						ptr += 2;
						continue;
					}
					partial.push_back(c);
					ptr++;
				}
				partial.push_back(beginning);
				out.emplace_back(partial.c_str());
				partial.clear();
			} break;
			// $-notation: continues with a string of any characters
			case '$': {
				while (classify(*ptr) != CharType::empty) {
					partial.push_back(*ptr);
					ptr++;
				}
				out.emplace_back(partial.c_str());
				partial.clear();
			} break;
			// comments
			case '#':
				if (n == '#') {
					ptr += 2;
					while (*ptr != '\0' && *ptr != '\n')
						ptr++;
					break;
				} else if (n == '*') {
					ptr += 2;
					while (*ptr != '\0' && !(ptr[0] == '#' && ptr[-1] == '*'))
						ptr++;
					break;
				}
			// combinable operators (example: +() and ++())
			// merged to be separated in preparser (TODO)
			/*
			case '+':
			case '-':
				if (c == n) {
				  while (*ptr == c) {
				    partial.push_back(c);
				    ptr++;
				  }
				  ptr--;
				  out.emplace_back(partial.c_str());
				  partial.clear();
				  break;
				}
			*/
			// tries to merge the next tree or two characters into an operator,
			// failing that, it pushes the single character as a token
			default: {
				partial.push_back(c);
				partial.push_back(n);
				if (n != '\0') {
					partial.push_back(ptr[2]);
					if (Operators::is_operator(partial.c_str())) {
						out.emplace_back(partial.c_str());
						partial.clear();
						ptr += 2;
						break;
					}
					partial.pop_back();
				}
				if (Operators::is_operator(partial.c_str())) {
					out.emplace_back(partial.c_str());
					partial.clear();
					ptr++;
					break;
				}
				partial.clear();
				const char new_token[2]{c, '\0'};
				out.emplace_back(new_token);
			} break;
			}
			break;
		}
	}
	return out;
}
// deletes all tokens that share this value
void free_token(const char* token) {
	uint64_t   hash    = hashfn(token);
	MapMember* current = token_map + (hash % 1024);
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
// deletes all tokens
void free_all_tokens() {
	for (size_t i = 0; i < 1024; i++) {
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
