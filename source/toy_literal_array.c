#include "toy_literal_array.h"

#include "toy_memory.h"

#include <stdio.h>
#include <string.h>

//exposed functions
void Toy_initLiteralArray(Toy_LiteralArray* array) {
	array->capacity = 0;
	array->count = 0;
	array->literals = NULL;
}

void Toy_freeLiteralArray(Toy_LiteralArray* array) {
	//clean up memory
	for(int i = 0; i < array->count; i++) {
		Toy_freeLiteral(array->literals[i]);
	}

	if (array->capacity > 0) {
		TOY_FREE_ARRAY(Toy_Literal, array->literals, array->capacity);
		Toy_initLiteralArray(array);
	}
}

int Toy_pushLiteralArray(Toy_LiteralArray* array, Toy_Literal literal) {
	if (array->capacity < array->count + 1) {
		int oldCapacity = array->capacity;

		array->capacity = TOY_GROW_CAPACITY(oldCapacity);
		array->literals = TOY_GROW_ARRAY(Toy_Literal, array->literals, oldCapacity, array->capacity);
	}

	array->literals[array->count] = Toy_copyLiteral(literal);
	return array->count++;
}

Toy_Literal Toy_popLiteralArray(Toy_LiteralArray* array) {
	if (array->count <= 0) {
		return TOY_TO_NULL_LITERAL;
	}

	//get the return
	Toy_Literal ret = array->literals[array->count-1];

	//null the existing data
	array->literals[array->count-1] = TOY_TO_NULL_LITERAL;

	array->count--;
	return ret;
}

//find a literal in the array that matches the "literal" argument
int Toy_findLiteralIndex(Toy_LiteralArray* array, Toy_Literal literal) {
	for (int i = 0; i < array->count; i++) {
		//not the same type
		if (array->literals[i].type != literal.type) {
			continue;
		}

		//types match?
		if (Toy_literalsAreEqual(array->literals[i], literal)) {
			return i;
		}
	}

	return -1;
}

bool Toy_setLiteralArray(Toy_LiteralArray* array, Toy_Literal index, Toy_Literal value) {
	if (!TOY_IS_INTEGER(index)) {
		return false;
	}

	int idx = TOY_AS_INTEGER(index);

	if (idx < 0 || idx >= array->count) {
		return false;
	}

	Toy_freeLiteral(array->literals[idx]);
	array->literals[idx] = Toy_copyLiteral(value);

	return true;
}

Toy_Literal Toy_getLiteralArray(Toy_LiteralArray* array, Toy_Literal index) {
	if (!TOY_IS_INTEGER(index)) {
		return TOY_TO_NULL_LITERAL;
	}

	int idx = TOY_AS_INTEGER(index);

	if (idx < 0 || idx >= array->count) {
		return TOY_TO_NULL_LITERAL;
	}

	return Toy_copyLiteral(array->literals[idx]);
}
