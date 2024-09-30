#include "toy_memory.h"
#include "toy_console_colors.h"

#include <stdio.h>

int test_reallocate() {
	//test single pointer
	{
		int* integer = TOY_ALLOCATE(int, 1);
		TOY_FREE(int, integer);
	}

	//test single pointer array
	{
		int* array = TOY_ALLOCATE(int, 10);

		//check you can access the memory
		array[1] = 42;

		TOY_FREE_ARRAY(int, array, 10);
	}

	//test multiple pointer arrays
	{
		int* array1 = TOY_ALLOCATE(int, 10);
		int* array2 = TOY_ALLOCATE(int, 10);

		array1[1] = 42; //access the given memory
		array2[1] = 42; //access the given memory

		TOY_FREE_ARRAY(int, array1, 10);
		TOY_FREE_ARRAY(int, array2, 10);
	}

	return 0;
}

int test_buckets() {
	//test initializing and freeing a bucket
	{
		//init
		Toy_Bucket* bucket = NULL;
		TOY_BUCKET_INIT(int, bucket, 32);

		//check
		if (bucket == NULL || bucket->capacity != 32 * sizeof(int)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to initialize 'Toy_Bucket'\n" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		TOY_BUCKET_FREE(bucket);
	}

	//test partitioning a bucket, several times
	{
		//init
		Toy_Bucket* bucket = NULL;
		TOY_BUCKET_INIT(int, bucket, 32);

		//grab some memory
		int* a = TOY_BUCKET_PART(int, bucket);
		int* b = TOY_BUCKET_PART(int, bucket);
		int* c = TOY_BUCKET_PART(int, bucket);
		int* d = TOY_BUCKET_PART(int, bucket);

		//check
		if (bucket == NULL || bucket->count != 4 * sizeof(int)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to partition 'Toy_Bucket' correctly: count is %d, expected %d\n" TOY_CC_RESET, (int)(bucket->count), (int)(4*sizeof(int)));
			return -1;
		}

		//cleanup
		TOY_BUCKET_FREE(bucket);
	}

	//test partitioning a bucket, several times, with an internal expansion
	{
		//init
		Toy_Bucket* bucket = NULL;
		TOY_BUCKET_INIT(int, bucket, 4);

		//grab some memory
		int* a = TOY_BUCKET_PART(int, bucket);
		int* b = TOY_BUCKET_PART(int, bucket);
		int* c = TOY_BUCKET_PART(int, bucket);
		int* d = TOY_BUCKET_PART(int, bucket);
		int* e = TOY_BUCKET_PART(int, bucket);
		int* f = TOY_BUCKET_PART(int, bucket);

		//checks - please note that the top-most bucket is what is being filled - older buckets are further along
		if (
			bucket->capacity != 4 * sizeof(int) ||
			bucket->count != 2 * sizeof(int) ||
			bucket->next == NULL ||
			bucket->next->capacity != 4 * sizeof(int) ||
			bucket->next->count != 4 * sizeof(int))
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to expand 'Toy_Bucket' correctly\n" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		TOY_BUCKET_FREE(bucket);
	}

	//test partitioning a bucket, several times, with an internal expansion, and awkward sizes
	{
		//init
		Toy_Bucket* bucket = NULL;
		Toy_initBucket(&bucket, 32);

		//grab some memory
		void* a = Toy_partBucket(&bucket, 16);
		void* b = Toy_partBucket(&bucket, 10);
		void* c = Toy_partBucket(&bucket, 10);
		void* d = Toy_partBucket(&bucket, 10);

		//checks - awkward and mismatched sizes is not officially supported, but it should work
		if (
			bucket->capacity != 32 ||
			bucket->count != 20 ||
			bucket->next == NULL ||
			bucket->next->capacity != 32 ||
			bucket->next->count != 26)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to expand 'Toy_Bucket' with awkward/mismatched sizes correctly\n" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		TOY_BUCKET_FREE(bucket);
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	res = test_reallocate();
	total += res;

	if (res == 0) {
		printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	}

	res = test_buckets();
	total += res;

	if (res == 0) {
		printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	}

	return total;
}
