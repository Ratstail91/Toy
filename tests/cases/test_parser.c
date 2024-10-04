#include "toy_parser.h"
#include "toy_console_colors.h"

#include <stdio.h>

//utils
Toy_Ast* makeAstFromSource(Toy_Bucket** bucketHandle, const char* source) {
	Toy_Lexer lexer;
	Toy_bindLexer(&lexer, source);

	Toy_Parser parser;
	Toy_bindParser(&parser, &lexer);

	return Toy_scanParser(bucketHandle, &parser);
}

//tests
int test_simple_empty_parsers(Toy_Bucket** bucketHandle) {
	//simple parser setup and cleanup
	{
		//raw source code and lexer
		const char* source = "";
		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		//parser
		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_END)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with empty source\n" TOY_CC_RESET);
			return -1;
		}
	}

	//repeat above, but with one semicolon
	{
		//raw source code and lexer
		const char* source = ";";
		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		//parser
		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_PASS)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with one semicolon\n" TOY_CC_RESET);
			return -1;
		}
	}

	//repeat above, but with multiple semicolons
	{
		//raw source code and lexer
		const char* source = ";;;;;";
		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		//parser
		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		Toy_Ast* iter = ast;

		while(iter != NULL) {
			//check each link and child
			if (
				iter == NULL ||
				iter->type != TOY_AST_BLOCK ||
				iter->block.child == NULL ||
				iter->block.child->type != TOY_AST_PASS)
			{
				fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with multiple semicolons\n" TOY_CC_RESET);
				return -1;
			}

			iter = iter->block.next;
		}
	}

	return 0;
}

int test_values(Toy_Bucket** bucketHandle) {
	//test boolean true
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "true;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_BOOLEAN(ast->block.child->value.value) == false ||
			TOY_VALUE_AS_BOOLEAN(ast->block.child->value.value) != true)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with boolean value true\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test boolean false (just to be safe)
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "false;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_BOOLEAN(ast->block.child->value.value) == false ||
			TOY_VALUE_AS_BOOLEAN(ast->block.child->value.value) != false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with boolean value false\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test integer
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "42;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->value.value) != 42)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with integer value 42\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test float
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "3.1415;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_FLOAT(ast->block.child->value.value) == false ||
			TOY_VALUE_AS_FLOAT(ast->block.child->value.value) != 3.1415f)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with float value 3.1415\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test integer with separators
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "1_234_567_890;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->value.value) != 1234567890)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with integer value 42 with separators\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test float with separators
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "3.141_592_65;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_FLOAT(ast->block.child->value.value) == false ||
			TOY_VALUE_AS_FLOAT(ast->block.child->value.value) != 3.14159265f)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with float value 3.1415 with separators\n" TOY_CC_RESET);
			return -1;
		}
	}

	return 0;
}

int test_unary(Toy_Bucket** bucketHandle) {
	//test unary boolean negation (!true)
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "!true;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_BOOLEAN(ast->block.child->value.value) == false ||
			TOY_VALUE_AS_BOOLEAN(ast->block.child->value.value) != false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with boolean value !true (unary negation)\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test unary boolean negation (!false, just to be safe)
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "!false;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_BOOLEAN(ast->block.child->value.value) == false ||
			TOY_VALUE_AS_BOOLEAN(ast->block.child->value.value) != true)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with boolean value !false (unary negation)\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test unary integer negation
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "-42;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->value.value) != -42)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with integer value -42 (unary negation)\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test unary float negation
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "-3.1415;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_FLOAT(ast->block.child->value.value) == false ||
			TOY_VALUE_AS_FLOAT(ast->block.child->value.value) != -3.1415f)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with float value -3.1415 (unary negation)\n" TOY_CC_RESET);
			return -1;
		}
	}

	//ensure unary negation doesn't occur with a group
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "-(42);");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type == TOY_AST_VALUE)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: unexpected successful unary negation in parser with grouped value -(42)\n" TOY_CC_RESET);
			return -1;
		}
	}

	//ensure unary negation doesn't occur with a space
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "- 42;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type == TOY_AST_VALUE)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: unexpected successful unary negation in parser with space character '- 42'\n" TOY_CC_RESET);
			return -1;
		}
	}

	return 0;
}

