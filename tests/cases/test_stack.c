#include "toy_stack.h"
#include "toy_console_colors.h"

#include <stdio.h>

int test_stack_with_init() {
	//init and free the stack
	{
		Toy_Stack stack;
		Toy_initStack(&stack);

		//check if it worked
		if (
			stack.ptr != NULL ||
			stack.capacity != 0 ||
			stack.count != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to init Toy_Stack\n" TOY_CC_RESET);
			return -1;
		}

		Toy_freeStack(&stack);
	}

	//push, peek and pop stack
	{
		Toy_Stack stack;
		Toy_initStack(&stack);

		//check if it worked (push)
		Toy_pushStack(&stack, TOY_VALUE_TO_INTEGER(42));
		Toy_pushStack(&stack, TOY_VALUE_TO_INTEGER(69));
		Toy_pushStack(&stack, TOY_VALUE_TO_INTEGER(420));
		if (
			stack.ptr == NULL ||
			stack.capacity != 64 ||
			stack.count != 3)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to push Toy_Stack\n" TOY_CC_RESET);
			return -1;
		}

		//check if it worked (peek)
		Toy_Value top1 = Toy_peekStack(&stack);
		if (
			TOY_VALUE_IS_INTEGER(top1) != true ||
			TOY_VALUE_AS_INTEGER(top1) != 420)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to peek Toy_Stack\n" TOY_CC_RESET);
			return -1;
		}

		//check if it worked (pop)
		Toy_Value top2 = Toy_popStack(&stack);
		if (
			stack.ptr == NULL ||
			stack.capacity != 64 ||
			stack.count != 2 ||
			TOY_VALUE_IS_INTEGER(top2) != true ||
			TOY_VALUE_AS_INTEGER(top2) != 420)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to pop Toy_Stack\n" TOY_CC_RESET);
			return -1;
		}

		//check if it worked (post-pop peek)
		Toy_Value top3 = Toy_peekStack(&stack);
		if (
			TOY_VALUE_IS_INTEGER(top3) != true ||
			TOY_VALUE_AS_INTEGER(top3) != 69)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to pop then peek Toy_Stack\n" TOY_CC_RESET);
			return -1;
		}

		Toy_freeStack(&stack);
	}

	return 0;
}

int test_stack_with_preallocate() {
	//preallocate and free the stack
	{
		Toy_Stack stack;
		Toy_preallocateStack(&stack);

		//check if it worked
		if (
			stack.ptr == NULL ||
			stack.capacity != 64 ||
			stack.count != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to preallocate Toy_Stack\n" TOY_CC_RESET);
			return -1;
		}

		Toy_freeStack(&stack);
	}

	//push, peek and pop stack
	{
		Toy_Stack stack;
		Toy_initStack(&stack);

		//check if it worked (push)
		Toy_pushStack(&stack, TOY_VALUE_TO_INTEGER(42));
		Toy_pushStack(&stack, TOY_VALUE_TO_INTEGER(69));
		Toy_pushStack(&stack, TOY_VALUE_TO_INTEGER(420));
		if (
			stack.ptr == NULL ||
			stack.capacity != 64 ||
			stack.count != 3)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to push Toy_Stack\n" TOY_CC_RESET);
			return -1;
		}

		//check if it worked (peek)
		Toy_Value top1 = Toy_peekStack(&stack);
		if (
			TOY_VALUE_IS_INTEGER(top1) != true ||
			TOY_VALUE_AS_INTEGER(top1) != 420)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to peek Toy_Stack\n" TOY_CC_RESET);
			return -1;
		}

		//check if it worked (pop)
		Toy_Value top2 = Toy_popStack(&stack);
		if (
			stack.ptr == NULL ||
			stack.capacity != 64 ||
			stack.count != 2 ||
			TOY_VALUE_IS_INTEGER(top2) != true ||
			TOY_VALUE_AS_INTEGER(top2) != 420)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to pop Toy_Stack\n" TOY_CC_RESET);
			return -1;
		}

		//check if it worked (post-pop peek)
		Toy_Value top3 = Toy_peekStack(&stack);
		if (
			TOY_VALUE_IS_INTEGER(top3) != true ||
			TOY_VALUE_AS_INTEGER(top3) != 69)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: failed to pop then peek Toy_Stack\n" TOY_CC_RESET);
			return -1;
		}

		Toy_freeStack(&stack);
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_stack_with_init();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_stack_with_preallocate();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
