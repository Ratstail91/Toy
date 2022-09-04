#include "literal.h"
#include "memory.h"

#include "literal_array.h"
#include "literal_dictionary.h"
#include "scope.h"

#include "console_colors.h"

#include <stdio.h>

//hash util functions
static unsigned int hashString(const char* string, int length) {
	unsigned int hash = 2166136261u;

	for (int i = 0; i < length; i++) {
		hash *= string[i];
		hash *= 16777619;
	}

	return hash;
}

static unsigned int hash(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

//exposed functions
void freeLiteral(Literal literal) {
	if (IS_STRING(literal)) {
		FREE_ARRAY(char, AS_STRING(literal), literal.as.string.length + 1);
		return;
	}

	if (IS_ARRAY(literal) || literal.type == LITERAL_DICTIONARY_INTERMEDIATE || literal.type == LITERAL_TYPE_INTERMEDIATE) {
		freeLiteralArray(AS_ARRAY(literal));
		FREE(LiteralArray, AS_ARRAY(literal));
		return;
	}

	if (IS_DICTIONARY(literal)) {
		freeLiteralDictionary(AS_DICTIONARY(literal));
		FREE(LiteralDictionary, AS_DICTIONARY(literal));
		return;
	}

	if (IS_FUNCTION(literal)) {
		popScope(AS_FUNCTION(literal).scope);
		AS_FUNCTION(literal).scope = NULL;
		FREE_ARRAY(unsigned char, AS_FUNCTION(literal).bytecode, AS_FUNCTION(literal).length);
	}

	if (IS_IDENTIFIER(literal)) {
		FREE_ARRAY(char, AS_IDENTIFIER(literal), literal.as.identifier.length + 1);
		return;
	}

	if (IS_TYPE(literal)) {
		for (int i = 0; i < AS_TYPE(literal).count; i++) {
			freeLiteral(((Literal*)(AS_TYPE(literal).subtypes))[i]);
		}
		FREE_ARRAY(Literal, AS_TYPE(literal).subtypes, AS_TYPE(literal).capacity);
		return;
	}
}

bool _isTruthy(Literal x) {
	if (IS_NULL(x)) {
		fprintf(stderr, ERROR "ERROR: Null is neither true nor false" RESET);
		return false;
	}

	if (IS_BOOLEAN(x)) {
		return AS_BOOLEAN(x);
	}

	return true;
}

Literal _toStringLiteral(char* str, int length) {
	return ((Literal){LITERAL_STRING, { .string.ptr = (char*)str, .string.length = length }});
}

Literal _toIdentifierLiteral(char* str, int length) {
	return ((Literal){LITERAL_IDENTIFIER,{ .identifier.ptr = (char*)str, .identifier.length = length, .identifier.hash = hashString(str, length) }});
}

Literal* _typePushSubtype(Literal* lit, Literal subtype) {
	//grow the subtype array
	if (AS_TYPE(*lit).count + 1 > AS_TYPE(*lit).capacity) {
		int oldCapacity = AS_TYPE(*lit).capacity;

		AS_TYPE(*lit).capacity = GROW_CAPACITY(oldCapacity);
		AS_TYPE(*lit).subtypes = GROW_ARRAY(Literal, AS_TYPE(*lit).subtypes, oldCapacity, AS_TYPE(*lit).capacity);
	}

	//actually push
	((Literal*)(AS_TYPE(*lit).subtypes))[ AS_TYPE(*lit).count++ ] = subtype;
	return &((Literal*)(AS_TYPE(*lit).subtypes))[ AS_TYPE(*lit).count - 1 ];
}

Literal copyLiteral(Literal original) {
	switch(original.type) {
		case LITERAL_NULL:
		case LITERAL_BOOLEAN:
		case LITERAL_INTEGER:
		case LITERAL_FLOAT:
			//no copying needed
			return original;

		case LITERAL_STRING: {
			return TO_STRING_LITERAL(copyString(AS_STRING(original), strlen(AS_STRING(original))), strlen(AS_STRING(original)));
		}

		case LITERAL_ARRAY: {
			LiteralArray* array = ALLOCATE(LiteralArray, 1);
			initLiteralArray(array);

			//copy each element
			for (int i = 0; i < AS_ARRAY(original)->count; i++) {
				pushLiteralArray(array, AS_ARRAY(original)->literals[i]);
			}

			return TO_ARRAY_LITERAL(array);
		}

		case LITERAL_DICTIONARY: {
			LiteralDictionary* dictionary = ALLOCATE(LiteralDictionary, 1);
			initLiteralDictionary(dictionary);

			//copy each entry
			for (int i = 0; i < AS_DICTIONARY(original)->capacity; i++) {
				if ( !IS_NULL(AS_DICTIONARY(original)->entries[i].key) ) {
					setLiteralDictionary(dictionary, AS_DICTIONARY(original)->entries[i].key, AS_DICTIONARY(original)->entries[i].value);
				}
			}

			return TO_DICTIONARY_LITERAL(dictionary);
		}

		case LITERAL_FUNCTION: {
			unsigned char* buffer = ALLOCATE(unsigned char, AS_FUNCTION(original).length);
			memcpy(buffer, AS_FUNCTION(original).bytecode, AS_FUNCTION(original).length);

			Literal literal = TO_FUNCTION_LITERAL(buffer, AS_FUNCTION(original).length);
			AS_FUNCTION(literal).scope = copyScope(AS_FUNCTION(original).scope);

			return literal;
		}

		case LITERAL_IDENTIFIER: {
			 return TO_IDENTIFIER_LITERAL(copyString(AS_IDENTIFIER(original), strlen(AS_IDENTIFIER(original)) ), strlen(AS_IDENTIFIER(original)));
		}

		case LITERAL_TYPE: {
			Literal lit = TO_TYPE_LITERAL(AS_TYPE(original).typeOf, AS_TYPE(original).constant);

			for (int i = 0; i < AS_TYPE(original).count; i++) {
				TYPE_PUSH_SUBTYPE(&lit, copyLiteral( ((Literal*)(AS_TYPE(original).subtypes))[i] ));
			}

			return lit;
		}

		case LITERAL_DICTIONARY_INTERMEDIATE: {
			LiteralArray* array = ALLOCATE(LiteralArray, 1);
			initLiteralArray(array);

			//copy each element
			for (int i = 0; i < AS_ARRAY(original)->count; i++) {
				Literal literal = copyLiteral(AS_ARRAY(original)->literals[i]);
				pushLiteralArray(array, literal);
				freeLiteral(literal);
			}

			Literal ret = TO_ARRAY_LITERAL(array);
			ret.type = LITERAL_DICTIONARY_INTERMEDIATE;
			return ret;
		}

		case LITERAL_TYPE_INTERMEDIATE: {
			LiteralArray* array = ALLOCATE(LiteralArray, 1);
			initLiteralArray(array);

			//copy each element
			for (int i = 0; i < AS_ARRAY(original)->count; i++) {
				Literal literal =  copyLiteral(AS_ARRAY(original)->literals[i]);
				pushLiteralArray(array, literal);
				freeLiteral(literal);
			}

			Literal ret = TO_ARRAY_LITERAL(array);
			ret.type = LITERAL_TYPE_INTERMEDIATE;
			return ret;
		}

		case LITERAL_FUNCTION_INTERMEDIATE: //caries a compiler
		case LITERAL_FUNCTION_NATIVE:
			//no copying possible
			return original;

		default:
			fprintf(stderr, ERROR "ERROR: Can't copy that literal type: %d\n" RESET, original.type);
			return TO_NULL_LITERAL;
	}
}

char* copyString(char* original, int length) {
	//make a local copy of the char array
	char* buffer = ALLOCATE(char, length + 1);
	strncpy(buffer, original, length);
	buffer[length] = '\0';
	return buffer;
}

bool literalsAreEqual(Literal lhs, Literal rhs) {
	//utility for other things
	if (lhs.type != rhs.type) {
		// ints and floats are compatible
		if ((IS_INTEGER(lhs) || IS_FLOAT(lhs)) && (IS_INTEGER(rhs) || IS_FLOAT(rhs))) {
			if (IS_INTEGER(lhs)) {
				return AS_INTEGER(lhs) + AS_FLOAT(rhs);
			}
			else {
				return AS_FLOAT(lhs) + AS_INTEGER(rhs);
			}
		}

		return false;
	}

	switch(lhs.type) {
		case LITERAL_NULL:
			return true; //can only be true because of the check above

		case LITERAL_BOOLEAN:
			return AS_BOOLEAN(lhs) == AS_BOOLEAN(rhs);

		case LITERAL_INTEGER:
			return AS_INTEGER(lhs) == AS_INTEGER(rhs);

		case LITERAL_FLOAT:
			return AS_FLOAT(lhs) == AS_FLOAT(rhs);

		case LITERAL_STRING:
			if (strlen(AS_STRING(lhs)) != strlen(AS_STRING(rhs))) {
				return false;
			}
			return !strncmp(AS_STRING(lhs), AS_STRING(rhs), strlen(AS_STRING(lhs)));

		case LITERAL_ARRAY:
		case LITERAL_DICTIONARY_INTERMEDIATE: //BUGFIX
		case LITERAL_TYPE_INTERMEDIATE: //BUGFIX: used for storing types as an array
			//mismatched sizes
			if (AS_ARRAY(lhs)->count != AS_ARRAY(rhs)->count) {
				return false;
			}

			//mismatched elements (in order)
			for (int i = 0; i < AS_ARRAY(lhs)->count; i++) {
				if (!literalsAreEqual( AS_ARRAY(lhs)->literals[i], AS_ARRAY(rhs)->literals[i] )) {
					return false;
				}
			}
			return true;

		case LITERAL_DICTIONARY:
			//relatively slow, especially when nested
			for (int i = 0; i < AS_DICTIONARY(lhs)->capacity; i++) {
				if (!IS_NULL(AS_DICTIONARY(lhs)->entries[i].key)) { //only compare non-null keys
					//check it exists in rhs
					if (!existsLiteralDictionary(AS_DICTIONARY(rhs), AS_DICTIONARY(lhs)->entries[i].key)) {
						return false;
					}

					//compare the values
					Literal val = getLiteralDictionary(AS_DICTIONARY(rhs), AS_DICTIONARY(lhs)->entries[i].key); //TODO: could be more efficient
					if (!literalsAreEqual(AS_DICTIONARY(lhs)->entries[i].value, val)) {
						freeLiteral(val);
						return false;
					}
					freeLiteral(val);
				}
			}

			return true;

		case LITERAL_FUNCTION:
		case LITERAL_FUNCTION_NATIVE:
			return false; //functions are never equal
		break;

		case LITERAL_IDENTIFIER:
			//check shortcuts
			if (HASH_I(lhs) != HASH_I(rhs) && strlen(AS_IDENTIFIER(lhs)) != strlen(AS_IDENTIFIER(rhs))) {
				return false;
			}

			return !strncmp(AS_IDENTIFIER(lhs), AS_IDENTIFIER(rhs), strlen( AS_IDENTIFIER(lhs) ));

		case LITERAL_TYPE:
			//check types
			if (AS_TYPE(lhs).typeOf != AS_TYPE(rhs).typeOf) {
				return false;
			}

			//const don't match
			if (AS_TYPE(lhs).constant != AS_TYPE(rhs).constant) {
				return false;
			}

			//check subtypes
			if (AS_TYPE(lhs).count != AS_TYPE(rhs).count) {
				return false;
			}

			//check array|dictionary signatures are the same (in order)
			if (AS_TYPE(lhs).typeOf == LITERAL_ARRAY || AS_TYPE(lhs).typeOf == LITERAL_DICTIONARY) {
				for (int i = 0; i < AS_TYPE(lhs).count; i++) {
					if (!literalsAreEqual(((Literal*)(AS_TYPE(lhs).subtypes))[i], ((Literal*)(AS_TYPE(rhs).subtypes))[i])) {
						return false;
					}
				}
			}
			return true;

		case LITERAL_ANY:
			return true;

		case LITERAL_FUNCTION_INTERMEDIATE:
			fprintf(stderr, ERROR "[internal] Can't compare intermediate functions\n" RESET);
			return false;

		default:
			//should never be seen
			fprintf(stderr, ERROR "[internal] Unrecognized literal type in equality: %d\n" RESET, lhs.type);
			return false;
	}

	return false;
}

int hashLiteral(Literal lit) {
	switch(lit.type) {
		case LITERAL_NULL:
			return 0;

		case LITERAL_BOOLEAN:
			return AS_BOOLEAN(lit) ? 1 : 0;

		case LITERAL_INTEGER:
			return hash((unsigned int)AS_INTEGER(lit));

		case LITERAL_FLOAT:
			return hash(*(unsigned int*)(&AS_FLOAT(lit)));

		case LITERAL_STRING:
			return hashString(AS_STRING(lit), strlen(AS_STRING(lit)));

		case LITERAL_ARRAY: {
			unsigned int res = 0;
			for (int i = 0; i < AS_ARRAY(lit)->count; i++) {
				res += hashLiteral(AS_ARRAY(lit)->literals[i]);
			}
			return hash(res);
		}

		case LITERAL_DICTIONARY: {
			unsigned int res = 0;
			for (int i = 0; i < AS_DICTIONARY(lit)->capacity; i++) {
				if (!IS_NULL(AS_DICTIONARY(lit)->entries[i].key)) { //only hash non-null keys
					res += hashLiteral(AS_DICTIONARY(lit)->entries[i].key);
					res += hashLiteral(AS_DICTIONARY(lit)->entries[i].value);
				}
			}
			return hash(res);
		}

		case LITERAL_FUNCTION:
			return 0;

		case LITERAL_IDENTIFIER:
			return HASH_I(lit); //pre-computed

		// case LITERAL_TYPE:
		// 	//not needed

		// case LITERAL_ANY:
		// 	//not needed

		default:
			//should never bee seen
			fprintf(stderr, ERROR "[internal] Unrecognized literal type in hash: %d\n" RESET, lit.type);
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

		globalPrintCapacity = GROW_CAPACITY(globalPrintCapacity);
		globalPrintBuffer = GROW_ARRAY(char, globalPrintBuffer, oldCapacity, globalPrintCapacity);
	}

	snprintf(globalPrintBuffer + globalPrintCount, strlen(str) + 1, "%s", str);
	globalPrintCount += strlen(str);
}

//exposed functions
void printLiteral(Literal literal) {
	printLiteralCustom(literal, stdoutWrapper);
}

void printLiteralCustom(Literal literal, void (printFn)(const char*)) {
	switch(literal.type) {
		case LITERAL_NULL:
			printFn("null");
		break;

		case LITERAL_BOOLEAN:
			printFn(AS_BOOLEAN(literal) ? "true" : "false");
		break;

		case LITERAL_INTEGER: {
			char buffer[256];
			snprintf(buffer, 256, "%d", AS_INTEGER(literal));
			printFn(buffer);
		}
		break;

		case LITERAL_FLOAT: {
			char buffer[256];
			snprintf(buffer, 256, "%g", AS_FLOAT(literal));
			printFn(buffer);
		}
		break;

		case LITERAL_STRING: {
			char buffer[MAX_STRING_LENGTH];
			if (!quotes) {
				snprintf(buffer, MAX_STRING_LENGTH, "%.*s", (int)strlen(AS_STRING(literal)), AS_STRING(literal));
			}
			else {
				snprintf(buffer, MAX_STRING_LENGTH, "%c%.*s%c", quotes, (int)strlen(AS_STRING(literal)), AS_STRING(literal), quotes);
			}
			printFn(buffer);
		}
		break;

		case LITERAL_ARRAY: {
			LiteralArray* ptr = AS_ARRAY(literal);

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
				printLiteralCustom(ptr->literals[i], printToBuffer);

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
			FREE_ARRAY(char, printBuffer, printCapacity);
			quotes = 0;
		}
		break;

		case LITERAL_DICTIONARY: {
			LiteralDictionary* ptr = AS_DICTIONARY(literal);

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
				if (IS_NULL(ptr->entries[i].key)) {
					continue;
				}

				if (delimCount++ > 0) {
					printToBuffer(",");
				}

				quotes = '"';
				printLiteralCustom(ptr->entries[i].key, printToBuffer);
				printToBuffer(":");
				quotes = '"';
				printLiteralCustom(ptr->entries[i].value, printToBuffer);
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
			FREE_ARRAY(char, printBuffer, printCapacity);
			quotes = 0;
		}
		break;

		//TODO: functions
		case LITERAL_FUNCTION:
		case LITERAL_FUNCTION_NATIVE:
			printFn("(function)");
		break;

		case LITERAL_IDENTIFIER: {
			char buffer[256];
			snprintf(buffer, 256, "%.*s", (int)strlen( AS_IDENTIFIER(literal) ), AS_IDENTIFIER(literal));
			printFn(buffer);
		}
		break;

		case LITERAL_TYPE: {
			//hold potential parent-call buffers on the C stack
			char* cacheBuffer = globalPrintBuffer;
			globalPrintBuffer = NULL;
			int cacheCapacity = globalPrintCapacity;
			globalPrintCapacity = 0;
			int cacheCount = globalPrintCount;
			globalPrintCount = 0;

			//print the type correctly
			printToBuffer("<");

			switch(AS_TYPE(literal).typeOf) {
				case LITERAL_NULL:
					printToBuffer("null");
				break;

				case LITERAL_BOOLEAN:
					printToBuffer("bool");
				break;

				case LITERAL_INTEGER:
					printToBuffer("int");
				break;

				case LITERAL_FLOAT:
					printToBuffer("float");
				break;

				case LITERAL_STRING:
					printToBuffer("string");
				break;

				case LITERAL_ARRAY:
					//print all in the array
					printToBuffer("[");
					for (int i = 0; i < AS_TYPE(literal).count; i++) {
						printLiteralCustom(((Literal*)(AS_TYPE(literal).subtypes))[i], printToBuffer);
					}
					printToBuffer("]");
				break;

				case LITERAL_DICTIONARY:
					printToBuffer("[");

					for (int i = 0; i < AS_TYPE(literal).count; i += 2) {
						printLiteralCustom(((Literal*)(AS_TYPE(literal).subtypes))[i], printToBuffer);
						printToBuffer(":");
						printLiteralCustom(((Literal*)(AS_TYPE(literal).subtypes))[i + 1], printToBuffer);
					}
					printToBuffer("]");
				break;

				case LITERAL_FUNCTION:
					printToBuffer("function");
					//TODO: how to print a function
				break;

				case LITERAL_IDENTIFIER:
					printToBuffer("identifier");
				break;

				case LITERAL_TYPE:
					printToBuffer("type");
				break;

				case LITERAL_ANY:
					printToBuffer("any");
				break;

				default:
					//should never be seen
					fprintf(stderr, ERROR "[internal] Unrecognized literal type in print type: %d\n" RESET, AS_TYPE(literal).typeOf);
			}

			//const (printed last)
			if (AS_TYPE(literal).constant) {
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
			FREE_ARRAY(char, printBuffer, printCapacity);
			quotes = 0;
		}
		break;

		case LITERAL_TYPE_INTERMEDIATE:
		case LITERAL_FUNCTION_INTERMEDIATE:
			printFn("Unprintable literal found");
		break;

		case LITERAL_ANY:
			printFn("(any)");
		break;

		default:
			//should never be seen
			fprintf(stderr, ERROR "[internal] Unrecognized literal type in print: %d\n" RESET, literal.type);
	}
}
