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

void freeLiteralArray(LiteralArray* array) {
	//clean up memory
	for(int i = 0; i < array->count; i++) {
		freeLiteral(array->literals[i]);
	}

	FREE_ARRAY(Literal, array->literals, array->capacity);
	initLiteralArray(array);
}

int pushLiteralArray(LiteralArray* array, Literal literal) {
	if (array->capacity < array->count + 1) {
		int oldCapacity = array->capacity;

		array->capacity = GROW_CAPACITY(oldCapacity);
		array->literals = GROW_ARRAY(Literal, array->literals, oldCapacity, array->capacity);
	}

	array->literals[array->count] = copyLiteral(literal);
	return array->count++;
}

Literal popLiteralArray(LiteralArray* array) {
	if (array->count <= 0) {
		return TO_NULL_LITERAL;
	}

	//get the return
	Literal ret = array->literals[array->count-1];

	//null the existing data
	array->literals[array->count-1] = TO_NULL_LITERAL;

	array->count--;
	return ret;
}

//find a literal in the array that matches the "literal" argument
int findLiteralIndex(LiteralArray* array, Literal literal) {
	for (int i = 0; i < array->count; i++) {
		//not the same type
		if (array->literals[i].type != literal.type) {
			continue;
		}

		//types match?
		if (literalsAreEqual(array->literals[i], literal)) {
			return i;
		}
	}

	return -1;
}

bool setLiteralArray(LiteralArray* array, Literal index, Literal value) {
	if (!IS_INTEGER(index)) {
		return false;
	}

	int idx = AS_INTEGER(index);

	if (idx < 0 || idx >= array->count) {
		return false;
	}

	freeLiteral(array->literals[idx]);
	array->literals[idx] = copyLiteral(value);

	return true;
}

Literal getLiteralArray(LiteralArray* array, Literal index) {
	if (!IS_INTEGER(index)) {
		return TO_NULL_LITERAL;
	}

	int idx = AS_INTEGER(index);

	if (idx < 0 || idx >= array->count) {
		return TO_NULL_LITERAL;
	}

	return copyLiteral(array->literals[idx]);
}