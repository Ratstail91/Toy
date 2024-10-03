#include "toy_table.h"
#include "toy_console_colors.h"

#include <stdio.h>

int test_table_allocation() {
	return 0;
}

int main() {
	//not finished yet
	fprintf(stderr, TOY_CC_WARN "'Toy_Table' is not yet tested\n" TOY_CC_RESET);
	return 0;

	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_table_allocation();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
