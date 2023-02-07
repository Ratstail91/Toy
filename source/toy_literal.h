#pragma once

#include "toy_common.h"

#include "toy_refstring.h"

//forward delcare stuff
struct Toy_Literal;
struct Toy_Interpreter;
struct Toy_LiteralArray;
typedef int (*Toy_NativeFn)(struct Toy_Interpreter* interpreter, struct Toy_LiteralArray* arguments);
typedef int (*Toy_HookFn)(struct Toy_Interpreter* interpreter, struct Toy_Literal identifier, struct Toy_Literal alias);

#include <string.h>

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
	Toy_LiteralType type;
	union {
		bool boolean;
		int integer;
		float number;
		struct {
			Toy_RefString* ptr;
			//string hash?
		} string;

		void* array;
		void* dictionary;

		struct {
			void* bytecode;
			Toy_NativeFn native; //already a pointer
			Toy_HookFn hook; //already a pointer
			void* scope;
			int length;
		} function;

		struct { //for variable names
            Toy_RefString* ptr;
			int hash;
        } identifier;

		struct {
			Toy_LiteralType typeOf;
			bool constant;
			void* subtypes; //for nested types caused by compounds
			int capacity;
			int count;
		} type;

		struct {
			void* ptr;
			int tag;
		} opaque;
	} as;
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
#define TOY_AS_FUNCTION_NATIVE(value)			((value).as.function.native)
#define TOY_AS_FUNCTION_HOOK(value)				((value).as.function.hook)
#define TOY_AS_IDENTIFIER(value)				((value).as.identifier.ptr)
#define TOY_AS_TYPE(value)						((value).as.type)
#define TOY_AS_OPAQUE(value)					((value).as.opaque.ptr)

#define TOY_TO_NULL_LITERAL						((Toy_Literal){TOY_LITERAL_NULL,			{ .integer = 0 }})
#define TOY_TO_BOOLEAN_LITERAL(value)			((Toy_Literal){TOY_LITERAL_BOOLEAN,			{ .boolean = value }})
#define TOY_TO_INTEGER_LITERAL(value)			((Toy_Literal){TOY_LITERAL_INTEGER,			{ .integer = value }})
#define TOY_TO_FLOAT_LITERAL(value)				((Toy_Literal){TOY_LITERAL_FLOAT,			{ .number = value }})
#define TOY_TO_STRING_LITERAL(value)			Toy_private_toStringLiteral(value)
#define TOY_TO_ARRAY_LITERAL(value)				((Toy_Literal){TOY_LITERAL_ARRAY,			{ .array = value }})
#define TOY_TO_DICTIONARY_LITERAL(value)		((Toy_Literal){TOY_LITERAL_DICTIONARY,		{ .dictionary = value }})
#define TOY_TO_FUNCTION_LITERAL(value, l)		((Toy_Literal){TOY_LITERAL_FUNCTION,		{ .function.bytecode = value, .function.scope = NULL, .function.length = l }})
#define TOY_TO_FUNCTION_NATIVE_LITERAL(value)	((Toy_Literal){TOY_LITERAL_FUNCTION_NATIVE,	{ .function.native = value, .function.scope = NULL, .function.length = 0 }})
#define TOY_TO_FUNCTION_HOOK_LITERAL(value)		((Toy_Literal){TOY_LITERAL_FUNCTION_HOOK,	{ .function.hook = value, .function.scope = NULL, .function.length = 0 }})
#define TOY_TO_IDENTIFIER_LITERAL(value)		Toy_private_toIdentifierLiteral(value)
#define TOY_TO_TYPE_LITERAL(value, c)			((Toy_Literal){ TOY_LITERAL_TYPE,			{ .type.typeOf = value, .type.constant = c, .type.subtypes = NULL, .type.capacity = 0, .type.count = 0 }})
#define TOY_TO_OPAQUE_LITERAL(value, t)			((Toy_Literal){ TOY_LITERAL_OPAQUE,			{ .opaque.ptr = value, .opaque.tag = t }})

//BUGFIX: For blank indexing
#define TOY_IS_INDEX_BLANK(value)				((value).type == TOY_LITERAL_INDEX_BLANK)
#define TOY_TO_INDEX_BLANK_LITERAL				((Toy_Literal){TOY_LITERAL_INDEX_BLANK,	{ .integer = 0 }})

TOY_API void Toy_freeLiteral(Toy_Literal literal);

#define TOY_IS_TRUTHY(x) Toy_private_isTruthy(x)

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

TOY_API void Toy_printLiteral(Toy_Literal literal);
TOY_API void Toy_printLiteralCustom(Toy_Literal literal, void (printFn)(const char*));
