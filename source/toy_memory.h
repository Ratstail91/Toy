#pragma once

#include "toy_common.h"

#define TOY_GROW_CAPACITY(capacity) \
	((capacity) < 8 ? 8 : (capacity) * 2)

#define TOY_GROW_ARRAY(type, pointer, oldSize, newSize) \
	(type*)Toy_reallocate(pointer, sizeof(type)*oldSize, sizeof(type)*newSize)

#define TOY_FREE_ARRAY(type, pointer, oldSize) \
	(type*)Toy_reallocate(pointer, sizeof(type)*oldSize, 0)

TOY_API void* Toy_reallocate(void* pointer, size_t oldSize, size_t newSize);
