#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "interpreter.h"

#include "console_colors.h"

#include "memory.h"

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

static int produce(Interpreter* interpreter, LiteralArray* arguments) {
	ArbitraryData* data = ALLOCATE(ArbitraryData, 1);
	data->value = 42;

	Literal o = TO_OPAQUE_LITERAL(data, 0);

	pushLiteralArray(&interpreter->stack, o);

	freeLiteral(o);

	return 1;
}

static int consume(Interpreter* interpreter, LiteralArray* arguments) {
	Literal o = popLiteralArray(arguments);

	Literal idn = o;

	if (parseIdentifierToValue(interpreter, &o)) {
		freeLiteral(idn);
	}

	if (IS_OPAQUE(o) && ((ArbitraryData*)(AS_OPAQUE(o)))->value == 42) {
		ArbitraryData* data = (ArbitraryData*)AS_OPAQUE(o);

		FREE(ArbitraryData, data);

		//all went well
		freeLiteral(o);
		return 0;
	}

	printf(ERROR "opaque failed: %d\n" RESET, IS_OPAQUE(o));

	exit(-1);
	return -1;
}

int main() {
	{
		size_t size = 0;
		char* source = readFile("scripts/opaque-data-type.toy", &size);
		unsigned char* tb = compileString(source, &size);
		free((void*)source);

		if (!tb) {
			return -1;
		}

		Interpreter interpreter;
		initInterpreter(&interpreter);

		injectNativeFn(&interpreter, "produce", produce);
		injectNativeFn(&interpreter, "consume", consume);

		//run teh script
		runInterpreter(&interpreter, tb, size);

		//clean up
		freeInterpreter(&interpreter);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

