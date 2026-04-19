#include "module.hpp"
#ifdef __MINGW64__
#define DEFAULT_OUTPUT_PATH "program.exe"
#else
#define DEFAULT_OUTPUT_PATH "program"
#endif
#define REQUIRED_ARGS \
	REQUIRED_STRING_ARG(input_file, "vstup", "Soubor ke kompilaci")
#define OPTIONAL_ARGS \
	OPTIONAL_STRING_ARG(output_file, const_cast<char*>(DEFAULT_OUTPUT_PATH), "-o", "výstup", "Název zkompilovaného souboru")
#define BOOLEAN_ARGS \
	BOOLEAN_ARG(asm_opt, "-a", "Assembly výstup") \
	BOOLEAN_ARG(debug, "-d", "Debug výstup") \
	BOOLEAN_ARG(help, "-h", "Seznam argumentů")
typedef bool _Bool;
#include "easyargs.h"
#include <iostream>
int main(int argc, char* argv[]) {
	// Parse CLI arguments
	args_t args = make_default_args();
	if (!parse_args(argc, argv, &args) || args.help) {
		print_help(argv[0]);
		return 1;
	}
	// Compile to LLVM IR
	Module module{args.input_file};
	IrFile ir = module.build();
	if (args.debug) { // Debug
		std::cout << module << '\n';
		std::cout << ir << '\n';
	}
	if (args.asm_opt) { // Assembly output
		AsmFile instructions = ir.make_asm();
		if (args.debug) // Debug
			std::cout << instructions << '\n';
		instructions.commit(args.output_file);
		return 0;
	}
	// Binary output
	ObjFile obj = ir.make_obj();
	obj.link_executable().commit(args.output_file);
}
