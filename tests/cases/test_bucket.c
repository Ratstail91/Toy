#include "toy_bucket.h"
#include "toy_console_colors.h"

#include <stdio.h>

int test_buckets() {
	//test initializing and freeing a bucket
	{
		//init
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(int) * 32);

		//check
		if (bucket == NULL || bucket->capacity != 32 * sizeof(int)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to initialize 'Toy_Bucket'\n" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//test partitioning a bucket, several times
	{
		//init
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(int) * 32);

		//grab some memory
		int* a = Toy_partitionBucket(&bucket, sizeof(int));
		int* b = Toy_partitionBucket(&bucket, sizeof(int));
		int* c = Toy_partitionBucket(&bucket, sizeof(int));
		int* d = Toy_partitionBucket(&bucket, sizeof(int));

		//check
		if (bucket == NULL || bucket->count != 4 * sizeof(int)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to partition 'Toy_Bucket' correctly: count is %d, expected %d\n" TOY_CC_RESET, (int)(bucket->count), (int)(4*sizeof(int)));
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//test partitioning a bucket, several times, with an internal expansion
	{
		//init
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(int) * 4);

		//grab some memory
		int* a = Toy_partitionBucket(&bucket, sizeof(int));
		int* b = Toy_partitionBucket(&bucket, sizeof(int));
		int* c = Toy_partitionBucket(&bucket, sizeof(int));
		int* d = Toy_partitionBucket(&bucket, sizeof(int));
		int* e = Toy_partitionBucket(&bucket, sizeof(int));
		int* f = Toy_partitionBucket(&bucket, sizeof(int));

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
		Toy_freeBucket(&bucket);
	}

	//test partitioning a bucket, several times, with an internal expansion, and awkward sizes
	{
		//init
		Toy_Bucket* bucket = Toy_allocateBucket(32);

		//grab some memory
		void* a = Toy_partitionBucket(&bucket, 16);
		void* b = Toy_partitionBucket(&bucket, 10);
		void* c = Toy_partitionBucket(&bucket, 10);
		void* d = Toy_partitionBucket(&bucket, 10);

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
		Toy_freeBucket(&bucket);
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_buckets();
		total += res;

		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
	}

	return total;
}
