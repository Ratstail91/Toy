#include "toy_vm.h"
#include "toy_console_colors.h"

#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_bytecode.h"

#include <stdio.h>
#include <string.h>

//utils
Toy_Bytecode makeBytecodeFromSource(Toy_Bucket** bucket, const char* source) {
	Toy_Lexer lexer;
	Toy_bindLexer(&lexer, source);

	Toy_Parser parser;
	Toy_bindParser(&parser, &lexer);

	Toy_Ast* ast = Toy_scanParser(bucket, &parser);
	Toy_Bytecode bc = Toy_compileBytecode(ast);

	return bc;
}

//tests
int test_setup_and_teardown(Toy_Bucket** bucket) {
	//basic init & quit
	{
		//generate bytecode for testing
		const char* source = "(1 + 2) * (3 + 4);";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucket, &parser);

		Toy_Bytecode bc = Toy_compileBytecode(ast);

		//run the setup
		Toy_VM vm;
		Toy_bindVM(&vm, bc.ptr, bc.capacity);

		//check the header size
		int headerSize = 3 + strlen(TOY_VERSION_BUILD) + 1;
		if (headerSize % 4 != 0) {
			headerSize += 4 - (headerSize % 4); //ceil
		}

		//check the routine was loaded correctly
		if (
			vm.routine - vm.bc != headerSize ||
			vm.routineSize != 72 ||
			vm.paramCount != 0 ||
			vm.jumpsCount != 0 ||
			vm.dataCount != 0 ||
			vm.subsCount != 0
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to setup and teadown Toy_VM, source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			Toy_freeVM(&vm);
			return -1;
		}

		//don't run it this time, simply teadown
		Toy_freeVM(&vm);
	}

	return 0;
}

int test_simple_execution(Toy_Bucket** bucket) {
	//test execution
	{
		//generate bytecode for testing
		const char* source = "(1 + 2) * (3 + 4);";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucket, &parser);

		Toy_Bytecode bc = Toy_compileBytecode(ast);

		//run the setup
		Toy_VM vm;
		Toy_bindVM(&vm, bc.ptr, bc.capacity);

		//run
		Toy_runVM(&vm);

		//check the final state of the stack
		if (vm.stack == NULL ||
			vm.stack->count != 1 ||
			TOY_VALUE_IS_INTEGER( Toy_peekStack(&vm.stack) ) != true ||
			TOY_VALUE_AS_INTEGER( Toy_peekStack(&vm.stack) ) != 21
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected result in 'Toy_VM', source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			Toy_freeVM(&vm);
			return -1;
		}

		//teadown
		Toy_freeVM(&vm);
	}

	return 0;
}

int test_opcode_not_equal(Toy_Bucket** bucket) {
	//testing a specific opcode; '!=' is compressed into a single word, so lets check it works
	{
		//generate bytecode for testing
		const char* source = "3 != 5;";

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Ast* ast = Toy_scanParser(bucket, &parser);

		Toy_Bytecode bc = Toy_compileBytecode(ast);

		//run the setup
		Toy_VM vm;
		Toy_bindVM(&vm, bc.ptr, bc.capacity);

		//run
		Toy_runVM(&vm);

		//check the final state of the stack
		if (vm.stack == NULL ||
			vm.stack->count != 1 ||
			TOY_VALUE_IS_BOOLEAN( Toy_peekStack(&vm.stack) ) != true ||
			TOY_VALUE_AS_BOOLEAN( Toy_peekStack(&vm.stack) ) != true
		)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected result in 'Toy_VM', source: %s\n" TOY_CC_RESET, source);

			//cleanup and return
			Toy_freeVM(&vm);
			return -1;
		}

		//teadown
		Toy_freeVM(&vm);
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(Toy_Ast) * 32);
		res = test_setup_and_teardown(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(Toy_Ast) * 32);
		res = test_simple_execution(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		Toy_Bucket* bucket = Toy_allocateBucket(sizeof(Toy_Ast) * 32);
		res = test_opcode_not_equal(&bucket);
		Toy_freeBucket(&bucket);
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
