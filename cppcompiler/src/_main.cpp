#include "module.hpp"
#include <cstring>
#include <iostream>
#define REQUIRED_ARGS \
	REQUIRED_STRING_ARG(input_file, "vstup", "Soubor ke kompilaci")
#define OPTIONAL_ARGS \
	OPTIONAL_STRING_ARG(output_file, const_cast<char*>("program"), "-o", "výstup", "Název zkompilovaného souboru")
#define BOOLEAN_ARGS \
	BOOLEAN_ARG(asm_opt, "-a", "Assembly výstup") \
	BOOLEAN_ARG(debug, "-d", "Debug výstup") \
	BOOLEAN_ARG(help, "-h", "Seznam argumentů")
typedef bool _Bool;
#include "easyargs.h"
int main(int argc, char* argv[]) {
	args_t args = make_default_args();
	if (!parse_args(argc, argv, &args) || args.help) {
		print_help(argv[0]);
		return 1;
	}
	Module module{args.input_file};
	IrFile ir = module.build();
	if (args.debug) {
		std::cout << module << '\n';
		std::cout << ir << '\n';
	}
	if (args.asm_opt) { // Assembly output
		AsmFile instructions = ir.make_asm();
		if (args.debug)
			std::cout << instructions << '\n';
		instructions.commit(args.output_file);
		return 0;
	}
	ObjFile obj = ir.make_obj();
	obj.link_executable().commit(args.output_file);
}