int test_binary(Toy_Bucket** bucketHandle) {
	//test binary add (term); also covers subtract
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "1 + 2;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_BINARY ||
			ast->block.child->binary.flag != TOY_AST_FLAG_ADD ||

			ast->block.child->binary.left == NULL ||
			ast->block.child->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->value.value) != 1 ||

			ast->block.child->binary.right == NULL ||
			ast->block.child->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.right->value.value) != 2)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with binary add '1 + 2' (term)\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test binary multiply (factor); also covers divide and modulo
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "3 * 5;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_BINARY ||
			ast->block.child->binary.flag != TOY_AST_FLAG_MULTIPLY ||

			ast->block.child->binary.left == NULL ||
			ast->block.child->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->value.value) != 3 ||

			ast->block.child->binary.right == NULL ||
			ast->block.child->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.right->value.value) != 5)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with binary multiply '3 * 5' (factor)\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test binary assign (using numbers for now, as identifiers aren't coded yet)
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "1 = 2;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_BINARY ||
			ast->block.child->binary.flag != TOY_AST_FLAG_ASSIGN ||

			ast->block.child->binary.left == NULL ||
			ast->block.child->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->value.value) != 1 ||

			ast->block.child->binary.right == NULL ||
			ast->block.child->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.right->value.value) != 2)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with binary assign '1 = 2'\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test binary compare (equality)
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "42 == 69;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_BINARY ||
			ast->block.child->binary.flag != TOY_AST_FLAG_COMPARE_EQUAL ||

			ast->block.child->binary.left == NULL ||
			ast->block.child->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->value.value) != 42 ||

			ast->block.child->binary.right == NULL ||
			ast->block.child->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.right->value.value) != 69)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser with binary compare '42 == 69'\n" TOY_CC_RESET);
			return -1;
		}
	}

	return 0;
}

