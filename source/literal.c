#include "literal.h"
#include "memory.h"

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
		FREE_ARRAY(char, AS_STRING(literal), literal.as.string.length);
		return;
	}

	if (IS_IDENTIFIER(literal)) {
		FREE_ARRAY(char, AS_IDENTIFIER(literal), literal.as.identifier.length);
		return;
	}

	if (IS_TYPE(literal)) {
		for (int i = 0; i < AS_TYPE(literal).count; i++) {
			freeLiteral(((Literal*)(AS_TYPE(literal).subtypes))[i]);
		}
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

