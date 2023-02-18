#pragma once

#include <stdbool.h>
#include <stddef.h>

#include "toy_common.h"

//memory allocation hook
typedef void* (*Toy_RefStringAllocatorFn)(void* pointer, size_t oldSize, size_t newSize);
TOY_API void Toy_setRefStringAllocatorFn(Toy_RefStringAllocatorFn);

//the RefString structure
typedef struct Toy_RefString {
	size_t length;
	int refCount;
	char data[];
} Toy_RefString;

//API
TOY_API Toy_RefString* Toy_createRefString(const char* cstring);
TOY_API Toy_RefString* Toy_createRefStringLength(const char* cstring, size_t length);
TOY_API void Toy_deleteRefString(Toy_RefString* refString);
TOY_API int Toy_countRefString(Toy_RefString* refString);
TOY_API size_t Toy_lengthRefString(Toy_RefString* refString);
TOY_API Toy_RefString* Toy_copyRefString(Toy_RefString* refString);
TOY_API Toy_RefString* Toy_deepCopyRefString(Toy_RefString* refString);
TOY_API const char* Toy_toCString(Toy_RefString* refString);
TOY_API bool Toy_equalsRefString(Toy_RefString* lhs, Toy_RefString* rhs);
TOY_API bool Toy_equalsRefStringCString(Toy_RefString* lhs, char* cstring);

//TODO: merge refstring memory

