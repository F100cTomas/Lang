#pragma once
#include <iostream>
#define ERROR(...) ::Error::throw_error(__FILE__, __LINE__, __VA_ARGS__)
namespace Error {
inline void error_msg() {
	std::cerr << "\x1b[0m\n";
}
template<typename T1, typename... T> void error_msg(T1 t1, T... t);
template<typename... T> [[noreturn]] void throw_error(const char* file_name, size_t line_number, T... msg);
template<typename... T> [[noreturn]] void throw_error(const char* file_name, size_t line_number, T... msg) {
	std::cerr << "\x1b[31mError in file " << file_name << " at line " << line_number << ": \n";
	error_msg(msg...);
	std::cerr << std::flush;
	exit(1);
}
template<typename T1, typename... T> void error_msg(T1 t1, T... t) {
	std::cerr << t1;
	error_msg(t...);
}
} // namespace Error
