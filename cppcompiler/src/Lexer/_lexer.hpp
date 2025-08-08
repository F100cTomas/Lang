#pragma once
#include <cstring>
#include <vector>

namespace Lexer
{
class Token {
	const char* _data{nullptr};
public:
	inline Token() {}
	Token(const char* name);
	constexpr Token(const Token& token) : _data(token._data) {}
	constexpr operator const char*() const { return _data; }
	constexpr const char* get() const { return _data; }
	inline bool operator==(const char* str) const { return strcmp(_data, str) == 0; }
	friend inline bool operator==(const char* str, const Lexer::Token& token) { return token == str; }
	inline bool operator!=(const char* str) const { return strcmp(_data, str) != 0; }
	friend inline bool operator!=(const char* str, const Lexer::Token& token) { return token != str; }
};
// Main function of the lexer
std::vector<Token> run(const char* code);
// Lexer manages memory of tokens
void        free_token(const char* token);
void        free_all_tokens();
} // namespace Lexer
