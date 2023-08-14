#include <stdlib.h>

#include "disassembler.h"

int main(int argc, const char* argv[]) {
	disassemble(argv[1]);
	return EXIT_SUCCESS;
}
