#pragma once

/*!
# toy_literal.h

This header defines the literal structure, which is used extensively throughout Toy to represent values of some kind.

The main way of interacting with literals is to use a macro of some kind, as the exact implementation of `Toy_Literal` has and will change based on the needs of Toy.

User data can be passed around within Toy as an opaque type - use the tag value for determining what kind of opaque it is, or leave it as 0.
!*/

#include "toy_common.h"

#include "toy_refstring.h"
#include "toy_reffunction.h"

//forward delcare stuff
struct Toy_Literal;
struct Toy_Interpreter;
struct Toy_LiteralArray;
struct Toy_LiteralDictionary;
struct Toy_Scope;
typedef int (*Toy_NativeFn)(struct Toy_Interpreter* interpreter, struct Toy_LiteralArray* arguments);
typedef int (*Toy_HookFn)(struct Toy_Interpreter* interpreter, struct Toy_Literal identifier, struct Toy_Literal alias);
typedef void (*Toy_PrintFn)(const char*);

/*!
## Defined Enums

### Toy_LiteralType

* `TOY_LITERAL_NULL`
* `TOY_LITERAL_BOOLEAN`
* `TOY_LITERAL_INTEGER`
* `TOY_LITERAL_FLOAT`
* `TOY_LITERAL_STRING`
* `TOY_LITERAL_ARRAY`
* `TOY_LITERAL_DICTIONARY`
* `TOY_LITERAL_FUNCTION`
* `TOY_LITERAL_FUNCTION_NATIVE`
* `TOY_LITERAL_FUNCTION_HOOK`
* `TOY_LITERAL_IDENTIFIER`
* `TOY_LITERAL_TYPE`
* `TOY_LITERAL_OPAQUE`
* `TOY_LITERAL_ANY`

These are the main values of `Toy_LiteralType`, each of which represents a potential state of the `Toy_Literal` structure. Do not interact with a literal without determining its type with the `IS_*` macros first.

Other type values are possible, but are only used internally.
!*/

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
		int integer; //4
		float number;//4

		struct {
			Toy_RefString* ptr; //8
			//string hash?
		} string; //8

		struct Toy_LiteralArray* array; //8
		struct Toy_LiteralDictionary* dictionary; //8

		struct {
			union {
				Toy_RefFunction* ptr;  //8
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
			struct Toy_Literal* subtypes; //8
			Toy_LiteralType typeOf;  //4
			unsigned char capacity; //1
			unsigned char count; //1
			bool constant; //1
		} type; //16

		struct {
			void* ptr; //8
			int tag; //4
		} opaque; //16

		void* generic; //8
	} as; //16

	Toy_LiteralType type; //4
	//4 - unused
	//shenanigans with byte alignment reduces the size of Toy_Literal
} Toy_Literal;

/*!
## Defined Macros
!*/

/*!
The following macros are used to determine if a given literal, passed in as `value`, is of a specific type. It should be noted that `TOY_IS_FUNCTION` will return false for native and hook functions.

* `TOY_IS_NULL(value)`
* `TOY_IS_BOOLEAN(value)`
* `TOY_IS_INTEGER(value)`
* `TOY_IS_FLOAT(value)`
* `TOY_IS_STRING(value)`
* `TOY_IS_ARRAY(value)`
* `TOY_IS_DICTIONARY(value)`
* `TOY_IS_FUNCTION(value)`
* `TOY_IS_FUNCTION_NATIVE(value)`
* `TOY_IS_FUNCTION_HOOK(value)`
* `TOY_IS_IDENTIFIER(value)`
* `TOY_IS_TYPE(value)`
* `TOY_IS_OPAQUE(value)`
!*/

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

/*!
The following macros are used to cast a literal to a specific C type to be used.

* `TOY_AS_BOOLEAN(value)`
* `TOY_AS_INTEGER(value)`
* `TOY_AS_FLOAT(value)`
* `TOY_AS_STRING(value)`
* `TOY_AS_ARRAY(value)`
* `TOY_AS_DICTIONARY(value)`
* `TOY_AS_FUNCTION(value)`
* `TOY_AS_FUNCTION_NATIVE(value)`
* `TOY_AS_FUNCTION_HOOK(value)`
* `TOY_AS_IDENTIFIER(value)`
* `TOY_AS_TYPE(value)`
* `TOY_AS_OPAQUE(value)`
!*/

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

