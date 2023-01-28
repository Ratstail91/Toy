#include "toy_refstring.h"

#include <string.h>
#include <assert.h>

//test variable sizes based on platform (safety)
#define STATIC_ASSERT(test_for_true) static_assert((test_for_true), "(" #test_for_true ") failed")

STATIC_ASSERT(sizeof(Toy_RefString) == 12);
STATIC_ASSERT(sizeof(int) == 4);
STATIC_ASSERT(sizeof(char) == 1);

//memory allocation
extern void* Toy_private_defaultMemoryAllocator(void* pointer, size_t oldSize, size_t newSize);
static Toy_RefStringAllocatorFn allocate = Toy_private_defaultMemoryAllocator;

void Toy_setRefStringAllocatorFn(Toy_RefStringAllocatorFn allocator) {
	allocate = allocator;
}

//API
Toy_RefString* Toy_createRefString(char* cstring) {
	int length = strlen(cstring);

	return Toy_createRefStringLength(cstring, length);
}

Toy_RefString* Toy_createRefStringLength(char* cstring, int length) {
	//allocate the memory area (including metadata space)
	Toy_RefString* refString = (Toy_RefString*)allocate(NULL, 0, sizeof(int) * 2 + sizeof(char) * length + 1);

	//set the data
	refString->refcount = 1;
	refString->length = length;
	strncpy(refString->data, cstring, refString->length);

	refString->data[refString->length] = '\0'; //string terminator

	return refString;
}

void Toy_deleteRefString(Toy_RefString* refString) {
	//decrement, then check
	refString->refcount--;
	if (refString->refcount <= 0) {
		allocate(refString, sizeof(int) * 2 + sizeof(char) * refString->length + 1, 0);
	}
}

int Toy_countRefString(Toy_RefString* refString) {
	return refString->refcount;
}

int Toy_lengthRefString(Toy_RefString* refString) {
	return refString->length;
}

Toy_RefString* Toy_copyRefString(Toy_RefString* refString) {
	//Cheaty McCheater Face
	refString->refcount++;
	return refString;
}

Toy_RefString* Toy_deepCopyRefString(Toy_RefString* refString) {
	//create a new string, with a new refcount
	return Toy_createRefStringLength(refString->data, refString->length);
}

char* Toy_toCString(Toy_RefString* refString) {
	return refString->data;
}

bool Toy_equalsRefString(Toy_RefString* lhs, Toy_RefString* rhs) {
	//same pointer
	if (lhs == rhs) {
		return true;
	}

	//different length
	if (lhs->length != rhs->length) {
		return false;
	}

	//same string
	return strncmp(lhs->data, rhs->data, lhs->length) == 0;
}

bool Toy_equalsRefStringCString(Toy_RefString* lhs, char* cstring) {
	//get the rhs length
	int length = strlen(cstring);

	//different length
	if (lhs->length != length) {
		return false;
	}

	//same string
	return strncmp(lhs->data, cstring, lhs->length) == 0;
}