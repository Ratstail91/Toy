#include "toy_bytecode.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <string.h>

//tests
int test_bytecode_header(Toy_Bucket* bucket) {
	//simple test to ensure the header looks right
	{
		//setup
		Toy_Ast* ast = NULL;
		Toy_private_emitAstPass(&bucket, &ast);

		//run
		Toy_Bytecode bc = Toy_compileBytecode(ast);

		//check
		if (bc.ptr[0] != TOY_VERSION_MAJOR ||
			bc.ptr[1] != TOY_VERSION_MINOR ||
			bc.ptr[2] != TOY_VERSION_PATCH ||
			strcmp((char*)(bc.ptr + 3), TOY_VERSION_BUILD) != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to write the bytecode header correctly:\n" TOY_CC_RESET);
			fprintf(stderr, TOY_CC_ERROR "\t%d.%d.%d.%s\n" TOY_CC_RESET, bc.ptr[0], bc.ptr[1], bc.ptr[2], (char*)(bc.ptr + 3));
			fprintf(stderr, TOY_CC_ERROR "\t%d.%d.%d.%s\n" TOY_CC_RESET, TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, TOY_VERSION_BUILD);
			return -1;
		}

		//cleanup
		Toy_freeBytecode(bc);
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		Toy_Bucket* bucket = NULL;
		TOY_BUCKET_INIT(Toy_Ast, bucket, 32);
		res = test_bytecode_header(bucket);
		TOY_BUCKET_FREE(bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}