/*!
The following macros are used to create a new literal, with the given `value` as it's internal value.

* `TOY_TO_NULL_LITERAL` - does not need parantheses
* `TOY_TO_BOOLEAN_LITERAL(value)`
* `TOY_TO_INTEGER_LITERAL(value)`
* `TOY_TO_FLOAT_LITERAL(value)`
* `TOY_TO_STRING_LITERAL(value)`
* `TOY_TO_ARRAY_LITERAL(value)`
* `TOY_TO_DICTIONARY_LITERAL(value)`
* `TOY_TO_FUNCTION_LITERAL(value, l)` - `l` represents the length of the bytecode passed as `value`
* `TOY_TO_FUNCTION_NATIVE_LITERAL(value)`
* `TOY_TO_FUNCTION_HOOK_LITERAL(value)`
* `TOY_TO_IDENTIFIER_LITERAL(value)`
* `TOY_TO_TYPE_LITERAL(value, c)` - `c` is the true of the type should be const
* `TOY_TO_OPAQUE_LITERAL(value, t)` - `t` is the integer tag
!*/

#define TOY_TO_NULL_LITERAL						((Toy_Literal){{ .integer = 0 }, TOY_LITERAL_NULL})
#define TOY_TO_BOOLEAN_LITERAL(value)			((Toy_Literal){{ .boolean = value }, TOY_LITERAL_BOOLEAN})
#define TOY_TO_INTEGER_LITERAL(value)			((Toy_Literal){{ .integer = value }, TOY_LITERAL_INTEGER})
#define TOY_TO_FLOAT_LITERAL(value)				((Toy_Literal){{ .number = value }, TOY_LITERAL_FLOAT})
#define TOY_TO_STRING_LITERAL(value)			((Toy_Literal){{ .string = { .ptr = value }},TOY_LITERAL_STRING})
#define TOY_TO_ARRAY_LITERAL(value)				((Toy_Literal){{ .array = value }, TOY_LITERAL_ARRAY})
#define TOY_TO_DICTIONARY_LITERAL(value)		((Toy_Literal){{ .dictionary = value }, TOY_LITERAL_DICTIONARY})
#define TOY_TO_FUNCTION_LITERAL(value)			((Toy_Literal){{ .function = { .inner = { .ptr = value }, .scope = NULL }}, TOY_LITERAL_FUNCTION})
#define TOY_TO_FUNCTION_NATIVE_LITERAL(value)	((Toy_Literal){{ .function = { .inner = { .native = value }, .scope = NULL }}, TOY_LITERAL_FUNCTION_NATIVE})
#define TOY_TO_FUNCTION_HOOK_LITERAL(value)		((Toy_Literal){{ .function = { .inner = { .hook = value }, .scope = NULL }}, TOY_LITERAL_FUNCTION_HOOK})
#define TOY_TO_IDENTIFIER_LITERAL(value)		Toy_private_toIdentifierLiteral(value)
#define TOY_TO_TYPE_LITERAL(value, c)			((Toy_Literal){{ .type = { .typeOf = value, .constant = c, .subtypes = NULL, .capacity = 0, .count = 0 }}, TOY_LITERAL_TYPE})
#define TOY_TO_OPAQUE_LITERAL(value, t)			((Toy_Literal){{ .opaque = { .ptr = value, .tag = t }}, TOY_LITERAL_OPAQUE})

//BUGFIX: For blank indexing - not for general use
#define TOY_IS_INDEX_BLANK(value)				((value).type == TOY_LITERAL_INDEX_BLANK)
#define TOY_TO_INDEX_BLANK_LITERAL				((Toy_Literal){{ .integer = 0 }, TOY_LITERAL_INDEX_BLANK})

/*!
## More Defined Macros

The following macros are utilities used throughout Toy's internals, and are available for the user as well.
!*/

/*!
### TOY_IS_TRUTHY(x)

Returns true of the literal `x` is truthy, otherwise it returns false.

Currently, every value is considered truthy except `false`, which is falsy and `null`, which is neither true or false.
!*/

#define TOY_IS_TRUTHY(x) 						Toy_private_isTruthy(x)

/*!
### TOY_AS_FUNCTION_BYTECODE_LENGTH(lit)

Returns the length of a Toy function's bytecode.

This macro is only valid on `TOY_LITERAL_FUNCTION`.
!*/
#define TOY_AS_FUNCTION_BYTECODE_LENGTH(lit)	(Toy_lengthRefFunction((lit).inner.ptr))

