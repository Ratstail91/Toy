#include "toy_value.h"
#include "toy_console_colors.h"

#include "toy_bucket.h"
#include "toy_string.h"

#include <stdio.h>

int main() {
	//test for the correct size
	{
#if TOY_BITNESS == 64
		if (sizeof(Toy_Value) != 16) {
#else
		if (sizeof(Toy_Value) != 8) {
#endif
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'Toy_Value' is an unexpected size in memory\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test creating a null
	{
		Toy_Value v = TOY_VALUE_FROM_NULL();

		if (!TOY_VALUE_IS_NULL(v)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: creating a 'null' value failed\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test creating values
	{
		Toy_Value t = TOY_VALUE_FROM_BOOLEAN(true);
		Toy_Value f = TOY_VALUE_FROM_BOOLEAN(false);

		if (!TOY_VALUE_IS_TRUTHY(t) || TOY_VALUE_IS_TRUTHY(f)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'boolean' value failed\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test value equality
	{
		Toy_Value answer = TOY_VALUE_FROM_INTEGER(42);
		Toy_Value question = TOY_VALUE_FROM_INTEGER(42);
		Toy_Value nice = TOY_VALUE_FROM_INTEGER(69);

		if (!TOY_VALUES_ARE_EQUAL(answer, question)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: equality check failed, expected true\n" TOY_CC_RESET);
			return -1;
		}

		if (TOY_VALUES_ARE_EQUAL(answer, nice)) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: equality check failed, expected false\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test value hashing
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		//values
		Toy_Value n = TOY_VALUE_FROM_NULL();
		Toy_Value t = TOY_VALUE_FROM_BOOLEAN(true);
		Toy_Value f = TOY_VALUE_FROM_BOOLEAN(false);
		Toy_Value i = TOY_VALUE_FROM_INTEGER(42);
		//skip float
		Toy_Value s = TOY_VALUE_FROM_STRING(Toy_createString(&bucket, "Hello world"));

		if (Toy_hashValue(n) != 0 ||
			Toy_hashValue(t) != 1 ||
			Toy_hashValue(f) != 0 ||
			Toy_hashValue(i) != 4147366645 ||
			Toy_hashValue(s) != 994097935 ||
			TOY_VALUE_AS_STRING(s)->cachedHash == 0
			)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected hash of a value\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}
