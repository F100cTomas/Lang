#include "_parser.hpp"
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>
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
std::vector<std::vector<const char*>> partition_code(const std::vector<const char*>& code) {
	std::vector<std::vector<const char*>> out{{}};
	std::vector<char>                     stack{};
	for (const char* token : code) {
		out.back().push_back(token);
		switch (*token) {
		case '(':
		case '[':
		case '{':
			stack.push_back(*token);
			break;
		case ')':
			if (stack.back() == '(')
				stack.pop_back();
			break;
		case ']':
			if (stack.back() == '[')
				stack.pop_back();
			break;
		case '}':
			if (stack.back() == '{')
				stack.pop_back();
			break;
		case ';':
			if (stack.size() == 0) {
				out.push_back({});
			}
			break;
		default:
			break;
		}
	}
	if (out.back().empty())
		out.pop_back();
	return out;
}
uint32_t operator_precedence(const char* token) {
	switch (hashfn(token)) {
	case hashfn("."):
		return 1;
	case hashfn("*"):
	case hashfn("/"):
	case hashfn("%"):
		return 3;
	case hashfn("+"):
	case hashfn("-"):
		return 5;
	case hashfn("^"):
		return 7;
	default:
		return UINT32_MAX;
	}
}
bool isoperator(const char* token) { return operator_precedence(token) != UINT32_MAX; }
enum class OperatorType : uint8_t { none, infix, prefix, postfix };
struct OperatorNode {
	const char*  _token{nullptr};
	OperatorType _op_type{OperatorType::none};
	OperatorNode(const char* token, OperatorType op_type);
	uint32_t precedence() const;
};
OperatorNode::OperatorNode(const char* token, OperatorType op_type) : _token(token), _op_type(op_type) {}
uint32_t OperatorNode::precedence() const { return operator_precedence(_token); }
struct Operation {
	const char*           _op{nullptr};
	std::vector<ASTNode*> _args{};
	Operation(const char* op);
};
Operation::Operation(const char* op) : _op(op), _args() {}
ASTNode* parse(const OperatorNode* begin, const OperatorNode* end) {
	/*
	std::cout << '(';
	for (const OperatorNode* current = begin; current < end; current++)
		std::cout << current->_token;
	std::cout << ")\n";
	*/
	if (begin >= end) {
		return new ASTNode(0, NodeType::Constant);
	}
	if (begin == end - 1) {
		return new ASTNode(reinterpret_cast<void*>(atoi(begin->_token)), NodeType::Constant);
	}
	const OperatorNode* max = begin;
	for (const OperatorNode* current = begin; current < end; current++)
		if (current->_op_type != OperatorType::none && (max->_op_type == OperatorType::none
		        ? true
		        : (current->precedence() > max->precedence() ||
		           (current->precedence() == max->precedence() && current->precedence() % 2 == 1))))
			max = current;
	Operation* op = new Operation(max->_token);
	op->_args.push_back(parse(begin, max));
	op->_args.push_back(parse(max + 1, end));
	return new ASTNode(op, NodeType::Operation);
}
} // namespace
void print_operation(std::ostream& stream, void* operation) {
	Operation& op = *reinterpret_cast<Operation*>(operation);
	for (ASTNode* node : op._args)
		stream << *node;
	stream << op._op;
}
ASTNode::ASTNode(void* value, NodeType node_type) : _data(value), _node_type(node_type) {}
ASTNode::~ASTNode() {}
AST::AST(const std::vector<const char*>& line) : _root(nullptr) {
	// debug print
	/*
	for (const char* token : line)
		std::cout << token << " ";
	std::cout << '\n';
	*/
	// format data
	std::vector<OperatorNode> parsing_data{};
	parsing_data.reserve(line.size() - 1);
	for (size_t i = 0; i < line.size() - 1; i++)
		parsing_data.emplace_back(line[i], isoperator(line[i]) ? OperatorType::infix : OperatorType::none);
	// parse data
	_root = parse(parsing_data.data(), parsing_data.data() + parsing_data.size());
	std::cout << *_root << '\n';
}
AST::~AST() {}
void run(const std::vector<const char*>& code) {
	std::vector<std::vector<const char*>> lines = partition_code(code);
	std::vector<AST>                      ast_lines{};
	ast_lines.reserve(lines.size());
	for (const std::vector<const char*>& line : lines)
		ast_lines.push_back(line);
}
}; // namespace Parser
