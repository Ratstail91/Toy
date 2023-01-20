#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "interpreter.h"

#include "console_colors.h"

#include "memory.h"

#include "../repl/repl_tools.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

//supress the print output
static void noPrintFn(const char* output) {
	//NO OP
}

void error(char* msg) {
	printf("%s", msg);
	exit(-1);
}

int main() {
	{
		size_t size = 0;
		char* source = readFile("scripts/call-from-host.toy", &size);
		unsigned char* tb = compileString(source, &size);
		free((void*)source);

		if (!tb) {
			return -1;
		}

		Interpreter interpreter;
		initInterpreter(&interpreter);
		runInterpreter(&interpreter, tb, size);

		//test answer
		{
			interpreter.printOutput("Testing answer");

			LiteralArray arguments;
			initLiteralArray(&arguments);
			LiteralArray returns;
			initLiteralArray(&returns);

			callFn(&interpreter, "answer", &arguments, &returns);

			//check the results
			if (arguments.count != 0) {
				error("Arguments has the wrong number of members\n");
			}

			if (returns.count != 1) {
				error("Returns has the wrong number of members\n");
			}

			if (!IS_INTEGER(returns.literals[0]) || AS_INTEGER(returns.literals[0]) != 42) {
				error("Returned value is incorrect\n");
			}

			freeLiteralArray(&arguments);
			freeLiteralArray(&returns);
		}

		//test identity
		{
			interpreter.printOutput("Testing identity");

			LiteralArray arguments;
			initLiteralArray(&arguments);
			LiteralArray returns;
			initLiteralArray(&returns);

			//push an argument
			float pi = 3.14;
			Literal arg = TO_FLOAT_LITERAL(pi);
			pushLiteralArray(&arguments, arg);

			callFn(&interpreter, "identity", &arguments, &returns);

			//check the results
			if (arguments.count != 0) {
				error("Arguments has the wrong number of members\n");
			}

			if (returns.count != 1) {
				error("Returns has the wrong number of members\n");
			}

			float epsilon = 0.1; //because floats are evil

			if (!IS_FLOAT(returns.literals[0]) || fabs(AS_FLOAT(returns.literals[0]) - pi) > epsilon) {
				error("Returned value is incorrect\n");
			}

			freeLiteralArray(&arguments);
			freeLiteralArray(&returns);
		}

		//test makeCounter (closures)
		{
			interpreter.printOutput("Testing makeCounter (closures)");

			LiteralArray arguments;
			initLiteralArray(&arguments);
			LiteralArray returns;
			initLiteralArray(&returns);

			callFn(&interpreter, "makeCounter", &arguments, &returns);

			//check the results
			if (arguments.count != 0) {
				error("Arguments has the wrong number of members\n");
			}

			if (returns.count != 1) {
				error("Returns has the wrong number of members\n");
			}

			//grab the resulting literal
			Literal counter = popLiteralArray(&returns);

			freeLiteralArray(&arguments);
			freeLiteralArray(&returns);

			//call counter repeatedly
			{
				LiteralArray arguments;
				initLiteralArray(&arguments);
				LiteralArray returns;
				initLiteralArray(&returns);

				callLiteralFn(&interpreter, counter, &arguments, &returns);

				//check the results
				if (arguments.count != 0) {
					error("Arguments (1) has the wrong number of members\n");
				}

				if (returns.count != 1) {
					error("Returns (1) has the wrong number of members\n");
				}

				if (!IS_INTEGER(returns.literals[0]) || AS_INTEGER(returns.literals[0]) != 1) {
					error("Returned value (1) is incorrect\n");
				}

				freeLiteralArray(&arguments);
				freeLiteralArray(&returns);
			}

			{
				LiteralArray arguments;
				initLiteralArray(&arguments);
				LiteralArray returns;
				initLiteralArray(&returns);

				callLiteralFn(&interpreter, counter, &arguments, &returns);

				//check the results
				if (arguments.count != 0) {
					error("Arguments (2) has the wrong number of members\n");
				}

				if (returns.count != 1) {
					error("Returns (2) has the wrong number of members\n");
				}

				if (!IS_INTEGER(returns.literals[0]) || AS_INTEGER(returns.literals[0]) != 2) {
					error("Returned value (2) is incorrect\n");
				}

				freeLiteralArray(&arguments);
				freeLiteralArray(&returns);
			}

			{
				LiteralArray arguments;
				initLiteralArray(&arguments);
				LiteralArray returns;
				initLiteralArray(&returns);

				callLiteralFn(&interpreter, counter, &arguments, &returns);

				//check the results
				if (arguments.count != 0) {
					error("Arguments (3) has the wrong number of members\n");
				}

				if (returns.count != 1) {
					error("Returns (3) has the wrong number of members\n");
				}

				if (!IS_INTEGER(returns.literals[0]) || AS_INTEGER(returns.literals[0]) != 3) {
					error("Returned value (3) is incorrect\n");
				}

				freeLiteralArray(&arguments);
				freeLiteralArray(&returns);
			}

			freeLiteral(counter);
		}

		//test assertion failure
		{
			interpreter.printOutput("Testing assertion failure");

			setInterpreterAssert(&interpreter, noPrintFn);

			LiteralArray arguments;
			initLiteralArray(&arguments);
			LiteralArray returns;
			initLiteralArray(&returns);

			bool ret = callFn(&interpreter, "fail", &arguments, &returns);

			//check the results
			if (arguments.count != 0) {
				error("Arguments has the wrong number of members\n");
			}

			if (returns.count != 1 || !IS_NULL(returns.literals[0])) {
				error("Returns has the wrong number of members\n");
			}

			if (!ret) {
				error("Assertion gives the wrong return value\n");
			}

			freeLiteralArray(&arguments);
			freeLiteralArray(&returns);
		}

		//clean up
		freeInterpreter(&interpreter);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

