#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"
#include "toy_interpreter.h"

#include "toy_console_colors.h"

#include "toy_memory.h"

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
	printf("%s\n", msg);
	exit(-1);
}

int main() {
	{
		size_t size = 0;
		const char* source = (const char*)Toy_readFile("scripts/call-from-host.toy", &size);
		const unsigned char* tb = Toy_compileString(source, &size);
		free((void*)source);

		if (!tb) {
			return -1;
		}

		Toy_Interpreter interpreter;
		Toy_initInterpreter(&interpreter);
		Toy_runInterpreter(&interpreter, tb, size);

		//test answer
		{
			interpreter.printOutput("Testing answer");

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callFn(&interpreter, "answer", &arguments, &returns);

			//check the results
			if (arguments.count != 0) {
				error("Arguments has the wrong number of members");
			}

			if (returns.count != 1) {
				error("Returns has the wrong number of members");
			}

			if (!TOY_IS_INTEGER(returns.literals[0]) || TOY_AS_INTEGER(returns.literals[0]) != 42) {
				error("Returned value is incorrect");
			}

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);
		}

		//test identity
		{
			interpreter.printOutput("Testing identity");

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			//push an argument
			float pi = 3.14;
			Toy_Literal arg = TOY_TO_FLOAT_LITERAL(pi);
			Toy_pushLiteralArray(&arguments, arg);

			Toy_callFn(&interpreter, "identity", &arguments, &returns);

			//check the results
			if (arguments.count != 0) {
				error("Arguments has the wrong number of members");
			}

			if (returns.count != 1) {
				error("Returns has the wrong number of members");
			}

			float epsilon = 0.1; //because floats are evil

			if (!TOY_IS_FLOAT(returns.literals[0]) || fabs(TOY_AS_FLOAT(returns.literals[0]) - pi) > epsilon) {
				error("Returned value is incorrect");
			}

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);
		}

		//test makeCounter (closures)
		{
			interpreter.printOutput("Testing makeCounter (closures)");

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callFn(&interpreter, "makeCounter", &arguments, &returns);

			//check the results
			if (arguments.count != 0) {
				error("Arguments has the wrong number of members");
			}

			if (returns.count != 1) {
				error("Returns has the wrong number of members");
			}

			//grab the resulting literal
			Toy_Literal counter = Toy_popLiteralArray(&returns);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);

			//call counter repeatedly
			{
				Toy_LiteralArray arguments;
				Toy_initLiteralArray(&arguments);
				Toy_LiteralArray returns;
				Toy_initLiteralArray(&returns);

				Toy_callLiteralFn(&interpreter, counter, &arguments, &returns);

				//check the results
				if (arguments.count != 0) {
					error("Arguments (1) has the wrong number of members");
				}

				if (returns.count != 1) {
					error("Returns (1) has the wrong number of members");
				}

				if (!TOY_IS_INTEGER(returns.literals[0]) || TOY_AS_INTEGER(returns.literals[0]) != 1) {
					error("Returned value (1) is incorrect");
				}

				Toy_freeLiteralArray(&arguments);
				Toy_freeLiteralArray(&returns);
			}

			{
				Toy_LiteralArray arguments;
				Toy_initLiteralArray(&arguments);
				Toy_LiteralArray returns;
				Toy_initLiteralArray(&returns);

				Toy_callLiteralFn(&interpreter, counter, &arguments, &returns);

				//check the results
				if (arguments.count != 0) {
					error("Arguments (2) has the wrong number of members");
				}

				if (returns.count != 1) {
					error("Returns (2) has the wrong number of members");
				}

				if (!TOY_IS_INTEGER(returns.literals[0]) || TOY_AS_INTEGER(returns.literals[0]) != 2) {
					error("Returned value (2) is incorrect");
				}

				Toy_freeLiteralArray(&arguments);
				Toy_freeLiteralArray(&returns);
			}

			{
				Toy_LiteralArray arguments;
				Toy_initLiteralArray(&arguments);
				Toy_LiteralArray returns;
				Toy_initLiteralArray(&returns);

				Toy_callLiteralFn(&interpreter, counter, &arguments, &returns);

				//check the results
				if (arguments.count != 0) {
					error("Arguments (3) has the wrong number of members");
				}

				if (returns.count != 1) {
					error("Returns (3) has the wrong number of members");
				}

				if (!TOY_IS_INTEGER(returns.literals[0]) || TOY_AS_INTEGER(returns.literals[0]) != 3) {
					error("Returned value (3) is incorrect");
				}

				Toy_freeLiteralArray(&arguments);
				Toy_freeLiteralArray(&returns);
			}

			Toy_freeLiteral(counter);
		}

		//test assertion failure
		{
			interpreter.printOutput("Testing assertion failure");

			Toy_setInterpreterAssert(&interpreter, noPrintFn);

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			bool ret = Toy_callFn(&interpreter, "fail", &arguments, &returns);

			//check the results
			if (arguments.count != 0) {
				error("Arguments has the wrong number of members");
			}

			if (returns.count != 1 || !TOY_IS_NULL(returns.literals[0])) {
				error("Returns has the wrong number of members");
			}

			if (!ret) {
				error("Assertion gives the wrong return value");
			}

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);
		}

		//clean up
		Toy_freeInterpreter(&interpreter);
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}

