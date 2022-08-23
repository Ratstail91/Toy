#pragma once

typedef enum Opcode {
	OP_EOF,

	//basic statements
	OP_ASSERT,
	OP_PRINT,

	//data
	OP_LITERAL,
	OP_LITERAL_LONG, //for more than 256 literals in a chunk
	OP_LITERAL_RAW, //forcibly get the raw value of the literal

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

	OP_VAR_DECL,		//declare a variable to be used (as a literal)
	OP_VAR_DECL_LONG,	//declare a variable to be used (as a long literal)

	OP_VAR_ASSIGN,		//assign to a literal
	OP_VAR_ADDITION_ASSIGN,
	OP_VAR_SUBTRACTION_ASSIGN,
	OP_VAR_MULTIPLICATION_ASSIGN,
	OP_VAR_DIVISION_ASSIGN,
	OP_VAR_MODULO_ASSIGN,

	OP_TYPE_CAST,		//temporarily change a type of an atomic value

	//comparion of values
	OP_COMPARE_EQUAL,
	OP_COMPARE_NOT_EQUAL,
	OP_COMPARE_LESS,
	OP_COMPARE_LESS_EQUAL,
	OP_COMPARE_GREATER,
	OP_COMPARE_GREATER_EQUAL,
	OP_INVERT, //for booleans

	//jumps, and conditional jumps (absolute)
	OP_JUMP,
	OP_IF_FALSE_JUMP,

	//meta
	OP_SECTION_END,
	//TODO: add more
} Opcode;

