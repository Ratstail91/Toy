#include "literal.h"
#include "memory.h"

#include "literal_array.h"
#include "literal_dictionary.h"

#include <stdio.h>
#include <string.h>

//utils
static void stdoutWrapper(const char* output) {
	fprintf(stdout, output);
}

//buffer the prints
static char* globalPrintBuffer = NULL;
static size_t globalPrintCapacity = 0;
static size_t globalPrintCount = 0;

static void printToBuffer(const char* str) {
	while (strlen(str) + globalPrintCount > globalPrintCapacity) {
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
			char buffer[4096];
			snprintf(buffer, 4096, "\"%.*s\"", STRLEN(literal), AS_STRING(literal));
			printFn(buffer);
		}
		break;

		case LITERAL_ARRAY: {
			LiteralArray* ptr = AS_ARRAY(literal);

			//hold potential parent-call buffers
			char* cacheBuffer = globalPrintBuffer;
			globalPrintBuffer = NULL;
			int cacheCapacity = globalPrintCapacity;
			globalPrintCapacity = 0;
			int cacheCount = globalPrintCount;
			globalPrintCount = 0;

			//print the contents to the global buffer
			printToBuffer("[");
			for (int i = 0; i < ptr->count; i++) {
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
		}
		break;

		case LITERAL_DICTIONARY: {
			LiteralDictionary* ptr = AS_DICTIONARY(literal);

			//hold potential parent-call buffers
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
				if (ptr->entries[i].key.type == LITERAL_NULL) {
					continue;
				}

				if (delimCount++ > 0) {
					printToBuffer(",");
				}

				printLiteralCustom(ptr->entries[i].key, printToBuffer);
				printToBuffer(":");
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
		}
		break;

		case LITERAL_IDENTIFIER: {
			char buffer[256];
			snprintf(buffer, 256, "%.*s", STRLEN_I(literal), AS_IDENTIFIER(literal));
			printFn(buffer);
		}
		break;

		default:
			//should never bee seen
			fprintf(stderr, "[Internal] Unrecognized literal type in print: %d\n", literal.type);
	}
}

void freeLiteral(Literal literal) {
	if (IS_STRING(literal)) {
		FREE(char, AS_STRING(literal));
		return;
	}
}

bool _isTruthy(Literal x) {
	return (IS_NULL(x) || (IS_BOOLEAN(x) && AS_BOOLEAN(x)) || (IS_INTEGER(x) && AS_INTEGER(x) != 0) || (IS_FLOAT(x) && AS_FLOAT(x) != 0));
}

Literal _toStringLiteral(char* str) {
	return ((Literal){LITERAL_STRING, {.string.ptr = (char*)str, .string.length = strlen((char*)str)}});
}

Literal _toIdentifierLiteral(char* str) {
	return ((Literal){LITERAL_IDENTIFIER,{.identifier.ptr = (char*)str,.identifier.length = strlen((char*)str)}});
}

char* copyString(char* original, int length) {
	char* buffer = ALLOCATE(char, length + 1);
	strncpy(buffer, original, length);
	buffer[length] = '\0';
	return buffer;
}

bool literalsAreEqual(Literal lhs, Literal rhs) {
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
		case LITERAL_BOOLEAN:
			return AS_BOOLEAN(lhs) == AS_BOOLEAN(rhs);

		case LITERAL_INTEGER:
			return AS_INTEGER(lhs) == AS_INTEGER(rhs);

		case LITERAL_FLOAT:
			return AS_FLOAT(lhs) == AS_FLOAT(rhs);

		case LITERAL_STRING:
			if (STRLEN(lhs) != STRLEN(rhs)) {
				return false;
			}
			return !strncmp(AS_STRING(lhs), AS_STRING(rhs), STRLEN(lhs));

		//TODO: literal array and literal dictionary equality checks

		case LITERAL_IDENTIFIER:
			if (STRLEN_I(lhs) != STRLEN_I(rhs)) {
				return false;
			}
			return !strncmp(AS_IDENTIFIER(lhs), AS_IDENTIFIER(rhs), STRLEN_I(lhs));

		default:
			//should never bee seen
			fprintf(stderr, "[Internal] Unrecognized literal type in equality: %d\n", lhs.type);
			return false;
	}
}

//hash functions
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
			return hashString(AS_STRING(lit), STRLEN(lit));

		case LITERAL_ARRAY: {
			unsigned int res = 0;
			for (int i = 0; i < AS_DICTIONARY(lit)->count; i++) {
				res += hashLiteral(AS_ARRAY(lit)->literals[i]);
			}
			return hash(res);
		}

		case LITERAL_DICTIONARY: {
			unsigned int res = 0;
			for (int i = 0; i < AS_DICTIONARY(lit)->count; i++) {
				if (!IS_NULL(AS_DICTIONARY(lit)->entries[i].key)) { //only hash non-null keys
					res += hashLiteral(AS_DICTIONARY(lit)->entries[i].key);
					res += hashLiteral(AS_DICTIONARY(lit)->entries[i].value);
				}
			}
			return hash(res);
		}

		case LITERAL_IDENTIFIER:
			return hashString(AS_IDENTIFIER(lit), STRLEN_I(lit));

		default:
			//should never bee seen
			fprintf(stderr, "[Internal] Unrecognized literal type in hash: %d\n", lit.type);
			return 0;
	}
}
