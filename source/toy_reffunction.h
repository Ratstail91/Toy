#pragma once

#include "toy_common.h"

//memory allocation hook
typedef void* (*Toy_RefFunctionAllocatorFn)(void* pointer, size_t oldSize, size_t newSize);
TOY_API void Toy_setRefFunctionAllocatorFn(Toy_RefFunctionAllocatorFn);

//the RefFunction structure
typedef struct Toy_RefFunction {
	size_t length;
	int refCount;
	unsigned char data[];
} Toy_RefFunction;

//API
TOY_API Toy_RefFunction* Toy_createRefFunction(const void* data, size_t length);
TOY_API void Toy_deleteRefFunction(Toy_RefFunction* refFunction);
TOY_API int Toy_countRefFunction(Toy_RefFunction* refFunction);
TOY_API size_t Toy_lengthRefFunction(Toy_RefFunction* refFunction);
TOY_API Toy_RefFunction* Toy_copyRefFunction(Toy_RefFunction* refFunction);
TOY_API Toy_RefFunction* Toy_deepCopyRefFunction(Toy_RefFunction* refFunction);

