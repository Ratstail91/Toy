#include "literal_array.h"

#include "memory.h"

#include <stdio.h>
#include <string.h>

//exposed functions
void initLiteralArray(LiteralArray* array) {
	array->capacity = 0;
	array->count = 0;
	array->literals = NULL;
}

int writeLiteralArray(LiteralArray* array, Literal literal) {
	if (array->capacity < array->count + 1) {
		int oldCapacity = array->capacity;

		array->capacity = GROW_CAPACITY(oldCapacity);
		array->literals = GROW_ARRAY(Literal, array->literals, oldCapacity, array->capacity);
	}

	//if it's a string, make a local copy
	if (IS_STRING(literal)) {
		literal = TO_STRING_LITERAL(copyString(AS_STRING(literal), STRLEN(literal)));
	}

	array->literals[array->count] = literal;
	return array->count++;
}

void freeLiteralArray(LiteralArray* array) {
	//clean up memory
	for(int i = 0; i < array->count; i++) {
		freeLiteral(array->literals[i]);
	}

	FREE_ARRAY(Literal, array->literals, array->capacity);
	initLiteralArray(array);
}

//find a literal in the array that matches the "literal" argument
int findLiteralIndex(LiteralArray* array, Literal literal) {
	for (int i = 0; i < array->count; i++) {
		//not the same type
		if (array->literals[i].type != literal.type) {
			continue;
		}

		//matching type, compare values
		switch(array->literals[i].type) {
			case LITERAL_NULL:
				return i;

			case LITERAL_BOOLEAN:
				if (AS_BOOLEAN(array->literals[i]) == AS_BOOLEAN(literal)) {
					return i;
				}
			continue;

			case LITERAL_INTEGER:
				if (AS_INTEGER(array->literals[i]) == AS_INTEGER(literal)) {
					return i;
				}
			continue;

			case LITERAL_FLOAT:
				if (AS_FLOAT(array->literals[i]) == AS_FLOAT(literal)) {
					return i;
				}
			continue;

			case LITERAL_STRING:
				if (strncmp(AS_STRING(array->literals[i]), AS_STRING(literal), STRLEN(literal)) == 0) {
					return i;
				}

			continue;

			default:
				fprintf(stderr, "[Internal] Unexpected literal type in findLiteralIndex(): %d\n", literal.type);
		}
	}

	return -1;
}
