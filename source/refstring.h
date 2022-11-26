#pragma once

#include <stdbool.h>
#include <stddef.h>

//memory allocation hook
typedef void* (*RefStringAllocatorFn)(void* pointer, size_t oldSize, size_t newSize);
void setRefStringAllocatorFn(RefStringAllocatorFn);

//the RefString structure
typedef struct RefString {
	int refcount;
	int length;
	char data[1];
} RefString;

//API
RefString* createRefString(char* cstring);
RefString* createRefStringLength(char* cstring, int length);
void deleteRefString(RefString* refString);
int countRefString(RefString* refString);
int lengthRefString(RefString* refString);
RefString* copyRefString(RefString* refString);
RefString* deepCopyRefString(RefString* refString);
char* toCString(RefString* refString);
bool equalsRefString(RefString* lhs, RefString* rhs);
bool equalsRefStringCString(RefString* lhs, char* cstring);
