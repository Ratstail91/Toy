#pragma once

typedef enum Opcode {
	OP_EOF,

	//basic statements
	OP_ASSERT,
	OP_PRINT,

	//data
	OP_LITERAL,
	OP_LITERAL_LONG, //for more than 256 literals in a chunk

	//arithmetic operators
	OP_NEGATE,
	OP_ADDITION,
	OP_SUBTRACTION,
	OP_MULTIPLICATION,
	OP_DIVISION,
	OP_MODULO,
	OP_GROUPING_BEGIN,
	OP_GROUPING_END,

	//variable stuff
	OP_SCOPE_BEGIN,
	OP_SCOPE_END,

	OP_TYPE_DECL,		//declare a type to be used (as a literal)
	OP_TYPE_DECL_LONG,	//declare a type to be used (as a long literal)

	OP_VAR_DECL,
	OP_VAR_DECL_LONG,
	// OP_VAR_ASSIGN, //stack: literal name, literal value

	//meta
	OP_SECTION_END,
	//TODO: add more
} Opcode;