int test_precedence(Toy_Bucket** bucketHandle) {
	//test term-factor precedence
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "1 * 2 + 3 * 4;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_BINARY ||
			ast->block.child->binary.flag != TOY_AST_FLAG_ADD ||

			ast->block.child->binary.left == NULL ||
			ast->block.child->binary.left->type != TOY_AST_BINARY ||
			ast->block.child->binary.left->binary.flag != TOY_AST_FLAG_MULTIPLY ||
			ast->block.child->binary.left->binary.left == NULL ||
			ast->block.child->binary.left->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->binary.left->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->binary.left->value.value) != 1 ||
			ast->block.child->binary.left->binary.right == NULL ||
			ast->block.child->binary.left->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->binary.right->value.value) != 2 ||

			ast->block.child->binary.right == NULL ||
			ast->block.child->binary.right->type != TOY_AST_BINARY ||
			ast->block.child->binary.right->binary.flag != TOY_AST_FLAG_MULTIPLY ||
			ast->block.child->binary.right->binary.left == NULL ||
			ast->block.child->binary.right->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.right->binary.left->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.right->binary.left->value.value) != 3 ||
			ast->block.child->binary.right->binary.right == NULL ||
			ast->block.child->binary.right->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.right->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.right->binary.right->value.value) != 4)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser precedence '1 * 2 + 3 * 4' (term-factor)\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test left-recrusive precedence
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "1 + 2 + 3 + 4 + 5 + 6;");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_BINARY ||
			ast->block.child->binary.flag != TOY_AST_FLAG_ADD ||

			// start from the right and work backwards
			ast->block.child->binary.right == NULL ||
			ast->block.child->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.right->value.value) != 6 ||

			ast->block.child->binary.left == NULL ||
			ast->block.child->binary.left->type != TOY_AST_BINARY ||
			ast->block.child->binary.left->binary.right == NULL ||
			ast->block.child->binary.left->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->binary.right->value.value) != 5 ||

			ast->block.child->binary.left->binary.left == NULL ||
			ast->block.child->binary.left->binary.left->type != TOY_AST_BINARY ||
			ast->block.child->binary.left->binary.left->binary.right == NULL ||
			ast->block.child->binary.left->binary.left->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->binary.left->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->binary.left->binary.right->value.value) != 4 ||

			ast->block.child->binary.left->binary.left->binary.left == NULL ||
			ast->block.child->binary.left->binary.left->binary.left->type != TOY_AST_BINARY ||
			ast->block.child->binary.left->binary.left->binary.left->binary.right == NULL ||
			ast->block.child->binary.left->binary.left->binary.left->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->binary.left->binary.left->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->binary.left->binary.left->binary.right->value.value) != 3 ||

			ast->block.child->binary.left->binary.left->binary.left->binary.left == NULL ||
			ast->block.child->binary.left->binary.left->binary.left->binary.left->type != TOY_AST_BINARY ||
			ast->block.child->binary.left->binary.left->binary.left->binary.left->binary.right == NULL ||
			ast->block.child->binary.left->binary.left->binary.left->binary.left->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->binary.left->binary.left->binary.left->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->binary.left->binary.left->binary.left->binary.right->value.value) != 2 ||

			ast->block.child->binary.left->binary.left->binary.left->binary.left->binary.left == NULL ||
			ast->block.child->binary.left->binary.left->binary.left->binary.left->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->binary.left->binary.left->binary.left->binary.left->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->binary.left->binary.left->binary.left->binary.left->value.value) != 1)

		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser precedence '1 + 2 + 3 + 4 + 5 + 6' (left-recursive)\n" TOY_CC_RESET);
			return -1;
		}
	}

	//test group precedence
	{
		Toy_Ast* ast = makeAstFromSource(bucketHandle, "(1 + 2) * (3 + 4);");

		//check if it worked
		if (
			ast == NULL ||
			ast->type != TOY_AST_BLOCK ||
			ast->block.child == NULL ||
			ast->block.child->type != TOY_AST_BINARY ||
			ast->block.child->binary.flag != TOY_AST_FLAG_MULTIPLY ||

			ast->block.child->binary.left == NULL ||
			ast->block.child->binary.left->type != TOY_AST_BINARY ||
			ast->block.child->binary.left->binary.flag != TOY_AST_FLAG_ADD ||
			ast->block.child->binary.left->binary.left == NULL ||
			ast->block.child->binary.left->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->binary.left->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->binary.left->value.value) != 1 ||
			ast->block.child->binary.left->binary.right == NULL ||
			ast->block.child->binary.left->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.left->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.left->binary.right->value.value) != 2 ||

			ast->block.child->binary.right == NULL ||
			ast->block.child->binary.right->type != TOY_AST_BINARY ||
			ast->block.child->binary.right->binary.flag != TOY_AST_FLAG_ADD ||
			ast->block.child->binary.right->binary.left == NULL ||
			ast->block.child->binary.right->binary.left->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.right->binary.left->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.right->binary.left->value.value) != 3 ||
			ast->block.child->binary.right->binary.right == NULL ||
			ast->block.child->binary.right->binary.right->type != TOY_AST_VALUE ||
			TOY_VALUE_IS_INTEGER(ast->block.child->binary.right->binary.right->value.value) == false ||
			TOY_VALUE_AS_INTEGER(ast->block.child->binary.right->binary.right->value.value) != 4)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to run the parser precedence '(1 + 2) * (3 + 4)' (group)\n" TOY_CC_RESET);
			return -1;
		}
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(Toy_Ast) * 32);
		res = test_simple_empty_parsers(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(Toy_Ast) * 32);
		res = test_values(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(Toy_Ast) * 32);
		res = test_unary(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(Toy_Ast) * 32);
		res = test_binary(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(Toy_Ast) * 32);
		res = test_precedence(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
