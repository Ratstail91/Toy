#pragma once

#include "common.h"

#include <string.h>

typedef enum {
	LITERAL_NULL,
	LITERAL_BOOLEAN,
	LITERAL_INTEGER,
	LITERAL_FLOAT,
	LITERAL_STRING,
	LITERAL_ARRAY,
	LITERAL_DICTIONARY,
	// LITERAL_FUNCTION,
	LITERAL_IDENTIFIER,
	LITERAL_TYPE,
} LiteralType;

typedef struct {
	LiteralType type;
	union {
		bool boolean;
		int integer;
		float number;
		struct {
			char* ptr;
			int length;
		} string;

		void* array;
		void* dictionary;

		// void* function;

		struct { //for variable names
            char* ptr;
            int length;
			int hash;
        } identifier;

		struct {
			unsigned char mask;
			void* subtypes; //for nested types caused by compounds
			int capacity;
			int count;
		} type;
	} as;
} Literal;

#define IS_NULL(value)						((value).type == LITERAL_NULL)
#define IS_BOOLEAN(value)					((value).type == LITERAL_BOOLEAN)
#define IS_INTEGER(value)					((value).type == LITERAL_INTEGER)
#define IS_FLOAT(value)						((value).type == LITERAL_FLOAT)
#define IS_STRING(value)					((value).type == LITERAL_STRING)
#define IS_ARRAY(value)						((value).type == LITERAL_ARRAY)
#define IS_DICTIONARY(value)				((value).type == LITERAL_DICTIONARY)
#define IS_FUNCTION(value)					((value).type == LITERAL_FUNCTION)
#define IS_IDENTIFIER(value)				((value).type == LITERAL_IDENTIFIER)
#define IS_TYPE(value)						((value).type == LITERAL_TYPE)

#define AS_BOOLEAN(value)					((value).as.boolean)
#define AS_INTEGER(value)					((value).as.integer)
#define AS_FLOAT(value)						((value).as.number)
#define AS_STRING(value)					((value).as.string.ptr)
#define AS_ARRAY(value)						((LiteralArray*)((value).as.array))
#define AS_DICTIONARY(value)				((LiteralDictionary*)((value).as.dictionary))
// #define AS_FUNCTION(value)
#define AS_IDENTIFIER(value)				((value).as.identifier.ptr)
#define AS_TYPE(value)						((value).as.type)

#define TO_NULL_LITERAL						((Literal){LITERAL_NULL,		{ .integer = 0 }})
#define TO_BOOLEAN_LITERAL(value)			((Literal){LITERAL_BOOLEAN,		{ .boolean = value }})
#define TO_INTEGER_LITERAL(value)			((Literal){LITERAL_INTEGER,		{ .integer = value }})
#define TO_FLOAT_LITERAL(value)				((Literal){LITERAL_FLOAT,		{ .number = value }})
#define TO_STRING_LITERAL(value)			_toStringLiteral(value)
#define TO_ARRAY_LITERAL(value)				((Literal){LITERAL_ARRAY,		{ .array = value }})
#define TO_DICTIONARY_LITERAL(value)		((Literal){LITERAL_DICTIONARY,	{ .dictionary = value }})
// #define TO_FUNCTION_LITERAL
#define TO_IDENTIFIER_LITERAL(value)		_toIdentifierLiteral(value)
#define TO_TYPE_LITERAL(value)				((Literal){ LITERAL_TYPE,		{ .type.mask = value, .type.subtypes = NULL, .type.capacity = 0, .type.count = 0 }})

#define MASK(x) 			(1 << (x))
#define TYPE_CONST			0
#define TYPE_BOOLEAN		1
#define TYPE_INTEGER		2
#define TYPE_FLOAT			3
#define TYPE_STRING			4
#define TYPE_ARRAY			5
#define TYPE_DICTIONARY		6
#define TYPE_FUNCTION		7
#define MASK_CONST			(MASK(TYPE_CONST))
#define MASK_BOOLEAN		(MASK(TYPE_BOOLEAN))
#define MASK_INTEGER		(MASK(TYPE_INTEGER))
#define MASK_FLOAT			(MASK(TYPE_FLOAT))
#define MASK_STRING			(MASK(TYPE_STRING))
#define MASK_ARRAY			(MASK(TYPE_ARRAY))
#define MASK_DICTIONARY		(MASK(TYPE_DICTIONARY))
#define MASK_FUNCTION		(MASK(TYPE_FUNCTION))
#define MASK_ANY			0

//utils
void printLiteral(Literal literal);
void printLiteralCustom(Literal literal, void (printFn)(const char*));
void freeLiteral(Literal literal);

#define IS_TRUTHY(x) _isTruthy(x)

#define STRLEN(lit)							((lit).as.string.length)
#define STRLEN_I(lit)						((lit).as.identifier.length)
#define HASH_I(lit)							((lit).as.identifier.hash)
#define TYPE_PUSH_SUBTYPE(lit, submask)		_typePushSubtype(lit, submask)

//BUGFIX: macros are not functions
bool _isTruthy(Literal x);
Literal _toStringLiteral(char* str);
Literal _toIdentifierLiteral(char* str);
Literal* _typePushSubtype(Literal* lit, unsigned char submask);

//utils
char* copyString(char* original, int length);
bool literalsAreEqual(Literal lhs, Literal rhs);
int hashLiteral(Literal lit);