/*!
### TOY_MAX_STRING_LENGTH

The maximum length of a string in Toy, which is 4096 bytes by default. This can be changed at compile time, but the results of doing so are not officially supported.
!*/
#define TOY_MAX_STRING_LENGTH					4096

/*!
### TOY_HASH_I(lit)

Identifiers are the names of values within Toy; to speed up execution, their "hash value" is computed at compile time and stored within them. Use this to access it, if needed.

This macro is only valid on `TOY_LITERAL_IDENTIFIER`.
!*/
#define TOY_HASH_I(lit)							((lit).as.identifier.hash)

/*!
### TOY_TYPE_PUSH_SUBTYPE(lit, subtype)

When building a complex type, such as the type of an array or dictionary, you may need to specify inner types. Use this to push a `subtype`. calling `Toy_freeLiteral()` on the outermost type should clean up all inner types, as expected.

This macro returns the index of the newly pushed value within it's parent.

This macro is only valid on `TOY_LITERAL_TYPE`, for both `type` and `subtype`.
!*/
#define TOY_TYPE_PUSH_SUBTYPE(lit, subtype)		Toy_private_typePushSubtype(lit, subtype)

/*!
### TOY_GET_OPAQUE_TAG(o)

Returns the value of the opaque `o`'s tag.

This macro is only valid on `TOY_LITERAL_OPAQUE`.
!*/
#define TOY_GET_OPAQUE_TAG(o)					o.as.opaque.tag

/*!
## Defined Functions
!*/

/*!
### void Toy_freeLiteral(Toy_Literal literal)

This function frees the given literal's memory. Any internal pointers are now invalid.

This function should be called on EVERY literal when it is no longer needed, regardless of type.
!*/
TOY_API void Toy_freeLiteral(Toy_Literal literal);

/*!
### Toy_Literal Toy_copyLiteral(Toy_Literal original)

This function returns a copy of the given literal. Literals should never be copied without this function, as it handles a lot of internal memory allocations.
!*/
TOY_API Toy_Literal Toy_copyLiteral(Toy_Literal original);

/*!
### bool Toy_literalsAreEqual(Toy_Literal lhs, Toy_Literal rhs)

This checks to see if two given literals are equal.

When an integer and a float are compared, the integer is cooerced into a float for the duration of the call.

Arrays or dictionaries are equal only if their keys and values all equal. Likewise, types only equal if all subtypes are equal, in order.

Functions and opaques are never equal to anything, while values with the type `TOY_LITERAL_ANY` are always equal.
!*/
TOY_API bool Toy_literalsAreEqual(Toy_Literal lhs, Toy_Literal rhs);

/*!
### int Toy_hashLiteral(Toy_Literal lit)

This finds the hash of a literal, for various purposes. Different hashing algorithms are used for different types, and some types can't be hashed at all.

types that can't be hashed are

* all kinds of functions
* type
* opaque
* any

In the case of identifiers, their hashes are precomputed on creation and are stored within the literal.
!*/
TOY_API int Toy_hashLiteral(Toy_Literal lit);

/*!
### void Toy_printLiteral(Toy_Literal literal)

This wraps a call to `Toy_printLiteralCustom`, with a printf-stdout wrapper as `printFn`.
!*/
TOY_API void Toy_printLiteral(Toy_Literal literal);

/*!
### void Toy_printLiteralCustom(Toy_Literal literal, PrintFn printFn)

This function passes the string representation of `literal` to `printFn`. 

This function is not thread safe - due to the loopy and recursive nature of printing compound values, this function uses some globally persistent variables.
!*/
TOY_API void Toy_printLiteralCustom(Toy_Literal literal, Toy_PrintFn);

/*!
### bool Toy_private_isTruthy(Toy_Literal x)

Utilized by the `TOY_IS_TRUTHY` macro.

Private functions are not intended for general use.
!*/
TOY_API bool Toy_private_isTruthy(Toy_Literal x);

/*!
### bool Toy_private_toIdentifierLiteral(Toy_RefString* ptr)

Utilized by the `TOY_TO_IDENTIFIER_LITERAL` macro.

Private functions are not intended for general use.
!*/
TOY_API Toy_Literal Toy_private_toIdentifierLiteral(Toy_RefString* ptr);

/*!
### bool Toy_private_typePushSubtype(Toy_Literal* lit, Toy_Literal subtype)

Utilized by the `TOY_TYPE_PUSH_SUBTYPE` macro.

Private functions are not intended for general use.
!*/
TOY_API Toy_Literal* Toy_private_typePushSubtype(Toy_Literal* lit, Toy_Literal subtype);
