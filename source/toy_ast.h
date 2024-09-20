#pragma once

#include "toy_common.h"
#include "toy_memory.h"

#include "toy_value.h"

//each major type
typedef enum Toy_AstType {
	TOY_AST_BLOCK,

	TOY_AST_VALUE,
	TOY_AST_UNARY,
	TOY_AST_BINARY,
	TOY_AST_GROUP,

	TOY_AST_PASS,
	TOY_AST_ERROR,
	TOY_AST_END,
} Toy_AstType;

//flags are handled differently by different types
typedef enum Toy_AstFlag {
	TOY_AST_FLAG_NONE,

	//binary flags
	TOY_AST_FLAG_ADD,
	TOY_AST_FLAG_SUBTRACT,
	TOY_AST_FLAG_MULTIPLY,
	TOY_AST_FLAG_DIVIDE,
	TOY_AST_FLAG_MODULO,
	TOY_AST_FLAG_ASSIGN, //TODO: implement the declare statement
	TOY_AST_FLAG_ADD_ASSIGN,
	TOY_AST_FLAG_SUBTRACT_ASSIGN,
	TOY_AST_FLAG_MULTIPLY_ASSIGN,
	TOY_AST_FLAG_DIVIDE_ASSIGN,
	TOY_AST_FLAG_MODULO_ASSIGN,
	TOY_AST_FLAG_COMPARE_EQUAL,
	TOY_AST_FLAG_COMPARE_NOT,
	TOY_AST_FLAG_COMPARE_LESS,
	TOY_AST_FLAG_COMPARE_LESS_EQUAL,
	TOY_AST_FLAG_COMPARE_GREATER,
	TOY_AST_FLAG_COMPARE_GREATER_EQUAL,
	TOY_AST_FLAG_AND,
	TOY_AST_FLAG_OR,

	//unary flags
	TOY_AST_FLAG_NEGATE,
	TOY_AST_FLAG_INCREMENT,
	TOY_AST_FLAG_DECREMENT,

	// TOY_AST_FLAG_TERNARY,
} Toy_AstFlag;

//the root AST type
typedef union Toy_Ast Toy_Ast;

void Toy_private_initAstBlock(Toy_Bucket** bucket, Toy_Ast** handle);
void Toy_private_appendAstBlock(Toy_Bucket** bucket, Toy_Ast** handle, Toy_Ast* child);

void Toy_private_emitAstValue(Toy_Bucket** bucket, Toy_Ast** handle, Toy_Value value);
void Toy_private_emitAstUnary(Toy_Bucket** bucket, Toy_Ast** handle, Toy_AstFlag flag);
void Toy_private_emitAstBinary(Toy_Bucket** bucket, Toy_Ast** handle,Toy_AstFlag flag, Toy_Ast* right);
void Toy_private_emitAstGroup(Toy_Bucket** bucket, Toy_Ast** handle);

void Toy_private_emitAstPass(Toy_Bucket** bucket, Toy_Ast** handle);
void Toy_private_emitAstError(Toy_Bucket** bucket, Toy_Ast** handle);
void Toy_private_emitAstEnd(Toy_Bucket** bucket, Toy_Ast** handle);

typedef struct Toy_AstBlock {
	Toy_AstType type;
	Toy_Ast* child; //begin encoding the line
	Toy_Ast* next; //'next' is either an AstBlock or null
	Toy_Ast* tail; //'tail' - either points to the tail of the current list, or null; only used by the head of a list as an optimisation
} Toy_AstBlock;

typedef struct Toy_AstValue {
	Toy_AstType type;
	Toy_Value value;
} Toy_AstValue;

typedef struct Toy_AstUnary {
	Toy_AstType type;
	Toy_AstFlag flag;
	Toy_Ast* child;
} Toy_AstUnary;

typedef struct Toy_AstBinary {
	Toy_AstType type;
	Toy_AstFlag flag;
	Toy_Ast* left;
	Toy_Ast* right;
} Toy_AstBinary;

typedef struct Toy_AstGroup {
	Toy_AstType type;
	Toy_Ast* child;
} Toy_AstGroup;

typedef struct Toy_AstPass {
	Toy_AstType type;
} Toy_AstPass;

typedef struct Toy_AstError {
	Toy_AstType type;
	//TODO: more data regarding the error
} Toy_AstError;

typedef struct Toy_AstEnd {
	Toy_AstType type;
} Toy_AstEnd;

union Toy_Ast {
	Toy_AstType type; //4
	Toy_AstBlock block; //12
	Toy_AstValue value; //12
	Toy_AstUnary unary; //12
	Toy_AstBinary binary; //16
	Toy_AstGroup group; //8
	Toy_AstPass pass; //4
	Toy_AstError error; //4
	Toy_AstEnd end; //4
}; //16
