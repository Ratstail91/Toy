#include "repl_tools.h"
#include "lib_standard.h"
#include "lib_timer.h"
#include "lib_runner.h"

#include "console_colors.h"

#include "lexer.h"
#include "parser.h"
#include "compiler.h"
#include "interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void repl() {
	//repl does it's own thing for now
	bool error = false;

	const int size = 2048;
	char input[size];
	memset(input, 0, size);

	Interpreter interpreter; //persist the interpreter for the scopes
	initInterpreter(&interpreter);

	//inject the libs
	injectNativeHook(&interpreter, "standard", hookStandard);
	injectNativeHook(&interpreter, "timer", hookTimer);
	injectNativeHook(&interpreter, "runner", hookRunner);

	for(;;) {
		printf("> ");

		//handle EOF for exits
		if (!fgets(input, size, stdin)) {
			break;
		}

		//escape the repl (length of 5 to accomodate the newline)
		if (strlen(input) == 5 && (!strncmp(input, "exit", 4) || !strncmp(input, "quit", 4))) {
			break;
		}

		//setup this iteration
		Lexer lexer;
		Parser parser;
		Compiler compiler;

		initLexer(&lexer, input);
		initParser(&parser, &lexer);
		initCompiler(&compiler);

		//run this iteration
		ASTNode* node = scanParser(&parser);
		while(node != NULL) {
			//pack up and restart
			if (node->type == AST_NODE_ERROR) {
				printf(ERROR "error node detected\n" RESET);
				error = true;
				freeASTNode(node);
				break;
			}

			writeCompiler(&compiler, node);
			freeASTNode(node);
			node = scanParser(&parser);
		}

		if (!error) {
			//get the bytecode dump
			int size = 0;
			unsigned char* tb = collateCompiler(&compiler, &size);

			//run the bytecode
			runInterpreter(&interpreter, tb, size);
		}

		//clean up this iteration
		freeCompiler(&compiler);
		freeParser(&parser);
		error = false;
	}

	freeInterpreter(&interpreter);
}

//entry point
int main(int argc, const char* argv[]) {
	initCommand(argc, argv);

	//lib setup (hacky - only really for this program)
	initDriveDictionary();

	Literal driveLiteral = TO_STRING_LITERAL(createRefString("scripts"));
	Literal pathLiteral = TO_STRING_LITERAL(createRefString("scripts"));

	setLiteralDictionary(getDriveDictionary(), driveLiteral, pathLiteral);

	freeLiteral(driveLiteral);
	freeLiteral(pathLiteral);

	//command specific actions
	if (command.error) {
		usageCommand(argc, argv);
		return 0;
	}

	if (command.help) {
		helpCommand(argc, argv);
		return 0;
	}

	if (command.version) {
		copyrightCommand(argc, argv);
		return 0;
	}

	//version
	if (command.verbose) {
		printf(NOTICE "Toy Programming Language Version %d.%d.%d\n" RESET, TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH);
	}

	//run source file
	if (command.sourcefile) {
		runSourceFile(command.sourcefile);

		//lib cleanup
		freeDriveDictionary();

		return 0;
	}

	//run from stdin
	if (command.source) {
		runSource(command.source);

		//lib cleanup
		freeDriveDictionary();

		return 0;
	}

	//compile source file
	if (command.compilefile && command.outfile) {
		size_t size = 0;
		char* source = readFile(command.compilefile, &size);
		unsigned char* tb = compileString(source, &size);
		if (!tb) {
			return 1;
		}
		writeFile(command.outfile, tb, size);
		return 0;
	}

	//run binary
	if (command.binaryfile) {
		runBinaryFile(command.binaryfile);

		//lib cleanup
		freeDriveDictionary();

		return 0;
	}

	repl();

	//lib cleanup
	freeDriveDictionary();

	return 0;
}
