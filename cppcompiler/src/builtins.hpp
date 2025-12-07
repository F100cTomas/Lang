#pragma once
#include "Lexer/_lexer.hpp"
#include <cstdint>
#include <optional>
namespace Operators {
// information about operators
enum class Type : uint8_t { undecided, none, infix, prefix, postfix };
namespace {
constexpr uint64_t hashfn(const char* str) {
	uint64_t hash = 14695981039346656037U;
	while (*str) {
		hash ^= static_cast<uint8_t>(*str++);
		hash *= 1099511628211U;
	}
	return hash;
}
} // namespace
constexpr std::optional<uint32_t> infix_operator_precedence(const Lexer::Token& token) {
	switch (hashfn(token)) {
	case hashfn("."): return 1;
	case hashfn(""):
	case hashfn("*"):
	case hashfn("/"):
	case hashfn("%"): return 3;
	case hashfn("+"):
	case hashfn("-"): return 5;
	case hashfn("^"): return 7;
	default: return {};
	}
}
constexpr std::optional<uint32_t> prefix_operator_precedence(const Lexer::Token& token) {
	switch (hashfn(token)) {
	case hashfn("+"):
	case hashfn("-"): return 2;
	default: return {};
	}
}
constexpr std::optional<uint32_t> postfix_operator_precedence(const Lexer::Token& token) {
	switch (hashfn(token)) {
	default: return {};
	}
}
constexpr bool is_operator(const Lexer::Token& token) {
	return infix_operator_precedence(token).has_value() || prefix_operator_precedence(token).has_value()
	       || postfix_operator_precedence(token).has_value();
}
constexpr bool is_keyword(const Lexer::Token& token) {
	switch (hashfn(token)) {
	case hashfn("("):
	case hashfn("{"):
	case hashfn("if"):
	case hashfn("fn"):
	case hashfn("let"): return true;
	default: return false;
	}
}
struct OperatorInfo {
	bool _none, _infix, _prefix, _postfix;

public:
	inline OperatorInfo(bool none, bool infix, bool prefix, bool postfix) :
	    _none(none), _infix(infix), _prefix(prefix), _postfix(postfix) {}
};
inline OperatorInfo decide_token(const Lexer::Token& token) {
	bool infix   = infix_operator_precedence(token).has_value();
	bool prefix  = prefix_operator_precedence(token).has_value();
	bool postfix = postfix_operator_precedence(token).has_value();
	bool none    = !(infix || prefix || postfix);
	return {none, infix, prefix, postfix};
}
} // namespace Operators
