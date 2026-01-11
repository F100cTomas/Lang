#include "module.hpp"
#include <cstring>
#include <iostream>

int main(int argc, const char* argv[]) {
	// TODO: Add compilation options
	Module module{"code"};
	IrFile ir = module.build();
	std::cout << module << '\n';
	std::cout << ir << '\n';
	if (argc > 1 && strcmp(argv[1], "-a") == 0) { // Assembly output
		AsmFile instructions = ir.make_asm();
		std::cout << instructions << '\n';
		instructions.commit("program.a");
		return 0;
	}
	ObjFile obj = ir.make_obj();
	obj.link_executable().commit("program");
}
