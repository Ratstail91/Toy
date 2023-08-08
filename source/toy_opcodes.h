#pragma once

typedef enum Toy_Opcode {
	TOY_OP_EOF,

	//do nothing
	TOY_OP_PASS,

	//basic statements
	TOY_OP_ASSERT,
	TOY_OP_PRINT,

	//data
	TOY_OP_LITERAL,
	TOY_OP_LITERAL_LONG, //for more than 256 literals in a chunk
	TOY_OP_LITERAL_RAW, //forcibly get the raw value of the literal

	//arithmetic operators
	TOY_OP_NEGATE,
	TOY_OP_ADDITION,
	TOY_OP_SUBTRACTION,
	TOY_OP_MULTIPLICATION,
	TOY_OP_DIVISION,
	TOY_OP_MODULO,
	TOY_OP_GROUPING_BEGIN,
	TOY_OP_GROUPING_END,

	//variable stuff
	TOY_OP_SCOPE_BEGIN,
	TOY_OP_SCOPE_END,

	TOY_OP_TYPE_DECL_removed,
	TOY_OP_TYPE_DECL_LONG_removed,

	TOY_OP_VAR_DECL,		//declare a variable to be used (as a literal)
	TOY_OP_VAR_DECL_LONG,	//declare a variable to be used (as a long literal)

	TOY_OP_FN_DECL,			//declare a function to be used (as a literal)
	TOY_OP_FN_DECL_LONG,	//declare a function to be used (as a long literal)

	TOY_OP_VAR_ASSIGN,		//assign to a literal
	TOY_OP_VAR_ADDITION_ASSIGN,
	TOY_OP_VAR_SUBTRACTION_ASSIGN,
	TOY_OP_VAR_MULTIPLICATION_ASSIGN,
	TOY_OP_VAR_DIVISION_ASSIGN,
	TOY_OP_VAR_MODULO_ASSIGN,

	TOY_OP_TYPE_CAST,		//temporarily change a type of an atomic value
	TOY_OP_TYPE_OF,			//get the type of a variable

	TOY_OP_IMPORT,
	TOY_OP_EXPORT_removed,

	//for indexing
	TOY_OP_INDEX,
	TOY_OP_INDEX_ASSIGN,
	TOY_OP_INDEX_ASSIGN_INTERMEDIATE,
	TOY_OP_DOT,

	//comparison of values
	TOY_OP_COMPARE_EQUAL,
	TOY_OP_COMPARE_NOT_EQUAL,
	TOY_OP_COMPARE_LESS,
	TOY_OP_COMPARE_LESS_EQUAL,
	TOY_OP_COMPARE_GREATER,
	TOY_OP_COMPARE_GREATER_EQUAL,
	TOY_OP_INVERT, //for booleans

	//logical operators
	TOY_OP_AND,
	TOY_OP_OR,

	//jumps, and conditional jumps (absolute)
	TOY_OP_JUMP,
	TOY_OP_IF_FALSE_JUMP,
	TOY_OP_FN_CALL,
	TOY_OP_FN_RETURN,

	//pop the stack at the end of a complex statement
	TOY_OP_POP_STACK,

	//ternary shorthand
	TOY_OP_TERNARY,

	//meta
	TOY_OP_FN_END, //different from SECTION_END
	TOY_OP_SECTION_END = 255,
	//TODO: add more

	//prefix & postfix signals (used internally)
	TOY_OP_PREFIX,
	TOY_OP_POSTFIX,
} Toy_Opcode;

