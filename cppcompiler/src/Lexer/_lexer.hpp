#include <vector>

namespace Lexer
{
// Main function of the lexer
std::vector<const char*> run(const char* code);
// Lexer manages memory of tokens
const char* make_token(const char* name);
void        free_token(const char* token);
void        free_all_tokens();
} // namespace Lexer
