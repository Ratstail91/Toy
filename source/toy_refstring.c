#include "toy_refstring.h"

#include <string.h>

//memory allocation
extern void* Toy_private_defaultMemoryAllocator(void* pointer, size_t oldSize, size_t newSize);
static Toy_RefStringAllocatorFn allocate = Toy_private_defaultMemoryAllocator;

void Toy_setRefStringAllocatorFn(Toy_RefStringAllocatorFn allocator) {
	allocate = allocator;
}

//API
Toy_RefString* Toy_createRefString(const char* cstring) {
	size_t length = strlen(cstring);

	return Toy_createRefStringLength(cstring, length);
}

Toy_RefString* Toy_createRefStringLength(const char* cstring, size_t length) {
	//allocate the memory area (including metadata space)
	Toy_RefString* refString = allocate(NULL, 0, sizeof(size_t) + sizeof(int) + sizeof(char) * (length + 1));

	if (refString == NULL) {
		return NULL;
	}

	//set the data
	refString->refCount = 1;
	refString->length = length;
	strncpy(refString->data, cstring, refString->length);

	refString->data[refString->length] = '\0'; //string terminator

	return refString;
}

void Toy_deleteRefString(Toy_RefString* refString) {
	//decrement, then check
	refString->refCount--;
	if (refString->refCount <= 0) {
		allocate(refString, sizeof(size_t) + sizeof(int) + sizeof(char) * (refString->length + 1), 0);
	}
}

int Toy_countRefString(Toy_RefString* refString) {
	return refString->refCount;
}

size_t Toy_lengthRefString(Toy_RefString* refString) {
	return refString->length;
}

Toy_RefString* Toy_copyRefString(Toy_RefString* refString) {
	//Cheaty McCheater Face
	refString->refCount++;
	return refString;
}

Toy_RefString* Toy_deepCopyRefString(Toy_RefString* refString) {
	//create a new string, with a new refCount
	return Toy_createRefStringLength(refString->data, refString->length);
}

const char* Toy_toCString(Toy_RefString* refString) {
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
	size_t length = strlen(cstring);

	//different length
	if (lhs->length != length) {
		return false;
	}

	//same string
	return strncmp(lhs->data, cstring, lhs->length) == 0;
}
