#pragma once

#include "opcodes.h"

#include "literal_array.h"
#include "literal_dictionary.h"

typedef void (*PrintFn)(const char*);

//the interpreter acts depending on the bytecode instructions
typedef struct Interpreter {
	LiteralArray literalCache; //generally doesn't change after initialization
	unsigned char* bytecode;
	int length;
	int count;
	LiteralArray stack;
	PrintFn printOutput;
	PrintFn assertOutput;
} Interpreter;

void initInterpreter(Interpreter* interpreter, unsigned char* bytecode, int length);
void freeInterpreter(Interpreter* interpreter);

//utilities for the host program
void setInterpreterPrint(Interpreter* interpreter, PrintFn printOutput);
void setInterpreterAssert(Interpreter* interpreter, PrintFn assertOutput);

void runInterpreter(Interpreter* interpreter);
