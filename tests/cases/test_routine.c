#include "toy_routine.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <string.h>

//tests
int test_routine_header(Toy_Bucket* bucket) {
	//simple test to ensure the header looks right
	{
		//setup
		Toy_Ast* ast = NULL;
		Toy_private_emitAstPass(&bucket, &ast);

		//run
		void* buffer = Toy_compileRoutine(ast);
		int len = ((int*)buffer)[0];

		//check
		//TODO

		//cleanup
		TOY_FREE_ARRAY(unsigned char, buffer, len);
	}

	return 0;
}

int main() {
	fprintf(stderr, TOY_CC_WARN "WARNING: Routine tests incomplete\n" TOY_CC_RESET);

	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		Toy_Bucket* bucket = NULL;
		TOY_BUCKET_INIT(Toy_Ast, bucket, 32);
		res = test_routine_header(bucket);
		TOY_BUCKET_FREE(bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}