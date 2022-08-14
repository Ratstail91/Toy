#pragma once

#include "opcodes.h"

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
	LiteralArray literalCache; //read-only - built from the bytecode, refreshed each time new bytecode is provided

	//operation
	Scope* scope;
	LiteralArray stack;

	//output
	// LiteralDictionary exports; //TODO: read-write - interface with Toy from C
	PrintFn printOutput;
	PrintFn assertOutput;
} Interpreter;

void initInterpreter(Interpreter* interpreter);
void freeInterpreter(Interpreter* interpreter);

//utilities for the host program
void setInterpreterPrint(Interpreter* interpreter, PrintFn printOutput);
void setInterpreterAssert(Interpreter* interpreter, PrintFn assertOutput);

void runInterpreter(Interpreter* interpreter, unsigned char* bytecode, int length);
