#include "toy_routine.h"
#include "toy_console_colors.h"

#include "toy_opcodes.h"
#include "toy_lexer.h"
#include "toy_parser.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//tests
int test_routine_expressions(Toy_Bucket** bucketHandle) {
	//simple test to ensure the header looks right with an empty ast
	{
		//setup
		Toy_Ast* ast = NULL;
		Toy_private_emitAstPass(bucketHandle, &ast);

		//run
		void* buffer = Toy_compileRoutine(ast);
		int len = ((int*)buffer)[0];

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 28 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, ast: PASS\n" TOY_CC_RESET);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 25)) != 0 ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, ast: PASS\n" TOY_CC_RESET);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//rerun the test with a more complex ast, derived from a snippet of source
	{
		//setup
		const char* source = ";"; //interestingly, different ASTs will produce the same output
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);
		int len = ((int*)buffer)[0];

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 28 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 25)) != 0 ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a null value
	{
		//setup
		const char* source = "null;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);
		int len = ((int*)buffer)[0];

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 32 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_NULL ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*((unsigned char*)(buffer + 28)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 29)) != 0 ||
			*((unsigned char*)(buffer + 30)) != 0 ||
			*((unsigned char*)(buffer + 31)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a boolean value
	{
		//setup
		const char* source = "true;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);
		int len = ((int*)buffer)[0];

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 32 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_BOOLEAN ||
			*((unsigned char*)(buffer + 26)) != 1 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*((unsigned char*)(buffer + 28)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 29)) != 0 ||
			*((unsigned char*)(buffer + 30)) != 0 ||
			*((unsigned char*)(buffer + 31)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce an integer value
	{
		//setup
		const char* source = "42;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);
		int len = ((int*)buffer)[0];

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 36 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(int*)(buffer + 28) != 42 ||
			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 33)) != 0 ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a float value
	{
		//setup
		const char* source = "3.1415;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);
		int len = ((int*)buffer)[0];

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 36 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_FLOAT ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(float*)(buffer + 28) != 3.1415f ||
			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 33)) != 0 ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	return 0;
}

// int test_routine_unary(Toy_Bucket** bucketHandle) {
// 	//Nothing produces a unary instruction yet
// }

