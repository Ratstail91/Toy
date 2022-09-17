#pragma once

#include "common.h"
#include "literal.h"
#include "literal_array.h"
#include "literal_dictionary.h"
#include "scope.h"

typedef void (*PrintFn)(const char*);

//the interpreter acts depending on the bytecode instructions
typedef struct Interpreter {
	//input
	unsigned char* bytecode;
	int length;
	int count;
	int codeStart; //BUGFIX: for jumps, must be initialized to -1
	LiteralArray literalCache; //read-only - built from the bytecode, refreshed each time new bytecode is provided

	//operation
	Scope* scope;
	LiteralArray stack;

	LiteralDictionary* exports; //read-write - interface with Toy from C - this is a pointer, since it works at a script-level
	LiteralDictionary* exportTypes;
	LiteralDictionary* hooks;

	//debug outputs
	PrintFn printOutput;
	PrintFn assertOutput;
	PrintFn errorOutput;

	int depth; //don't overflow
	bool panic;
} Interpreter;

//native API
typedef int (*NativeFn)(Interpreter* interpreter, LiteralArray* arguments);
TOY_API bool injectNativeFn(Interpreter* interpreter, char* name, NativeFn func);

typedef int (*HookFn)(Interpreter* interpreter, Literal identifier, Literal alias);
TOY_API bool injectNativeHook(Interpreter* interpreter, char* name, HookFn hook);

//utilities for the host program
TOY_API bool parseIdentifierToValue(Interpreter* interpreter, Literal* literalPtr);
TOY_API void setInterpreterPrint(Interpreter* interpreter, PrintFn printOutput);
TOY_API void setInterpreterAssert(Interpreter* interpreter, PrintFn assertOutput);
TOY_API void setInterpreterError(Interpreter* interpreter, PrintFn errorOutput);

//main access
TOY_API void initInterpreter(Interpreter* interpreter); //start of program
TOY_API void runInterpreter(Interpreter* interpreter, unsigned char* bytecode, int length); //run the code
TOY_API void resetInterpreter(Interpreter* interpreter); //use this to reset the interpreter's environment between runs
TOY_API void freeInterpreter(Interpreter* interpreter); //end of program
