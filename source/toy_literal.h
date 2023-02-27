#pragma once

#include "toy_common.h"

#include "toy_refstring.h"

#include <stdint.h>

//forward delcare stuff
struct Toy_Literal;
struct Toy_Interpreter;
struct Toy_LiteralArray;
struct Toy_Scope;
typedef int (*Toy_NativeFn)(struct Toy_Interpreter* interpreter, struct Toy_LiteralArray* arguments);
typedef int (*Toy_HookFn)(struct Toy_Interpreter* interpreter, struct Toy_Literal identifier, struct Toy_Literal alias);
typedef void (*Toy_PrintFn)(const char*);

typedef enum {
	TOY_LITERAL_NULL,
	TOY_LITERAL_BOOLEAN,
	TOY_LITERAL_INTEGER,
	TOY_LITERAL_FLOAT,
	TOY_LITERAL_STRING,
	TOY_LITERAL_ARRAY,
	TOY_LITERAL_DICTIONARY,
	TOY_LITERAL_FUNCTION,
	TOY_LITERAL_IDENTIFIER,
	TOY_LITERAL_TYPE,
	TOY_LITERAL_OPAQUE,
	TOY_LITERAL_ANY,

	//these are meta-level types - not for general use
	TOY_LITERAL_TYPE_INTERMEDIATE, //used to process types in the compiler only
	TOY_LITERAL_ARRAY_INTERMEDIATE, //used to process arrays in the compiler only
	TOY_LITERAL_DICTIONARY_INTERMEDIATE, //used to process dictionaries in the compiler only
	TOY_LITERAL_FUNCTION_INTERMEDIATE, //used to process functions in the compiler only
	TOY_LITERAL_FUNCTION_ARG_REST, //used to process function rest parameters only
	TOY_LITERAL_FUNCTION_NATIVE, //for handling native functions only
	TOY_LITERAL_FUNCTION_HOOK, //for handling hook functions within literals only
	TOY_LITERAL_INDEX_BLANK, //for blank indexing i.e. arr[:]
} Toy_LiteralType;

typedef struct Toy_Literal {
	union {
		bool boolean; //1
		int64_t integer; //4
		float number;//4

		struct {
			Toy_RefString* ptr; //8
			//string hash?
		} string; //8

		void* array; //8
		void* dictionary; //8

		struct {
			union {
				void* bytecode;  //8
				Toy_NativeFn native; //8
				Toy_HookFn hook; //8
			} inner;  //8
			struct Toy_Scope* scope; //8
		} function; //16

		struct { //for variable names
            Toy_RefString* ptr;  //8
			int hash; //4
        } identifier; //16

		struct {
			void* subtypes; //8
			Toy_LiteralType typeOf;  //4
			unsigned char capacity; //1
			unsigned char count; //1
			bool constant; //1
		} type; //16

		struct {
			void* ptr; //8
			int tag; //4
		} opaque; //16
	} as; //16

	Toy_LiteralType type; //4
	int bytecodeLength; //4 - shenanigans with byte alignment reduces the size of Toy_Literal
} Toy_Literal;

#define TOY_IS_NULL(value)						((value).type == TOY_LITERAL_NULL)
#define TOY_IS_BOOLEAN(value)					((value).type == TOY_LITERAL_BOOLEAN)
#define TOY_IS_INTEGER(value)					((value).type == TOY_LITERAL_INTEGER)
#define TOY_IS_FLOAT(value)						((value).type == TOY_LITERAL_FLOAT)
#define TOY_IS_STRING(value)					((value).type == TOY_LITERAL_STRING)
#define TOY_IS_ARRAY(value)						((value).type == TOY_LITERAL_ARRAY)
#define TOY_IS_DICTIONARY(value)				((value).type == TOY_LITERAL_DICTIONARY)
#define TOY_IS_FUNCTION(value)					((value).type == TOY_LITERAL_FUNCTION)
#define TOY_IS_FUNCTION_NATIVE(value)			((value).type == TOY_LITERAL_FUNCTION_NATIVE)
#define TOY_IS_FUNCTION_HOOK(value)				((value).type == TOY_LITERAL_FUNCTION_HOOK)
#define TOY_IS_IDENTIFIER(value)				((value).type == TOY_LITERAL_IDENTIFIER)
#define TOY_IS_TYPE(value)						((value).type == TOY_LITERAL_TYPE)
#define TOY_IS_OPAQUE(value)					((value).type == TOY_LITERAL_OPAQUE)

