#include "toy_literal.h"
#include "toy_memory.h"

#include "toy_literal_array.h"
#include "toy_literal_dictionary.h"
#include "toy_scope.h"

#include "toy_console_colors.h"

#include <stdio.h>
#include <string.h>

//hash util functions
static unsigned int hashString(const char* string, int length) {
	unsigned int hash = 2166136261u;

	for (int i = 0; i < length; i++) {
		hash *= string[i];
		hash ^= 16777619;
	}

	return hash;
}

static unsigned int hashUInt(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

//exposed functions
void Toy_freeLiteral(Toy_Literal literal) {
	//refstrings
	if (TOY_IS_STRING(literal)) {
		Toy_deleteRefString(TOY_AS_STRING(literal));
		return;
	}

	if (TOY_IS_IDENTIFIER(literal)) {
		Toy_deleteRefString(TOY_AS_IDENTIFIER(literal));
		return;
	}

	//compounds
	if (TOY_IS_ARRAY(literal) || literal.type == TOY_LITERAL_ARRAY_INTERMEDIATE || literal.type == TOY_LITERAL_DICTIONARY_INTERMEDIATE || literal.type == TOY_LITERAL_TYPE_INTERMEDIATE) {
		Toy_freeLiteralArray(TOY_AS_ARRAY(literal));
		TOY_FREE(Toy_LiteralArray, TOY_AS_ARRAY(literal));
		return;
	}

	if (TOY_IS_DICTIONARY(literal)) {
		Toy_freeLiteralDictionary(TOY_AS_DICTIONARY(literal));
		TOY_FREE(Toy_LiteralDictionary, TOY_AS_DICTIONARY(literal));
		return;
	}

	//complex literals
	if (TOY_IS_FUNCTION(literal)) {
		Toy_popScope(TOY_AS_FUNCTION(literal).scope);
		TOY_AS_FUNCTION(literal).scope = NULL;
		TOY_FREE_ARRAY(unsigned char, TOY_AS_FUNCTION(literal).inner.bytecode, TOY_AS_FUNCTION_BYTECODE_LENGTH(literal));
	}

	if (TOY_IS_TYPE(literal) && TOY_AS_TYPE(literal).capacity > 0) {
		for (int i = 0; i < TOY_AS_TYPE(literal).count; i++) {
			Toy_freeLiteral(((Toy_Literal*)(TOY_AS_TYPE(literal).subtypes))[i]);
		}
		TOY_FREE_ARRAY(Toy_Literal, TOY_AS_TYPE(literal).subtypes, TOY_AS_TYPE(literal).capacity);
		return;
	}
}

bool Toy_private_isTruthy(Toy_Literal x) {
	if (TOY_IS_NULL(x)) {
		fprintf(stderr, TOY_CC_ERROR "Null is neither true nor false\n" TOY_CC_RESET);
		return false;
	}

	if (TOY_IS_BOOLEAN(x)) {
		return TOY_AS_BOOLEAN(x);
	}

	return true;
}

Toy_Literal Toy_private_toStringLiteral(Toy_RefString* ptr) {
	return ((Toy_Literal){{ .string = { .ptr = ptr }},TOY_LITERAL_STRING, 0});
}

Toy_Literal Toy_private_toIdentifierLiteral(Toy_RefString* ptr) {
	return ((Toy_Literal){{ .identifier = { .ptr = ptr, .hash = hashString(Toy_toCString(ptr), Toy_lengthRefString(ptr)) }},TOY_LITERAL_IDENTIFIER, 0});
}

Toy_Literal* Toy_private_typePushSubtype(Toy_Literal* lit, Toy_Literal subtype) {
	//grow the subtype array
	if (TOY_AS_TYPE(*lit).count + 1 > TOY_AS_TYPE(*lit).capacity) {
		int oldCapacity = TOY_AS_TYPE(*lit).capacity;

		TOY_AS_TYPE(*lit).capacity = TOY_GROW_CAPACITY(oldCapacity);
		TOY_AS_TYPE(*lit).subtypes = TOY_GROW_ARRAY(Toy_Literal, TOY_AS_TYPE(*lit).subtypes, oldCapacity, TOY_AS_TYPE(*lit).capacity);
	}

	//actually push
	((Toy_Literal*)(TOY_AS_TYPE(*lit).subtypes))[ TOY_AS_TYPE(*lit).count++ ] = subtype;
	return &((Toy_Literal*)(TOY_AS_TYPE(*lit).subtypes))[ TOY_AS_TYPE(*lit).count - 1 ];
}

Toy_Literal Toy_copyLiteral(Toy_Literal original) {
	switch(original.type) {
		case TOY_LITERAL_NULL:
		case TOY_LITERAL_BOOLEAN:
		case TOY_LITERAL_INTEGER:
		case TOY_LITERAL_FLOAT:
			//no copying needed
			return original;

		case TOY_LITERAL_STRING: {
			return TOY_TO_STRING_LITERAL(Toy_copyRefString(TOY_AS_STRING(original)));
		}

		case TOY_LITERAL_ARRAY: {
			Toy_LiteralArray* array = TOY_ALLOCATE(Toy_LiteralArray, 1);
			Toy_initLiteralArray(array);

			//copy each element
			for (int i = 0; i < TOY_AS_ARRAY(original)->count; i++) {
				Toy_pushLiteralArray(array, TOY_AS_ARRAY(original)->literals[i]);
			}

			return TOY_TO_ARRAY_LITERAL(array);
		}

		case TOY_LITERAL_DICTIONARY: {
			Toy_LiteralDictionary* dictionary = TOY_ALLOCATE(Toy_LiteralDictionary, 1);
			Toy_initLiteralDictionary(dictionary);

			//copy each entry
			for (int i = 0; i < TOY_AS_DICTIONARY(original)->capacity; i++) {
				if ( !TOY_IS_NULL(TOY_AS_DICTIONARY(original)->entries[i].key) ) {
					Toy_setLiteralDictionary(dictionary, TOY_AS_DICTIONARY(original)->entries[i].key, TOY_AS_DICTIONARY(original)->entries[i].value);
				}
			}

			return TOY_TO_DICTIONARY_LITERAL(dictionary);
		}

		case TOY_LITERAL_FUNCTION: {
			unsigned char* buffer = TOY_ALLOCATE(unsigned char, TOY_AS_FUNCTION_BYTECODE_LENGTH(original));
			memcpy(buffer, TOY_AS_FUNCTION(original).inner.bytecode, TOY_AS_FUNCTION_BYTECODE_LENGTH(original));

			Toy_Literal literal = TOY_TO_FUNCTION_LITERAL(buffer, TOY_AS_FUNCTION_BYTECODE_LENGTH(original));
			TOY_AS_FUNCTION(literal).scope = Toy_copyScope(TOY_AS_FUNCTION(original).scope);

			return literal;
		}

		case TOY_LITERAL_IDENTIFIER: {
			//NOTE: could optimise this by copying the hash manually, but it's a very small increase in performance
			return TOY_TO_IDENTIFIER_LITERAL(Toy_copyRefString(TOY_AS_IDENTIFIER(original)));
		}

		case TOY_LITERAL_TYPE: {
			Toy_Literal lit = TOY_TO_TYPE_LITERAL(TOY_AS_TYPE(original).typeOf, TOY_AS_TYPE(original).constant);

			for (int i = 0; i < TOY_AS_TYPE(original).count; i++) {
				TOY_TYPE_PUSH_SUBTYPE(&lit, Toy_copyLiteral( ((Toy_Literal*)(TOY_AS_TYPE(original).subtypes))[i] ));
			}

			return lit;
		}

		case TOY_LITERAL_OPAQUE: {
			return original; //literally a shallow copy
		}

		case TOY_LITERAL_ARRAY_INTERMEDIATE: {
			Toy_LiteralArray* array = TOY_ALLOCATE(Toy_LiteralArray, 1);
			Toy_initLiteralArray(array);

			//copy each element
			for (int i = 0; i < TOY_AS_ARRAY(original)->count; i++) {
				Toy_Literal literal = Toy_copyLiteral(TOY_AS_ARRAY(original)->literals[i]);
				Toy_pushLiteralArray(array, literal);
				Toy_freeLiteral(literal);
			}

			Toy_Literal ret = TOY_TO_ARRAY_LITERAL(array);
			ret.type = TOY_LITERAL_ARRAY_INTERMEDIATE;
			return ret;
		}

		case TOY_LITERAL_DICTIONARY_INTERMEDIATE: {
			Toy_LiteralArray* array = TOY_ALLOCATE(Toy_LiteralArray, 1);
			Toy_initLiteralArray(array);

			//copy each element
			for (int i = 0; i < TOY_AS_ARRAY(original)->count; i++) {
				Toy_Literal literal = Toy_copyLiteral(TOY_AS_ARRAY(original)->literals[i]);
				Toy_pushLiteralArray(array, literal);
				Toy_freeLiteral(literal);
			}

			Toy_Literal ret = TOY_TO_ARRAY_LITERAL(array);
			ret.type = TOY_LITERAL_DICTIONARY_INTERMEDIATE;
			return ret;
		}

		case TOY_LITERAL_TYPE_INTERMEDIATE: {
			Toy_LiteralArray* array = TOY_ALLOCATE(Toy_LiteralArray, 1);
			Toy_initLiteralArray(array);

			//copy each element
			for (int i = 0; i < TOY_AS_ARRAY(original)->count; i++) {
				Toy_Literal literal =  Toy_copyLiteral(TOY_AS_ARRAY(original)->literals[i]);
				Toy_pushLiteralArray(array, literal);
				Toy_freeLiteral(literal);
			}

			Toy_Literal ret = TOY_TO_ARRAY_LITERAL(array);
			ret.type = TOY_LITERAL_TYPE_INTERMEDIATE;
			return ret;
		}

		case TOY_LITERAL_FUNCTION_INTERMEDIATE: //caries a compiler
		case TOY_LITERAL_FUNCTION_NATIVE:
		case TOY_LITERAL_FUNCTION_HOOK:
		case TOY_LITERAL_INDEX_BLANK:
			//no copying possible
			return original;

		default:
			fprintf(stderr, TOY_CC_ERROR "Can't copy that literal type: %d\n" TOY_CC_RESET, original.type);
			return TOY_TO_NULL_LITERAL;
	}
}

bool Toy_literalsAreEqual(Toy_Literal lhs, Toy_Literal rhs) {
	//utility for other things
	if (lhs.type != rhs.type) {
		// ints and floats are compatible
		if ((TOY_IS_INTEGER(lhs) || TOY_IS_FLOAT(lhs)) && (TOY_IS_INTEGER(rhs) || TOY_IS_FLOAT(rhs))) {
			if (TOY_IS_INTEGER(lhs)) {
				return TOY_AS_INTEGER(lhs) == TOY_AS_FLOAT(rhs);
			}
			else {
				return TOY_AS_FLOAT(lhs) == TOY_AS_INTEGER(rhs);
			}
		}

		return false;
	}

	switch(lhs.type) {
		case TOY_LITERAL_NULL:
			return true; //can only be true because of the check above

		case TOY_LITERAL_BOOLEAN:
			return TOY_AS_BOOLEAN(lhs) == TOY_AS_BOOLEAN(rhs);

		case TOY_LITERAL_INTEGER:
			return TOY_AS_INTEGER(lhs) == TOY_AS_INTEGER(rhs);

		case TOY_LITERAL_FLOAT:
			return TOY_AS_FLOAT(lhs) == TOY_AS_FLOAT(rhs);

		case TOY_LITERAL_STRING:
			return Toy_equalsRefString(TOY_AS_STRING(lhs), TOY_AS_STRING(rhs));

		case TOY_LITERAL_ARRAY:
		case TOY_LITERAL_ARRAY_INTERMEDIATE:
		case TOY_LITERAL_DICTIONARY_INTERMEDIATE: //BUGFIX
		case TOY_LITERAL_TYPE_INTERMEDIATE: //BUGFIX: used for storing types as an array
			//mismatched sizes
			if (TOY_AS_ARRAY(lhs)->count != TOY_AS_ARRAY(rhs)->count) {
				return false;
			}

			//mismatched elements (in order)
			for (int i = 0; i < TOY_AS_ARRAY(lhs)->count; i++) {
				if (!Toy_literalsAreEqual( TOY_AS_ARRAY(lhs)->literals[i], TOY_AS_ARRAY(rhs)->literals[i] )) {
					return false;
				}
			}
			return true;

		case TOY_LITERAL_DICTIONARY:
			//relatively slow, especially when nested
			for (int i = 0; i < TOY_AS_DICTIONARY(lhs)->capacity; i++) {
				if (!TOY_IS_NULL(TOY_AS_DICTIONARY(lhs)->entries[i].key)) { //only compare non-null keys
					//check it exists in rhs
					if (!Toy_existsLiteralDictionary(TOY_AS_DICTIONARY(rhs), TOY_AS_DICTIONARY(lhs)->entries[i].key)) {
						return false;
					}

					//compare the values
					Toy_Literal val = Toy_getLiteralDictionary(TOY_AS_DICTIONARY(rhs), TOY_AS_DICTIONARY(lhs)->entries[i].key); //TODO: could be more efficient
					if (!Toy_literalsAreEqual(TOY_AS_DICTIONARY(lhs)->entries[i].value, val)) {
						Toy_freeLiteral(val);
						return false;
					}
					Toy_freeLiteral(val);
				}
			}

			return true;

		case TOY_LITERAL_FUNCTION:
		case TOY_LITERAL_FUNCTION_NATIVE:
		case TOY_LITERAL_FUNCTION_HOOK:
			return false; //functions are never equal
		break;

		case TOY_LITERAL_IDENTIFIER:
			//check shortcuts
			if (TOY_HASH_I(lhs) != TOY_HASH_I(rhs)) {
				return false;
			}

			return Toy_equalsRefString(TOY_AS_IDENTIFIER(lhs), TOY_AS_IDENTIFIER(rhs));

		case TOY_LITERAL_TYPE:
			//check types
			if (TOY_AS_TYPE(lhs).typeOf != TOY_AS_TYPE(rhs).typeOf) {
				return false;
			}

			//const don't match
			if (TOY_AS_TYPE(lhs).constant != TOY_AS_TYPE(rhs).constant) {
				return false;
			}

			//check subtypes
			if (TOY_AS_TYPE(lhs).count != TOY_AS_TYPE(rhs).count) {
				return false;
			}

			//check array|dictionary signatures are the same (in order)
			if (TOY_AS_TYPE(lhs).typeOf == TOY_LITERAL_ARRAY || TOY_AS_TYPE(lhs).typeOf == TOY_LITERAL_DICTIONARY) {
				for (int i = 0; i < TOY_AS_TYPE(lhs).count; i++) {
					if (!Toy_literalsAreEqual(((Toy_Literal*)(TOY_AS_TYPE(lhs).subtypes))[i], ((Toy_Literal*)(TOY_AS_TYPE(rhs).subtypes))[i])) {
						return false;
					}
				}
			}
			return true;

		case TOY_LITERAL_OPAQUE:
			return false; //IDK what this is!

		case TOY_LITERAL_ANY:
			return true;

		case TOY_LITERAL_FUNCTION_INTERMEDIATE:
			fprintf(stderr, TOY_CC_ERROR "[internal] Can't compare intermediate functions\n" TOY_CC_RESET);
			return false;

		case TOY_LITERAL_INDEX_BLANK:
			return false;

		default:
			//should never be seen
			fprintf(stderr, TOY_CC_ERROR "[internal] Unrecognized literal type in equality: %d\n" TOY_CC_RESET, lhs.type);
			return false;
	}

	return false;
}

int Toy_hashLiteral(Toy_Literal lit) {
	switch(lit.type) {
		case TOY_LITERAL_NULL:
			return 0;

		case TOY_LITERAL_BOOLEAN:
			return TOY_AS_BOOLEAN(lit) ? 1 : 0;

		case TOY_LITERAL_INTEGER:
			return hashUInt((unsigned int)TOY_AS_INTEGER(lit));

		case TOY_LITERAL_FLOAT:
			return hashUInt(*(unsigned int*)(&TOY_AS_FLOAT(lit)));

		case TOY_LITERAL_STRING:
			return hashString(Toy_toCString(TOY_AS_STRING(lit)), Toy_lengthRefString(TOY_AS_STRING(lit)));

		case TOY_LITERAL_ARRAY: {
			unsigned int res = 0;
			for (int i = 0; i < TOY_AS_ARRAY(lit)->count; i++) {
				res += Toy_hashLiteral(TOY_AS_ARRAY(lit)->literals[i]);
			}
			return hashUInt(res);
		}

		case TOY_LITERAL_DICTIONARY: {
			unsigned int res = 0;
			for (int i = 0; i < TOY_AS_DICTIONARY(lit)->capacity; i++) {
				if (!TOY_IS_NULL(TOY_AS_DICTIONARY(lit)->entries[i].key)) { //only hash non-null keys
					res += Toy_hashLiteral(TOY_AS_DICTIONARY(lit)->entries[i].key);
					res += Toy_hashLiteral(TOY_AS_DICTIONARY(lit)->entries[i].value);
				}
			}
			return hashUInt(res);
		}

		case TOY_LITERAL_FUNCTION:
		case TOY_LITERAL_FUNCTION_NATIVE:
		case TOY_LITERAL_FUNCTION_HOOK:
			return -1; //can't hash these

		case TOY_LITERAL_IDENTIFIER:
			return TOY_HASH_I(lit); //pre-computed

		case TOY_LITERAL_TYPE:
			return -1; //not much i can really do

		case TOY_LITERAL_OPAQUE:
		case TOY_LITERAL_ANY:
			return -1;

		default:
			//should never be seen
			fprintf(stderr, TOY_CC_ERROR "[internal] Unrecognized literal type in hash: %d\n" TOY_CC_RESET, lit.type);
			return 0;
	}
}

//utils
static void stdoutWrapper(const char* output) {
	printf("%s", output);
}

//buffer the prints
static char* globalPrintBuffer = NULL;
static size_t globalPrintCapacity = 0;
static size_t globalPrintCount = 0;

//BUGFIX: string quotes shouldn't show when just printing strings, but should show when printing them as members of something else
static char quotes = 0; //set to 0 to not show string quotes

static void printToBuffer(const char* str) {
	while (strlen(str) + globalPrintCount + 1 > globalPrintCapacity) {
		int oldCapacity = globalPrintCapacity;

		globalPrintCapacity = TOY_GROW_CAPACITY(globalPrintCapacity);
		globalPrintBuffer = TOY_GROW_ARRAY(char, globalPrintBuffer, oldCapacity, globalPrintCapacity);
	}

	snprintf(globalPrintBuffer + globalPrintCount, strlen(str) + 1, "%s", str);
	globalPrintCount += strlen(str);
}

//exposed functions
void Toy_printLiteral(Toy_Literal literal) {
	Toy_printLiteralCustom(literal, stdoutWrapper);
}

void Toy_printLiteralCustom(Toy_Literal literal, Toy_PrintFn printFn) {
	switch(literal.type) {
		case TOY_LITERAL_NULL:
			printFn("null");
		break;

		case TOY_LITERAL_BOOLEAN:
			printFn(TOY_AS_BOOLEAN(literal) ? "true" : "false");
		break;

		case TOY_LITERAL_INTEGER: {
			char buffer[256];
			snprintf(buffer, 256, "%d", TOY_AS_INTEGER(literal));
			printFn(buffer);
		}
		break;

		case TOY_LITERAL_FLOAT: {
			char buffer[256];

			if (TOY_AS_FLOAT(literal) - (int)TOY_AS_FLOAT(literal)) {
				snprintf(buffer, 256, "%g", TOY_AS_FLOAT(literal));
			}
			else {
				snprintf(buffer, 256, "%.1f", TOY_AS_FLOAT(literal));
			}

			printFn(buffer);
		}
		break;

		case TOY_LITERAL_STRING: {
			char buffer[TOY_MAX_STRING_LENGTH];
			if (!quotes) {
				snprintf(buffer, TOY_MAX_STRING_LENGTH, "%.*s", (int)Toy_lengthRefString(TOY_AS_STRING(literal)), Toy_toCString(TOY_AS_STRING(literal)));
			}
			else {
				snprintf(buffer, TOY_MAX_STRING_LENGTH, "%c%.*s%c", quotes, (int)Toy_lengthRefString(TOY_AS_STRING(literal)), Toy_toCString(TOY_AS_STRING(literal)), quotes);
			}
			printFn(buffer);
		}
		break;

		case TOY_LITERAL_ARRAY: {
			Toy_LiteralArray* ptr = TOY_AS_ARRAY(literal);

			//hold potential parent-call buffers on the C stack
			char* cacheBuffer = globalPrintBuffer;
			globalPrintBuffer = NULL;
			int cacheCapacity = globalPrintCapacity;
			globalPrintCapacity = 0;
			int cacheCount = globalPrintCount;
			globalPrintCount = 0;

			//print the contents to the global buffer
			printToBuffer("[");
			for (int i = 0; i < ptr->count; i++) {
				quotes = '"';
				Toy_printLiteralCustom(ptr->literals[i], printToBuffer);

				if (i + 1 < ptr->count) {
					printToBuffer(",");
				}
			}
			printToBuffer("]");

			//swap the parent-call buffer back into place
			char* printBuffer = globalPrintBuffer;
			int printCapacity = globalPrintCapacity;
			int printCount = globalPrintCount;

			globalPrintBuffer = cacheBuffer;
			globalPrintCapacity = cacheCapacity;
			globalPrintCount = cacheCount;

			//finally, output and cleanup
			printFn(printBuffer);
			TOY_FREE_ARRAY(char, printBuffer, printCapacity);
			quotes = 0;
		}
		break;

		case TOY_LITERAL_DICTIONARY: {
			Toy_LiteralDictionary* ptr = TOY_AS_DICTIONARY(literal);

			//hold potential parent-call buffers on the C stack
			char* cacheBuffer = globalPrintBuffer;
			globalPrintBuffer = NULL;
			int cacheCapacity = globalPrintCapacity;
			globalPrintCapacity = 0;
			int cacheCount = globalPrintCount;
			globalPrintCount = 0;

			//print the contents to the global buffer
			int delimCount = 0;
			printToBuffer("[");
			for (int i = 0; i < ptr->capacity; i++) {
				if (TOY_IS_NULL(ptr->entries[i].key)) {
					continue;
				}

				if (delimCount++ > 0) {
					printToBuffer(",");
				}

				quotes = '"';
				Toy_printLiteralCustom(ptr->entries[i].key, printToBuffer);
				printToBuffer(":");
				quotes = '"';
				Toy_printLiteralCustom(ptr->entries[i].value, printToBuffer);
			}

			//empty dicts MUST have a ":" printed
			if (ptr->count == 0) {
				printToBuffer(":");
			}

			printToBuffer("]");

			//swap the parent-call buffer back into place
			char* printBuffer = globalPrintBuffer;
			int printCapacity = globalPrintCapacity;
			int printCount = globalPrintCount;

			globalPrintBuffer = cacheBuffer;
			globalPrintCapacity = cacheCapacity;
			globalPrintCount = cacheCount;

			//finally, output and cleanup
			printFn(printBuffer);
			TOY_FREE_ARRAY(char, printBuffer, printCapacity);
			quotes = 0;
		}
		break;

		case TOY_LITERAL_FUNCTION:
		case TOY_LITERAL_FUNCTION_NATIVE:
		case TOY_LITERAL_FUNCTION_HOOK:
			printFn("(function)");
		break;

		case TOY_LITERAL_IDENTIFIER: {
			char buffer[256];
			snprintf(buffer, 256, "%.*s", (int)Toy_lengthRefString(TOY_AS_IDENTIFIER(literal)), Toy_toCString(TOY_AS_IDENTIFIER(literal)));
			printFn(buffer);
		}
		break;

		case TOY_LITERAL_TYPE: {
			//hold potential parent-call buffers on the C stack
			char* cacheBuffer = globalPrintBuffer;
			globalPrintBuffer = NULL;
			int cacheCapacity = globalPrintCapacity;
			globalPrintCapacity = 0;
			int cacheCount = globalPrintCount;
			globalPrintCount = 0;

			//print the type correctly
			printToBuffer("<");

			switch(TOY_AS_TYPE(literal).typeOf) {
				case TOY_LITERAL_NULL:
					printToBuffer("null");
				break;

				case TOY_LITERAL_BOOLEAN:
					printToBuffer("bool");
				break;

				case TOY_LITERAL_INTEGER:
					printToBuffer("int");
				break;

				case TOY_LITERAL_FLOAT:
					printToBuffer("float");
				break;

				case TOY_LITERAL_STRING:
					printToBuffer("string");
				break;

				case TOY_LITERAL_ARRAY:
					//print all in the array
					printToBuffer("[");
					for (int i = 0; i < TOY_AS_TYPE(literal).count; i++) {
						Toy_printLiteralCustom(((Toy_Literal*)(TOY_AS_TYPE(literal).subtypes))[i], printToBuffer);
					}
					printToBuffer("]");
				break;

				case TOY_LITERAL_DICTIONARY:
					printToBuffer("[");

					for (int i = 0; i < TOY_AS_TYPE(literal).count; i += 2) {
						Toy_printLiteralCustom(((Toy_Literal*)(TOY_AS_TYPE(literal).subtypes))[i], printToBuffer);
						printToBuffer(":");
						Toy_printLiteralCustom(((Toy_Literal*)(TOY_AS_TYPE(literal).subtypes))[i + 1], printToBuffer);
					}
					printToBuffer("]");
				break;

				case TOY_LITERAL_FUNCTION:
					printToBuffer("function");
				break;

				case TOY_LITERAL_FUNCTION_NATIVE:
					printToBuffer("native");
				break;

				case TOY_LITERAL_IDENTIFIER:
					printToBuffer("identifier");
				break;

				case TOY_LITERAL_TYPE:
					printToBuffer("type");
				break;

				case TOY_LITERAL_OPAQUE:
					printToBuffer("opaque");
				break;

				case TOY_LITERAL_ANY:
					printToBuffer("any");
				break;

				default:
					//should never be seen
					fprintf(stderr, TOY_CC_ERROR "[internal] Unrecognized literal type in print type: %d\n" TOY_CC_RESET, TOY_AS_TYPE(literal).typeOf);
			}

			//const (printed last)
			if (TOY_AS_TYPE(literal).constant) {
				printToBuffer(" const");
			}

			printToBuffer(">");

			//swap the parent-call buffer back into place
			char* printBuffer = globalPrintBuffer;
			int printCapacity = globalPrintCapacity;
			int printCount = globalPrintCount;

			globalPrintBuffer = cacheBuffer;
			globalPrintCapacity = cacheCapacity;
			globalPrintCount = cacheCount;

			//finally, output and cleanup
			printFn(printBuffer);
			TOY_FREE_ARRAY(char, printBuffer, printCapacity);
			quotes = 0;
		}
		break;

		case TOY_LITERAL_TYPE_INTERMEDIATE:
		case TOY_LITERAL_FUNCTION_INTERMEDIATE:
			printFn("Unprintable literal found");
		break;

		case TOY_LITERAL_OPAQUE:
			printFn("(opaque)");
		break;

		case TOY_LITERAL_ANY:
			printFn("(any)");
		break;

		default:
			//should never be seen
			fprintf(stderr, TOY_CC_ERROR "[internal] Unrecognized literal type in print: %d\n" TOY_CC_RESET, literal.type);
	}
}
