#include "node.h"

#include "memory.h"
#include "console_colors.h"

#include <stdio.h>

int main() {
	{
		//test literals
		char* str = "foobar";

		Node* node;
		emitNodeLiteral(&node, TO_STRING_LITERAL(copyString(str, strlen(str)), strlen(str)) );
		freeNode(node);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

