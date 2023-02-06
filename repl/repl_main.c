#include "repl_tools.h"
#include "lib_about.h"
#include "lib_compound.h"
#include "lib_standard.h"
#include "lib_timer.h"
#include "lib_runner.h"

#include "toy_console_colors.h"

#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"
#include "toy_interpreter.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void repl() {
	//repl does it's own thing for now
	bool error = false;

	const int size = 2048;
	char input[size];
	memset(input, 0, size);

	Toy_Interpreter interpreter; //persist the interpreter for the scopes
	Toy_initInterpreter(&interpreter);

	//inject the libs
	Toy_injectNativeHook(&interpreter, "about", Toy_hookAbout);
	Toy_injectNativeHook(&interpreter, "compound", Toy_hookCompound);
	Toy_injectNativeHook(&interpreter, "standard", Toy_hookStandard);
	Toy_injectNativeHook(&interpreter, "timer", Toy_hookTimer);
	Toy_injectNativeHook(&interpreter, "runner", Toy_hookRunner);

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
		Toy_Lexer lexer;
		Toy_Parser parser;
		Toy_Compiler compiler;

		Toy_initLexer(&lexer, input);
		Toy_initParser(&parser, &lexer);
		Toy_initCompiler(&compiler);

		//run this iteration
		Toy_ASTNode* node = Toy_scanParser(&parser);
		while(node != NULL) {
			//pack up and restart
			if (node->type == TOY_AST_NODE_ERROR) {
				if (Toy_commandLine.verbose) {
					printf(TOY_CC_ERROR "Error node detected\n" TOY_CC_RESET);
				}
				error = true;
				Toy_freeASTNode(node);
				break;
			}

			Toy_writeCompiler(&compiler, node);
			Toy_freeASTNode(node);
			node = Toy_scanParser(&parser);
		}

		if (!error) {
			//get the bytecode dump
			int size = 0;
			unsigned char* tb = Toy_collateCompiler(&compiler, &size);

			//run the bytecode
			Toy_runInterpreter(&interpreter, tb, size);
		}

		//clean up this iteration
		Toy_freeCompiler(&compiler);
		Toy_freeParser(&parser);
		error = false;
	}

	Toy_freeInterpreter(&interpreter);
}

//entry point
int main(int argc, const char* argv[]) {
	Toy_initCommandLine(argc, argv);

	//lib setup (hacky - only really for this program)
	Toy_initDriveDictionary();

	Toy_Literal driveLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("scripts"));
	Toy_Literal pathLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("scripts"));

	Toy_setLiteralDictionary(Toy_getDriveDictionary(), driveLiteral, pathLiteral);

	Toy_freeLiteral(driveLiteral);
	Toy_freeLiteral(pathLiteral);

	//command line specific actions
	if (Toy_commandLine.error) {
		Toy_usageCommandLine(argc, argv);
		return 0;
	}

	if (Toy_commandLine.help) {
		Toy_helpCommandLine(argc, argv);
		return 0;
	}

	if (Toy_commandLine.version) {
		Toy_copyrightCommandLine(argc, argv);
		return 0;
	}

	//version
	if (Toy_commandLine.verbose) {
		printf(TOY_CC_NOTICE "Toy Programming Language Version %d.%d.%d, built '%s'\n" TOY_CC_RESET, TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, TOY_VERSION_BUILD);
	}

	//run source file
	if (Toy_commandLine.sourcefile) {
		Toy_runSourceFile(Toy_commandLine.sourcefile);

		//lib cleanup
		Toy_freeDriveDictionary();

		return 0;
	}

	//run from stdin
	if (Toy_commandLine.source) {
		Toy_runSource(Toy_commandLine.source);

		//lib cleanup
		Toy_freeDriveDictionary();

		return 0;
	}

	//compile source file
	if (Toy_commandLine.compilefile && Toy_commandLine.outfile) {
		size_t size = 0;
		char* source = Toy_readFile(Toy_commandLine.compilefile, &size);
		if (!source) {
			return 1;
		}
		unsigned char* tb = Toy_compileString(source, &size);
		if (!tb) {
			return 1;
		}
		Toy_writeFile(Toy_commandLine.outfile, tb, size);
		return 0;
	}

	//run binary
	if (Toy_commandLine.binaryfile) {
		Toy_runBinaryFile(Toy_commandLine.binaryfile);

		//lib cleanup
		Toy_freeDriveDictionary();

		return 0;
	}

	repl();

	//lib cleanup
	Toy_freeDriveDictionary();

	return 0;
}
