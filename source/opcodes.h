#pragma once

typedef enum Opcode {
	OP_EOF,

	//basic operations
	OP_PRINT,

	//data
	OP_LITERAL,
	OP_LITERAL_LONG, //for more than 256 literals in a chunk

	//operators
	OP_NEGATE,

	//meta
	OP_SECTION_END,
	//TODO: add more
} Opcode;

