#include "toy_ast.h"
#include "toy_console_colors.h"

#include <stdio.h>

int test_sizeof_ast_64bit() {
#define TEST_SIZEOF(type, size) \
	if (sizeof(type) != size) { \
		fprintf(stderr, TOY_CC_ERROR "ERROR: sizeof(" #type ") is %d, expected %d\n" TOY_CC_RESET, (int)sizeof(type), size); \
		++err; \
	}

	//count errors
	int err = 0;

	//run for each type
	TEST_SIZEOF(Toy_AstType, 4);
	TEST_SIZEOF(Toy_AstBlock, 32);
	TEST_SIZEOF(Toy_AstValue, 12);
	TEST_SIZEOF(Toy_AstUnary, 16);
	TEST_SIZEOF(Toy_AstBinary, 24);
	TEST_SIZEOF(Toy_AstGroup, 16);
	TEST_SIZEOF(Toy_AstPass, 4);
	TEST_SIZEOF(Toy_AstError, 4);
	TEST_SIZEOF(Toy_AstEnd, 4);
	TEST_SIZEOF(Toy_Ast, 32);

#undef TEST_SIZEOF

	return -err;
}

int test_sizeof_ast_32bit() {
#define TEST_SIZEOF(type, size) \
	if (sizeof(type) != size) { \
		fprintf(stderr, TOY_CC_ERROR "ERROR: sizeof(" #type ") is %d, expected %d\n" TOY_CC_RESET, (int)sizeof(type), size); \
		++err; \
	}

	//count errors
	int err = 0;

	//run for each type
	TEST_SIZEOF(Toy_AstType, 4);
	TEST_SIZEOF(Toy_AstBlock, 16);
	TEST_SIZEOF(Toy_AstValue, 12);
	TEST_SIZEOF(Toy_AstUnary, 12);
	TEST_SIZEOF(Toy_AstBinary, 16);
	TEST_SIZEOF(Toy_AstGroup, 8);
	TEST_SIZEOF(Toy_AstPass, 4);
	TEST_SIZEOF(Toy_AstError, 4);
	TEST_SIZEOF(Toy_AstEnd, 4);
	TEST_SIZEOF(Toy_Ast, 16);

#undef TEST_SIZEOF

	return -err;
}

int test_type_emission(Toy_Bucket** bucket) {
	//emit value
	{
		//emit to an AST
		Toy_Ast* ast = NULL;
		Toy_private_emitAstValue(bucket, &ast, TOY_VALUE_TO_INTEGER(42));

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->value.value) != 42)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a value as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit unary
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_private_emitAstValue(bucket, &ast, TOY_VALUE_TO_INTEGER(42));
		Toy_private_emitAstUnary(bucket, &ast, TOY_AST_FLAG_NEGATE);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_UNARY ||
			ast->unary.flag != TOY_AST_FLAG_NEGATE ||
			ast->unary.child->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->unary.child->value.value) != 42)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a unary as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit binary
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_Ast* right = NULL;
		Toy_private_emitAstValue(bucket, &ast, TOY_VALUE_TO_INTEGER(42));
		Toy_private_emitAstValue(bucket, &right, TOY_VALUE_TO_INTEGER(69));
		Toy_private_emitAstBinary(bucket, &ast, TOY_AST_FLAG_ADD, right);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BINARY ||
			ast->binary.flag != TOY_AST_FLAG_ADD ||
			ast->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->binary.left->value.value) != 42 ||
			ast->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->binary.right->value.value) != 69)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a binary as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit group
	{
		//build the AST
		Toy_Ast* ast = NULL;
		Toy_Ast* right = NULL;
		Toy_private_emitAstValue(bucket, &ast, TOY_VALUE_TO_INTEGER(42));
		Toy_private_emitAstValue(bucket, &right, TOY_VALUE_TO_INTEGER(69));
		Toy_private_emitAstBinary(bucket, &ast, TOY_AST_FLAG_ADD, right);
		Toy_private_emitAstGroup(bucket, &ast);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_GROUP ||
			ast->group.child == NULL ||
			ast->group.child->type != TOY_AST_BINARY ||
			ast->group.child->binary.flag != TOY_AST_FLAG_ADD ||
			ast->group.child->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->group.child->binary.left->value.value) != 42 ||
			ast->group.child->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_AS_INTEGER(ast->group.child->binary.right->value.value) != 69)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a group as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
			return -1;
		}
	}

	//emit and append blocks of code
	{
		//initialize the root block
		Toy_Ast* block = NULL;
		Toy_private_initAstBlock(bucket, &block);

		//loop over the ast emissions, appending each one as you go
		for (int i = 0; i < 5; i++) {
			//build the AST
			Toy_Ast* ast = NULL;
			Toy_Ast* right = NULL;
			Toy_private_emitAstValue(bucket, &ast, TOY_VALUE_TO_INTEGER(42));
			Toy_private_emitAstValue(bucket, &right, TOY_VALUE_TO_INTEGER(69));
			Toy_private_emitAstBinary(bucket, &ast, TOY_AST_FLAG_ADD, right);
			Toy_private_emitAstGroup(bucket, &ast);

			Toy_private_appendAstBlock(bucket, &block, ast);
		}

		//check if it worked
		Toy_Ast* iter = block;

		while(iter != NULL) {
			if (
				iter->type != TOY_AST_BLOCK ||
				iter->block.child == NULL ||
				iter->block.child->type != TOY_AST_GROUP ||
				iter->block.child->group.child == NULL ||
				iter->block.child->group.child->type != TOY_AST_BINARY ||
				iter->block.child->group.child->binary.flag != TOY_AST_FLAG_ADD ||
				iter->block.child->group.child->binary.left->type != TOY_AST_VALUE ||
				TOY_VALUE_AS_INTEGER(iter->block.child->group.child->binary.left->value.value) != 42 ||
				iter->block.child->group.child->binary.right->type != TOY_AST_VALUE ||
				TOY_VALUE_AS_INTEGER(iter->block.child->group.child->binary.right->value.value) != 69)
			{
				fprintf(stderr, TOY_CC_ERROR "ERROR: failed to emit a block as 'Toy_Ast', state unknown\n" TOY_CC_RESET);
				return -1;
			}

			iter = iter->block.next;
		}
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;


#if TOY_BITNESS == 64
	res = test_sizeof_ast_64bit();
	total += res;

	if (res == 0) {
		printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	}
#elif TOY_BITNESS == 32
	res = test_sizeof_ast_32bit();
	total += res;

	if (res == 0) {
		printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	}
#else
	fprintf(stderr, TOY_CC_WARN "WARNING: Skipping test_sizeof_ast_*bit(); Can't determine the 'bitness' of this platform (seems to be %d)\n" TOY_CC_RESET, TOY_BITNESS);
#endif

	{
		Toy_Bucket* bucket = NULL;
		TOY_BUCKET_INIT(Toy_Ast, bucket, 32);
		res = test_type_emission(&bucket);
		TOY_BUCKET_FREE(bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}