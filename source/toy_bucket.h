#pragma once

#include "toy_common.h"

//NOTE: this structure has restrictions on it's usage:
// - It can only expand until it is freed
// - It cannot be copied around within RAM
// - It cannot allocate more memory than it has capacity
// If each of these rules are followed, the bucket is actually more efficient than any other option

//a custom allocator
typedef struct Toy_Bucket {  //32 | 64 BITNESS
	struct Toy_Bucket* next; //4  | 8
	unsigned int capacity;   //4  | 4
	unsigned int count;      //4  | 4
	char data[];             //-  | -
} Toy_Bucket;                //12 | 16

TOY_API Toy_Bucket* Toy_allocateBucket(unsigned int capacity);
TOY_API void* Toy_partitionBucket(Toy_Bucket** bucketHandle, unsigned int amount);
TOY_API void Toy_freeBucket(Toy_Bucket** bucketHandle);

//some useful bucket sizes, could be swapped or ifdef'd as needed
#define TOY_BUCKET_TINY (1024 * 2)
#define TOY_BUCKET_SMALL (1024 * 4)
#define TOY_BUCKET_MEDIUM (1024 * 8)
#define TOY_BUCKET_LARGE (1024 * 16)
#define TOY_BUCKET_HUGE (1024 * 32)

//sizeof(Toy_Bucket) is added internally, so reverse it here
#define TOY_BUCKET_IDEAL (TOY_BUCKET_HUGE - sizeof(Toy_Bucket))

