#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"
#include "toy_interpreter.h"

#include "toy_console_colors.h"

#include "toy_memory.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../repl/repl_tools.h"
#include "../repl/drive_system.h"

#include "../repl/lib_about.h"
#include "../repl/lib_random.h"
#include "../repl/lib_runner.h"
#include "../repl/lib_standard.h"

//supress the print output
static void noPrintFn(const char* output) {
	//NO OP
}

static int failedAsserts = 0;
static void assertWrapper(const char* output) {
	failedAsserts++;
	fprintf(stderr, TOY_CC_ERROR "Assertion failure: ");
	fprintf(stderr, "%s", output);
	fprintf(stderr, "\n" TOY_CC_RESET); //default new line
}

static void errorWrapper(const char* output) {
	failedAsserts++;
	fprintf(stderr, TOY_CC_ERROR "%s" TOY_CC_RESET, output);
}

void runBinaryWithLibrary(const unsigned char* tb, size_t size, const char* library, Toy_HookFn hook) {
	Toy_Interpreter interpreter;
	Toy_initInterpreter(&interpreter);

	//NOTE: supress print output for testing
	Toy_setInterpreterPrint(&interpreter, noPrintFn);
	Toy_setInterpreterAssert(&interpreter, assertWrapper);
	Toy_setInterpreterError(&interpreter, errorWrapper);

	//inject the standard libraries into this interpreter
	if (hook != Toy_hookStandard) {
		Toy_injectNativeHook(&interpreter, "standard", Toy_hookStandard);
	}
	Toy_injectNativeHook(&interpreter, library, hook);

	Toy_runInterpreter(&interpreter, tb, size);
	Toy_freeInterpreter(&interpreter);
}

typedef struct Payload {
	char* fname;
	char* libname;
	Toy_HookFn hook;
} Payload;

int main() {
	//setup the runner filesystem (hacky)
	Toy_initDriveSystem();

	Toy_setDrivePath("scripts", "scripts");

	{
		//run each file in test/scripts
		Payload payloads[] = {
			{"interactions.toy", "standard", Toy_hookStandard}, //interactions needs standard
			{"about.toy", "about", Toy_hookAbout},
			{"standard.toy", "standard", Toy_hookStandard},
			{"runner.toy", "runner", Toy_hookRunner},
			{"random.toy", "random", Toy_hookRandom},
			{NULL, NULL, NULL}
		};

		for (int i = 0; payloads[i].fname; i++) {
			printf("Running %s\n", payloads[i].fname);

			char fname[128];
			snprintf(fname, 128, "scripts/lib/%s", payloads[i].fname);

			//compile the source
			size_t size = 0;
			const char* source = (const char*)Toy_readFile(fname, &size);
			if (!source) {
				printf(TOY_CC_ERROR "Failed to load file: %s\n" TOY_CC_RESET, fname);
				failedAsserts++;
				continue;
			}

			const unsigned char* tb = Toy_compileString(source, &size);
			free((void*)source);

			if (!tb) {
				printf(TOY_CC_ERROR "Failed to compile file: %s\n" TOY_CC_RESET, fname);
				failedAsserts++;
				continue;
			}

			runBinaryWithLibrary(tb, size, payloads[i].libname, payloads[i].hook);
		}
	}

	//lib cleanup
	Toy_freeDriveSystem();

	if (!failedAsserts) {
		printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	}
	else {
		printf(TOY_CC_WARN "Problems detected in libraries\n" TOY_CC_RESET);
	}

	return failedAsserts;
}

