#pragma once

#include "opcodes.h"

#include "literal_array.h"

//the interpreter acts depending on the bytecode instructions
typedef struct Interpreter {
	LiteralArray literalCache; //generally doesn't change after initialization
	unsigned char* bytecode;
	int length;
	int count;

	LiteralArray stack;
} Interpreter;

void initInterpreter(Interpreter* interpreter, unsigned char* bytecode, int length);
void freeInterpreter(Interpreter* interpreter);

void runInterpreter(Interpreter* interpreter);
