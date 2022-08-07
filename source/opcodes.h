#pragma once

typedef enum Opcode {
	OP_EOF,

	//basic statements
	OP_ASSERT,
	OP_PRINT,

	//data
	OP_LITERAL,
	OP_LITERAL_LONG, //for more than 256 literals in a chunk

	//operators
	OP_NEGATE,
	OP_ADDITION,
	OP_SUBTRACTION,
	OP_MULTIPLICATION,
	OP_DIVISION,
	OP_MODULO,
	OP_GROUPING_BEGIN,
	OP_GROUPING_END,

	//meta
	OP_SECTION_END,
	//TODO: add more
} Opcode;

