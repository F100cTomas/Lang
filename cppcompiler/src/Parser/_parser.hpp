#include "../Lexer/_lexer.hpp"
#include <cstdint>
#include <iostream>
#include <vector>

namespace Parser {
void print_operation(std::ostream& stream, void* operation);
enum class NodeType : uint8_t { Constant, Operation };
class ASTNode {
	void*    _data;
	NodeType _node_type;
public:
	ASTNode(void* value, NodeType node_type);
	~ASTNode();
	inline friend std::ostream& operator<<(std::ostream& stream, const ASTNode& node) {
		if (node._node_type == NodeType::Constant) {
			stream << reinterpret_cast<int64_t>(node._data);
		} else
			print_operation(stream, node._data);
		stream << ' ';
		return stream;
	}
};
class AST {
	ASTNode* _root;
public:
	AST(const std::vector<const char*>& line);
	~AST();
};
void run(const std::vector<const char*>& code);
} // namespace Parser
