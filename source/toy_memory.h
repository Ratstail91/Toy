#pragma once

#include "toy_common.h"

//standard movable array for general use
#define TOY_GROW_CAPACITY(capacity) \
	((capacity) < 8 ? 8 : (capacity) * 2)

#define TOY_ALLOCATE(type, count) \
	(type*)Toy_reallocate(NULL, 0, sizeof(type)*(count))

#define TOY_FREE(type, pointer) \
	(type*)Toy_reallocate(pointer, sizeof(type), 0)

#define TOY_GROW_ARRAY(type, pointer, oldSize, newSize) \
	(type*)Toy_reallocate(pointer, sizeof(type)*oldSize, sizeof(type)*newSize)

#define TOY_SHRINK_ARRAY(type, pointer, oldCount, count) \
	(type*)Toy_reallocate((type*)pointer, sizeof(type)*(oldCount), sizeof(type)*(count))

#define TOY_FREE_ARRAY(type, pointer, oldSize) \
	(type*)Toy_reallocate(pointer, sizeof(type)*oldSize, 0)

TOY_API void* Toy_reallocate(void* pointer, size_t oldSize, size_t newSize);

//immobile "bucket" memory structure for custom allocators
#define TOY_BUCKET_INIT(type, bucket, capacity) \
	Toy_initBucket(&(bucket), sizeof(type)*(capacity))

#define TOY_BUCKET_PART(type, bucket) \
	(type*)Toy_partBucket(&(bucket), sizeof(type))

#define TOY_BUCKET_FREE(bucket) \
	Toy_freeBucket(&(bucket))

typedef struct Toy_Bucket {
	struct Toy_Bucket* next;
	void* contents;
	int capacity;
	int count;
} Toy_Bucket;

TOY_API void Toy_initBucket(Toy_Bucket** bucketHandle, size_t capacity);
TOY_API void* Toy_partBucket(Toy_Bucket** bucketHandle, size_t space);
TOY_API void Toy_freeBucket(Toy_Bucket** bucketHandle);
