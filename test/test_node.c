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

	{
		//test compound (dictionary)
		char* idn = "foobar";
		char* str = "hello world";

		Node* dictionary;
		Node* left;
		Node* right;

		emitNodeCompound(&dictionary, LITERAL_DICTIONARY);
		emitNodeLiteral(&left, TO_IDENTIFIER_LITERAL(copyString(idn, strlen(idn)), strlen(idn)) );
		emitNodeLiteral(&right, TO_STRING_LITERAL(copyString(str, strlen(str)), strlen(str)) );

		//grow the node if needed
		if (dictionary->compound.capacity < dictionary->compound.count + 1) {
			int oldCapacity = dictionary->compound.capacity;

			dictionary->compound.capacity = GROW_CAPACITY(oldCapacity);
			dictionary->compound.nodes = GROW_ARRAY(Node, dictionary->compound.nodes, oldCapacity, dictionary->compound.capacity);
		}

		//store the left and right in the node
		Node* pair = NULL;
		emitNodePair(&pair, left, right);
		dictionary->compound.nodes[dictionary->compound.count++] = *pair;

		//the real test
		freeNode(dictionary);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

