#include "node.h"

#include "memory.h"
#include "console_colors.h"

#include <stdio.h>

int main() {
	{
		//test literals
		char* str = "foobar";

		Literal literal =  TO_STRING_LITERAL(copyString(str, strlen(str)), strlen(str));

		Node* node;
		emitNodeLiteral(&node, literal);
		freeLiteral(literal);
		freeNode(node);
	}

	{
		//test compound (dictionary)
		char* idn = "foobar";
		char* str = "hello world";

		Node* dictionary;
		Node* left;
		Node* right;

		Literal identifier = TO_IDENTIFIER_LITERAL(copyString(idn, strlen(idn)), strlen(idn));
		Literal string = TO_STRING_LITERAL(copyString(str, strlen(str)), strlen(str));

		emitNodeCompound(&dictionary, LITERAL_DICTIONARY);
		emitNodeLiteral(&left, identifier);
		emitNodeLiteral(&right, string);

		//grow the node if needed
		if (dictionary->compound.capacity < dictionary->compound.count + 1) {
			int oldCapacity = dictionary->compound.capacity;

			dictionary->compound.capacity = GROW_CAPACITY(oldCapacity);
			dictionary->compound.nodes = GROW_ARRAY(Node, dictionary->compound.nodes, oldCapacity, dictionary->compound.capacity);
		}

		//store the left and right in the node
		setNodePair(&dictionary->compound.nodes[dictionary->compound.count++], left, right);

		//the real test
		freeNode(dictionary);
		freeLiteral(identifier);
		freeLiteral(string);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

