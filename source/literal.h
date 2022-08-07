#pragma once

#include "common.h"

#include <string.h>

typedef enum {
	LITERAL_NULL,
	LITERAL_BOOLEAN,
	LITERAL_INTEGER,
	LITERAL_FLOAT,
	LITERAL_STRING,
	// LITERAL_ARRAY,
	// LITERAL_DICTIONARY,
	// LITERAL_FUNCTION,
} LiteralType;

typedef struct {
	LiteralType type;
	union {
		bool boolean;
		int integer;
		float number;
		struct {
			char* ptr;
			int length; //could possibly cut it down further by removing this
		} string;

		// //experimental
		// void* array;
		// void* dictionary;
		// void* function;
	} as;
} Literal;

#define IS_NULL(value)				((value).type == LITERAL_NULL)
#define IS_BOOLEAN(value)			((value).type == LITERAL_BOOLEAN)
#define IS_INTEGER(value)			((value).type == LITERAL_INTEGER)
#define IS_FLOAT(value)				((value).type == LITERAL_FLOAT)
#define IS_STRING(value)			((value).type == LITERAL_STRING)
#define IS_ARRAY(value)				((value).type == LITERAL_ARRAY)
#define IS_DICTIONARY(value)		((value).type == LITERAL_DICTIONARY)
#define IS_FUNCTION(value)			((value).type == LITERAL_FUNCTION)

#define AS_BOOLEAN(value)			((value).as.boolean)
#define AS_INTEGER(value)			((value).as.integer)
#define AS_FLOAT(value)				((value).as.number)
#define AS_STRING(value)			((value).as.string.ptr)
// #define AS_ARRAY_PTR(value)
// #define AS_DICTIONARY_PTR(value)
// #define AS_FUNCTION_PTR(value)		((Function*)((value).as.function))

#define TO_NULL_LITERAL				((Literal){LITERAL_NULL,		{ .integer = 0 }})
#define TO_BOOLEAN_LITERAL(value)	((Literal){LITERAL_BOOLEAN,		{ .boolean = value }})
#define TO_INTEGER_LITERAL(value)	((Literal){LITERAL_INTEGER,		{ .integer = value }})
#define TO_FLOAT_LITERAL(value)		((Literal){LITERAL_FLOAT,		{ .number = value }})
#define TO_STRING_LITERAL(value)	_toStringLiteral(value)
// #define TO_ARRAY_PTR
// #define TO_DICTIONARY_PTR
// #define TO_FUNCTION_PTR(value)		((Literal){LITERAL_FUNCTION,	{ .function = (Function*)value }})

void printLiteral(Literal literal);
void printLiteralCustom(Literal literal, void (printFn)(const char*));
void freeLiteral(Literal literal);

#define IS_TRUTHY(x) _isTruthy(x)

#define STRLEN(lit) ((lit).as.string.length)

//BUGFIX: macros are not functions
bool _isTruthy(Literal x);
Literal _toStringLiteral(char* cstr);

//utils
char* copyString(char* original, int length);