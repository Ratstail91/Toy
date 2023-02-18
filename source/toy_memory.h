#pragma once

#include "toy_common.h"

#define TOY_GROW_CAPACITY(capacity) 						((capacity) < 8 ? 8 : (capacity) * 2)
#define TOY_GROW_CAPACITY_FAST(capacity)					((capacity) < 32 ? 32 : (capacity) * 2)

#define TOY_ALLOCATE(type, count)							((type*)Toy_reallocate(NULL, 0, sizeof(type) * (count)))

#define TOY_FREE(type, pointer)								Toy_reallocate(pointer, sizeof(type), 0)
#define TOY_FREE_ARRAY(type, pointer, oldCount)				Toy_reallocate((type*)pointer, sizeof(type) * (oldCount), 0)

#define TOY_GROW_ARRAY(type, pointer, oldCount, count)		(type*)Toy_reallocate((type*)pointer, sizeof(type) * (oldCount), sizeof(type) * (count))
#define TOY_SHRINK_ARRAY(type, pointer, oldCount, count)	(type*)Toy_reallocate((type*)pointer, sizeof(type) * (oldCount), sizeof(type) * (count))

//implementation details
TOY_API void* Toy_reallocate(void* pointer, size_t oldSize, size_t newSize);

//assign the memory allocator
typedef void* (*Toy_MemoryAllocatorFn)(void* pointer, size_t oldSize, size_t newSize);
TOY_API void Toy_setMemoryAllocator(Toy_MemoryAllocatorFn);
