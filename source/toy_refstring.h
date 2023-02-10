#pragma once

#include <stdbool.h>
#include <stddef.h>

//memory allocation hook
typedef void* (*Toy_RefStringAllocatorFn)(void* pointer, size_t oldSize, size_t newSize);
void Toy_setRefStringAllocatorFn(Toy_RefStringAllocatorFn);

//the RefString structure
typedef struct Toy_RefString {
	size_t length;
	int refCount;
	char data[];
} Toy_RefString;

//API
Toy_RefString* Toy_createRefString(const char* cstring);
Toy_RefString* Toy_createRefStringLength(const char* cstring, size_t length);
void Toy_deleteRefString(Toy_RefString* refString);
int Toy_countRefString(Toy_RefString* refString);
size_t Toy_lengthRefString(Toy_RefString* refString);
Toy_RefString* Toy_copyRefString(Toy_RefString* refString);
Toy_RefString* Toy_deepCopyRefString(Toy_RefString* refString);
const char* Toy_toCString(Toy_RefString* refString);
bool Toy_equalsRefString(Toy_RefString* lhs, Toy_RefString* rhs);
bool Toy_equalsRefStringCString(Toy_RefString* lhs, char* cstring);
