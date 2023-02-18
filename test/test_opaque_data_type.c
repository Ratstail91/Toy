#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"
#include "toy_interpreter.h"

#include "toy_console_colors.h"

#include "toy_memory.h"

#include "../repl/repl_tools.h"

#include <stdio.h>
#include <stdlib.h>

//supress the print output
static void noPrintFn(const char* output) {
	//NO OP
}

void error(char* msg) {
	printf("%s", msg);
	exit(-1);
}

//utilities
typedef struct ArbitraryData {
	int value;
} ArbitraryData;

static int produce(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	ArbitraryData* data = TOY_ALLOCATE(ArbitraryData, 1);
	data->value = 42;

	Toy_Literal o = TOY_TO_OPAQUE_LITERAL(data, 0);

	Toy_pushLiteralArray(&interpreter->stack, o);

	Toy_freeLiteral(o);

	return 1;
}

static int consume(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	Toy_Literal o = Toy_popLiteralArray(arguments);

	Toy_Literal idn = o;

	if (Toy_parseIdentifierToValue(interpreter, &o)) {
		Toy_freeLiteral(idn);
	}

	if (TOY_IS_OPAQUE(o) && ((ArbitraryData*)(TOY_AS_OPAQUE(o)))->value == 42) {
		ArbitraryData* data = (ArbitraryData*)TOY_AS_OPAQUE(o);

		TOY_FREE(ArbitraryData, data);

		//all went well
		Toy_freeLiteral(o);
		return 0;
	}

	printf(TOY_CC_ERROR "opaque failed: %d\n" TOY_CC_RESET, TOY_IS_OPAQUE(o));

	exit(-1);
	return -1;
}

int main() {
	{
		size_t size = 0;
		const char* source = (const char*)Toy_readFile("scripts/opaque-data-type.toy", &size);
		const unsigned char* tb = Toy_compileString(source, &size);
		free((void*)source);

		if (!tb) {
			return -1;
		}

		Toy_Interpreter interpreter;
		Toy_initInterpreter(&interpreter);

		Toy_injectNativeFn(&interpreter, "produce", produce);
		Toy_injectNativeFn(&interpreter, "consume", consume);

		//run teh script
		Toy_runInterpreter(&interpreter, tb, size);

		//clean up
		Toy_freeInterpreter(&interpreter);
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}

