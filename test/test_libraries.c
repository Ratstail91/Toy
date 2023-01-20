#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "interpreter.h"

#include "console_colors.h"

#include "memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../repl/repl_tools.h"

#include "../repl/lib_standard.h"
#include "../repl/lib_timer.h"
#include "../repl/lib_runner.h"

//supress the print output
static void noPrintFn(const char* output) {
	//NO OP
}

static int failedAsserts = 0;
static void assertWrapper(const char* output) {
	failedAsserts++;
	fprintf(stderr, ERROR "Assertion failure: ");
	fprintf(stderr, "%s", output);
	fprintf(stderr, "\n" RESET); //default new line
}

static void errorWrapper(const char* output) {
	failedAsserts++;
	fprintf(stderr, ERROR "%s" RESET, output);
}

void runBinaryWithLibrary(unsigned char* tb, size_t size, char* library, HookFn hook) {
	Interpreter interpreter;
	initInterpreter(&interpreter);

	//NOTE: supress print output for testing
	setInterpreterPrint(&interpreter, noPrintFn);
	setInterpreterAssert(&interpreter, assertWrapper);
	setInterpreterError(&interpreter, errorWrapper);

	//inject the standard libraries into this interpreter
	injectNativeHook(&interpreter, library, hook);

	runInterpreter(&interpreter, tb, size);
	freeInterpreter(&interpreter);
}

typedef struct Payload {
	char* fname;
	char* libname;
	HookFn hook;
} Payload;

int main() {
	//setup the runner filesystem (hacky)
	initDriveDictionary();

	Literal driveLiteral = TO_STRING_LITERAL(createRefString("scripts"));
	Literal pathLiteral = TO_STRING_LITERAL(createRefString("scripts"));

	setLiteralDictionary(getDriveDictionary(), driveLiteral, pathLiteral);

	freeLiteral(driveLiteral);
	freeLiteral(pathLiteral);

	{
		//run each file in test/scripts
		Payload payloads[] = {
			{"interactions.toy", "standard", hookStandard}, //interactions needs standard
			{"standard.toy", "standard", hookStandard},
			{"timer.toy", "timer", hookTimer},
			{"runner.toy", "runner", hookRunner},
			{NULL, NULL, NULL}
		};

		for (int i = 0; payloads[i].fname; i++) {
			printf("Running %s\n", payloads[i].fname);

			char fname[128];
			snprintf(fname, 128, "scripts/lib/%s", payloads[i].fname);

			//compile the source
			size_t size = 0;
			char* source = readFile(fname, &size);
			if (!source) {
				printf(ERROR "Failed to load file: %s\n" RESET, fname);
				failedAsserts++;
				continue;
			}

			unsigned char* tb = compileString(source, &size);
			free((void*)source);

			if (!tb) {
				printf(ERROR "Failed to compile file: %s\n" RESET, fname);
				failedAsserts++;
				continue;
			}

			runBinaryWithLibrary(tb, size, payloads[i].libname, payloads[i].hook);
		}
	}

	//lib cleanup
	freeDriveDictionary();

	if (!failedAsserts) {
		printf(NOTICE "All good\n" RESET);
	}
	else {
		printf(WARN "Problems detected in libraries\n" RESET);
	}

	return failedAsserts;
}