#define TOY_AS_BOOLEAN(value)					((value).as.boolean)
#define TOY_AS_INTEGER(value)					((value).as.integer)
#define TOY_AS_FLOAT(value)						((value).as.number)
#define TOY_AS_STRING(value)					((value).as.string.ptr)
#define TOY_AS_ARRAY(value)						((Toy_LiteralArray*)((value).as.array))
#define TOY_AS_DICTIONARY(value)				((Toy_LiteralDictionary*)((value).as.dictionary))
#define TOY_AS_FUNCTION(value)					((value).as.function)
#define TOY_AS_FUNCTION_NATIVE(value)			((value).as.function.inner.native)
#define TOY_AS_FUNCTION_HOOK(value)				((value).as.function.inner.hook)
#define TOY_AS_IDENTIFIER(value)				((value).as.identifier.ptr)
#define TOY_AS_TYPE(value)						((value).as.type)
#define TOY_AS_OPAQUE(value)					((value).as.opaque.ptr)

#define TOY_TO_NULL_LITERAL						((Toy_Literal){{ .integer = 0 }, TOY_LITERAL_NULL, 0})
#define TOY_TO_BOOLEAN_LITERAL(value)			((Toy_Literal){{ .boolean = value }, TOY_LITERAL_BOOLEAN, 0})
#define TOY_TO_INTEGER_LITERAL(value)			((Toy_Literal){{ .integer = value }, TOY_LITERAL_INTEGER, 0})
#define TOY_TO_FLOAT_LITERAL(value)				((Toy_Literal){{ .number = value }, TOY_LITERAL_FLOAT, 0})
#define TOY_TO_STRING_LITERAL(value)			Toy_private_toStringLiteral(value)
#define TOY_TO_ARRAY_LITERAL(value)				((Toy_Literal){{ .array = value }, TOY_LITERAL_ARRAY, 0})
#define TOY_TO_DICTIONARY_LITERAL(value)		((Toy_Literal){{ .dictionary = value }, TOY_LITERAL_DICTIONARY, 0})
#define TOY_TO_FUNCTION_LITERAL(value, l)		((Toy_Literal){{ .function = { .inner = { .bytecode = value }, .scope = NULL }}, TOY_LITERAL_FUNCTION, l})
#define TOY_TO_FUNCTION_NATIVE_LITERAL(value)	((Toy_Literal){{ .function = { .inner = { .native = value }, .scope = NULL }}, TOY_LITERAL_FUNCTION_NATIVE, 0})
#define TOY_TO_FUNCTION_HOOK_LITERAL(value)		((Toy_Literal){{ .function = { .inner = { .hook = value }, .scope = NULL }}, TOY_LITERAL_FUNCTION_HOOK, 0})
#define TOY_TO_IDENTIFIER_LITERAL(value)		Toy_private_toIdentifierLiteral(value)
#define TOY_TO_TYPE_LITERAL(value, c)			((Toy_Literal){{ .type = { .typeOf = value, .constant = c, .subtypes = NULL, .capacity = 0, .count = 0 }}, TOY_LITERAL_TYPE, 0})
#define TOY_TO_OPAQUE_LITERAL(value, t)			((Toy_Literal){{ .opaque = { .ptr = value, .tag = t }}, TOY_LITERAL_OPAQUE, 0})

//BUGFIX: For blank indexing
#define TOY_IS_INDEX_BLANK(value)				((value).type == TOY_LITERAL_INDEX_BLANK)
#define TOY_TO_INDEX_BLANK_LITERAL				((Toy_Literal){{ .integer = 0 }, TOY_LITERAL_INDEX_BLANK, 0})

TOY_API void Toy_freeLiteral(Toy_Literal literal);

#define TOY_IS_TRUTHY(x) Toy_private_isTruthy(x)

#define TOY_AS_FUNCTION_BYTECODE_LENGTH(lit)	((lit).bytecodeLength)

#define TOY_MAX_STRING_LENGTH					4096
#define TOY_HASH_I(lit)							((lit).as.identifier.hash)
#define TOY_TYPE_PUSH_SUBTYPE(lit, subtype)		Toy_private_typePushSubtype(lit, subtype)
#define TOY_GET_OPAQUE_TAG(o)					o.as.opaque.tag

//BUGFIX: macros are not functions
TOY_API bool Toy_private_isTruthy(Toy_Literal x);
TOY_API Toy_Literal Toy_private_toStringLiteral(Toy_RefString* ptr);
TOY_API Toy_Literal Toy_private_toIdentifierLiteral(Toy_RefString* ptr);
TOY_API Toy_Literal* Toy_private_typePushSubtype(Toy_Literal* lit, Toy_Literal subtype);

//utils
TOY_API Toy_Literal Toy_copyLiteral(Toy_Literal original);
TOY_API bool Toy_literalsAreEqual(Toy_Literal lhs, Toy_Literal rhs);
TOY_API int Toy_hashLiteral(Toy_Literal lit);

//not thread-safe
TOY_API void Toy_printLiteral(Toy_Literal literal);
TOY_API void Toy_printLiteralCustom(Toy_Literal literal, Toy_PrintFn);