int test_routine_binary(Toy_Bucket** bucketHandle) {
	//produce a simple algorithm
	{
		//setup
		const char* source = "3 + 5;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);
		int len = ((int*)buffer)[0];

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 48 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(int*)(buffer + 28) != 3 ||

			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 33)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0 ||
			*(int*)(buffer + 36) != 5 ||

			*((unsigned char*)(buffer + 40)) != TOY_OPCODE_ADD ||
			*((unsigned char*)(buffer + 41)) != 0 ||
			*((unsigned char*)(buffer + 42)) != 0 ||
			*((unsigned char*)(buffer + 43)) != 0 ||

			*((unsigned char*)(buffer + 44)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 45)) != 0 ||
			*((unsigned char*)(buffer + 46)) != 0 ||
			*((unsigned char*)(buffer + 47)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a simple comparison
	{
		//setup
		const char* source = "3 == 5;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);
		int len = ((int*)buffer)[0];

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 48 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(int*)(buffer + 28) != 3 ||

			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 33)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0 ||
			*(int*)(buffer + 36) != 5 ||

			*((unsigned char*)(buffer + 40)) != TOY_OPCODE_COMPARE_EQUAL ||
			*((unsigned char*)(buffer + 41)) != 0 ||
			*((unsigned char*)(buffer + 42)) != 0 ||
			*((unsigned char*)(buffer + 43)) != 0 ||

			*((unsigned char*)(buffer + 44)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 45)) != 0 ||
			*((unsigned char*)(buffer + 46)) != 0 ||
			*((unsigned char*)(buffer + 47)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a simple comparison
	{
		//setup
		const char* source = "3 != 5;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);
		int len = ((int*)buffer)[0];

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 48 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(int*)(buffer + 28) != 3 ||

			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 33)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0 ||
			*(int*)(buffer + 36) != 5 ||

			*((unsigned char*)(buffer + 40)) != TOY_OPCODE_COMPARE_EQUAL ||
			*((unsigned char*)(buffer + 41)) != TOY_OPCODE_NEGATE ||
			*((unsigned char*)(buffer + 42)) != 0 ||
			*((unsigned char*)(buffer + 43)) != 0 ||

			*((unsigned char*)(buffer + 44)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 45)) != 0 ||
			*((unsigned char*)(buffer + 46)) != 0 ||
			*((unsigned char*)(buffer + 47)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	//produce a more complex algorithm
	{
		//setup
		const char* source = "(1 + 2) * (3 + 4);";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);
		int len = ((int*)buffer)[0];

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 72 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (
			//left hand side
			*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(int*)(buffer + 28) != 1 ||

			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 33)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0 ||
			*(int*)(buffer + 36) != 2 ||

			*((unsigned char*)(buffer + 40)) != TOY_OPCODE_ADD ||
			*((unsigned char*)(buffer + 41)) != 0 ||
			*((unsigned char*)(buffer + 42)) != 0 ||
			*((unsigned char*)(buffer + 43)) != 0 ||

			//right hand side
			*((unsigned char*)(buffer + 44)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 45)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 46)) != 0 ||
			*((unsigned char*)(buffer + 47)) != 0 ||
			*(int*)(buffer + 48) != 3 ||

			*((unsigned char*)(buffer + 52)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 53)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 54)) != 0 ||
			*((unsigned char*)(buffer + 55)) != 0 ||
			*(int*)(buffer + 56) != 4 ||

			*((unsigned char*)(buffer + 60)) != TOY_OPCODE_ADD ||
			*((unsigned char*)(buffer + 61)) != 0 ||
			*((unsigned char*)(buffer + 62)) != 0 ||
			*((unsigned char*)(buffer + 63)) != 0 ||

			//multiply the two values
			*((unsigned char*)(buffer + 64)) != TOY_OPCODE_MULTIPLY ||
			*((unsigned char*)(buffer + 65)) != 0 ||
			*((unsigned char*)(buffer + 66)) != 0 ||
			*((unsigned char*)(buffer + 67)) != 0 ||

			*((unsigned char*)(buffer + 68)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 69)) != 0 ||
			*((unsigned char*)(buffer + 70)) != 0 ||
			*((unsigned char*)(buffer + 71)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	return 0;
}

int test_routine_keywords(Toy_Bucket** bucketHandle) {
	//print
	{
		//setup
		const char* source = "print 42;";
		Toy_Lexer lexer;
		Toy_Parser parser;

		Toy_bindLexer(&lexer, source);
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(bucketHandle, &parser);

		//run
		void* buffer = Toy_compileRoutine(ast);
		int len = ((int*)buffer)[0];

		//check header
		int* ptr = (int*)buffer;

		if ((ptr++)[0] != 40 || //total size
			(ptr++)[0] != 0 || //param count
			(ptr++)[0] != 0 || //jump count
			(ptr++)[0] != 0 || //data count
			(ptr++)[0] != 0) //subs count
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine header, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//check code
		if (*((unsigned char*)(buffer + 24)) != TOY_OPCODE_READ ||
			*((unsigned char*)(buffer + 25)) != TOY_VALUE_INTEGER ||
			*((unsigned char*)(buffer + 26)) != 0 ||
			*((unsigned char*)(buffer + 27)) != 0 ||
			*(int*)(buffer + 28) != 42 ||
			*((unsigned char*)(buffer + 32)) != TOY_OPCODE_PRINT ||
			*((unsigned char*)(buffer + 33)) != 0 ||
			*((unsigned char*)(buffer + 34)) != 0 ||
			*((unsigned char*)(buffer + 35)) != 0 ||
			*((unsigned char*)(buffer + 36)) != TOY_OPCODE_RETURN ||
			*((unsigned char*)(buffer + 37)) != 0 ||
			*((unsigned char*)(buffer + 38)) != 0 ||
			*((unsigned char*)(buffer + 39)) != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to produce the expected routine code, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			free(buffer);
			return -1;
		}

		//cleanup
		free(buffer);
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(Toy_Ast) * 32);
		res = test_routine_expressions(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(Toy_Ast) * 32);
		res = test_routine_binary(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

		{
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(Toy_Ast) * 32);
		res = test_routine_keywords(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}