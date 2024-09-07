#include "toy_memory.h"

#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>

void* Toy_reallocate(void* pointer, size_t oldSize, size_t newSize) {
	if (newSize == 0) {
		free(pointer);
		return NULL;
	}

	void* result = realloc(pointer, newSize);

	if (result == NULL) {
		fprintf(stderr, TOY_CC_ERROR "[internal] ERROR: Memory allocation error (requested %d, replacing %d)\n" TOY_CC_RESET, (int)newSize, (int)oldSize);
		exit(1);
	}

	return result;
}

//buckets of fun
void Toy_initBucket(Toy_Bucket** bucketHandle, size_t capacity) {
	if (capacity == 0) {
		fprintf(stderr, TOY_CC_ERROR "[internal] ERROR: Cannot init a bucket with zero capacity\n" TOY_CC_RESET);
		exit(1);
	}

	(*bucketHandle) = malloc(sizeof(Toy_Bucket));

	if ((*bucketHandle) == NULL) {
		fprintf(stderr, TOY_CC_ERROR "[internal] ERROR: Failed to allocate space for a bucket\n" TOY_CC_RESET);
		exit(1);
	}

	//initialize the bucket
	(*bucketHandle)->next = NULL;
	(*bucketHandle)->contents = NULL; //leave until the first partition
	(*bucketHandle)->capacity = capacity;
	(*bucketHandle)->count = 0;
}

void* Toy_partBucket(Toy_Bucket** bucketHandle, size_t space) {
	if ((*bucketHandle) == NULL) {
		fprintf(stderr, TOY_CC_ERROR "[internal] ERROR: Expected bucket, received NULL\n" TOY_CC_RESET);
		exit(1);
	}

	//if out of space in the current bucket
	if ((*bucketHandle)->capacity < (*bucketHandle)->count + space) {
		//move to the next bucket
		Toy_Bucket* tmp = NULL;
		Toy_initBucket(&tmp, (*bucketHandle)->capacity);
		tmp->next = (*bucketHandle);
		(*bucketHandle) = tmp;
	}

	//if no space allocated for the current bucket
	if ((*bucketHandle)->contents == NULL) {
		//allocate space for the current bucket
		(*bucketHandle)->contents = malloc((*bucketHandle)->capacity);

		//double check
		if ((*bucketHandle)->contents == NULL) {
			fprintf(stderr, TOY_CC_ERROR "[internal] ERROR: Failed to allocate space for bucket contents\n" TOY_CC_RESET);
			exit(1);
		}
	}

	//track the new count, and return the specified memory space
	(*bucketHandle)->count += space;
	return ((*bucketHandle)->contents + (*bucketHandle)->count - space);
}

void Toy_freeBucket(Toy_Bucket** bucketHandle) {
	while ((*bucketHandle) != NULL) {
		//run down the chain
		Toy_Bucket* ptr = (*bucketHandle);
		(*bucketHandle) = (*bucketHandle)->next;

		//clear the previous bucket from memory
		free(ptr->contents);
		free(ptr);
	}
}
