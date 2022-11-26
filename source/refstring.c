#include "refstring.h"

#include <string.h>
#include <assert.h>

//test variable sizes based on platform (safety)
#define STATIC_ASSERT(test_for_true) static_assert((test_for_true), "(" #test_for_true ") failed")

STATIC_ASSERT(sizeof(RefString) == 12);
STATIC_ASSERT(sizeof(int) == 4);
STATIC_ASSERT(sizeof(char) == 1);

//memory allocation
static RefStringAllocatorFn allocate;

void setRefStringAllocatorFn(RefStringAllocatorFn allocator) {
	allocate = allocator;
}

//API
RefString* createRefString(char* cstring) {
	int length = strlen(cstring);

	return createRefStringLength(cstring, length);
}

RefString* createRefStringLength(char* cstring, int length) {
	//allocate the memory area (including metadata space)
	RefString* refString = (RefString*)allocate(NULL, 0, sizeof(int) * 2 + sizeof(char) * length + 1);

	//set the data
	refString->refcount = 1;
	refString->length = length;
	strncpy(refString->data, cstring, refString->length);

	refString->data[refString->length] = '\0'; //string terminator

	return refString;
}

void deleteRefString(RefString* refString) {
	if (refString->refcount > 0) {
		//decrement, then check
		refString->refcount--;
		if (refString->refcount <= 0) {
			allocate(refString, sizeof(int) * 2 + sizeof(char) * refString->length + 1, 0);
		}
	}
}

int countRefString(RefString* refString) {
	return refString->refcount;
}

int lengthRefString(RefString* refString) {
	return refString->length;
}

RefString* copyRefString(RefString* refString) {
	//Cheaty McCheater Face
	refString->refcount++;
	return refString;
}

RefString* deepCopyRefString(RefString* refString) {
	//create a new string, with a new refcount
	return createRefStringLength(refString->data, refString->length);
}

char* toCString(RefString* refString) {
	return refString->data;
}

bool equalsRefString(RefString* lhs, RefString* rhs) {
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

bool equalsRefStringCString(RefString* lhs, char* cstring) {
	//get the rhs length
	int length = strlen(cstring);

	//different length
	if (lhs->length != length) {
		return false;
	}

	//same string
	return strncmp(lhs->data, cstring, lhs->length) == 0;
}