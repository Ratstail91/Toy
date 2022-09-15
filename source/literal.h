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
	LITERAL_FUNCTION,
	LITERAL_IDENTIFIER,
	LITERAL_TYPE,
	LITERAL_ANY,

	//these are meta-level types - not for general use
	LITERAL_TYPE_INTERMEDIATE, //used to process types in the compiler only
	LITERAL_DICTIONARY_INTERMEDIATE, //used to process dictionaries in the compiler only
	LITERAL_FUNCTION_INTERMEDIATE, //used to process functions in the compiler only
	LITERAL_FUNCTION_ARG_REST, //used to process function rest parameters only
	LITERAL_FUNCTION_NATIVE, //for handling native functions only
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

		struct {
			void* bytecode;
			void* scope;
			int length;
		} function;

		struct { //for variable names
            char* ptr;
            int length;
			int hash;
        } identifier;

		struct {
			LiteralType typeOf; //no longer a mask
			bool constant;
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
#define IS_FUNCTION_NATIVE(value)			((value).type == LITERAL_FUNCTION_NATIVE)
#define IS_IDENTIFIER(value)				((value).type == LITERAL_IDENTIFIER)
#define IS_TYPE(value)						((value).type == LITERAL_TYPE)

#define AS_BOOLEAN(value)					((value).as.boolean)
#define AS_INTEGER(value)					((value).as.integer)
#define AS_FLOAT(value)						((value).as.number)
#define AS_STRING(value)					((value).as.string.ptr)
#define AS_ARRAY(value)						((LiteralArray*)((value).as.array))
#define AS_DICTIONARY(value)				((LiteralDictionary*)((value).as.dictionary))
#define AS_FUNCTION(value)					((value).as.function)
#define AS_IDENTIFIER(value)				((value).as.identifier.ptr)
#define AS_TYPE(value)						((value).as.type)

#define TO_NULL_LITERAL						((Literal){LITERAL_NULL,		{ .integer = 0 }})
#define TO_BOOLEAN_LITERAL(value)			((Literal){LITERAL_BOOLEAN,		{ .boolean = value }})
#define TO_INTEGER_LITERAL(value)			((Literal){LITERAL_INTEGER,		{ .integer = value }})
#define TO_FLOAT_LITERAL(value)				((Literal){LITERAL_FLOAT,		{ .number = value }})
#define TO_STRING_LITERAL(value, l)			_toStringLiteral(value, l)
#define TO_ARRAY_LITERAL(value)				((Literal){LITERAL_ARRAY,		{ .array = value }})
#define TO_DICTIONARY_LITERAL(value)		((Literal){LITERAL_DICTIONARY,	{ .dictionary = value }})
#define TO_FUNCTION_LITERAL(value, l)		((Literal){LITERAL_FUNCTION,	{ .function.bytecode = value, .function.scope = NULL, .function.length = l }})
#define TO_IDENTIFIER_LITERAL(value, l)		_toIdentifierLiteral(value, l)
#define TO_TYPE_LITERAL(value, c)			((Literal){ LITERAL_TYPE,		{ .type.typeOf = value, .type.constant = c, .type.subtypes = NULL, .type.capacity = 0, .type.count = 0 }})

TOY_API void freeLiteral(Literal literal);

#define IS_TRUTHY(x) _isTruthy(x)

#define MAX_STRING_LENGTH					4096
#define HASH_I(lit)							((lit).as.identifier.hash)
#define TYPE_PUSH_SUBTYPE(lit, subtype)		_typePushSubtype(lit, subtype)

//BUGFIX: macros are not functions
TOY_API bool _isTruthy(Literal x);
TOY_API Literal _toStringLiteral(char* str, int length);
TOY_API Literal _toIdentifierLiteral(char* str, int length);
TOY_API Literal* _typePushSubtype(Literal* lit, Literal subtype);

//utils
TOY_API Literal copyLiteral(Literal original);
TOY_API char* copyString(char* original, int length);
TOY_API bool literalsAreEqual(Literal lhs, Literal rhs);
TOY_API int hashLiteral(Literal lit);

TOY_API void printLiteral(Literal literal);
TOY_API void printLiteralCustom(Literal literal, void (printFn)(const char*));
