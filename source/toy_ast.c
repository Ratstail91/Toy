#include "toy_ast.h"

void Toy_private_initAstBlock(Toy_Bucket** bucket, Toy_Ast** handle) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucket, sizeof(Toy_Ast));

	tmp->type = TOY_AST_BLOCK;
	tmp->block.child = NULL;
	tmp->block.next = NULL;
	tmp->block.tail = NULL;

	(*handle) = tmp;
}

void Toy_private_appendAstBlock(Toy_Bucket** bucket, Toy_Ast* block, Toy_Ast* child) {
	//first, check if we're an empty head
	if (block->block.child == NULL) {
		block->block.child = child;
		return; //NOTE: first call on an empty head skips any memory allocations
	}

	//run (or jump) until we hit the current tail
	Toy_Ast* iter = block->block.tail ? block->block.tail : block;

	while(iter->block.next != NULL) {
		iter = iter->block.next;
	}

	//append a new link to the chain
	Toy_private_initAstBlock(bucket, &(iter->block.next));

	//store the child in the new link, prep the tail pointer
	iter->block.next->block.child = child;
	block->block.tail = iter->block.next;
}

void Toy_private_emitAstValue(Toy_Bucket** bucket, Toy_Ast** handle, Toy_Value value) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucket, sizeof(Toy_Ast));

	tmp->type = TOY_AST_VALUE;
	tmp->value.value = value;

	(*handle) = tmp;
}

void Toy_private_emitAstUnary(Toy_Bucket** bucket, Toy_Ast** handle, Toy_AstFlag flag) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucket, sizeof(Toy_Ast));

	tmp->type = TOY_AST_UNARY;
	tmp->unary.flag = flag;
	tmp->unary.child = *handle;

	(*handle) = tmp;
}

void Toy_private_emitAstBinary(Toy_Bucket** bucket, Toy_Ast** handle, Toy_AstFlag flag, Toy_Ast* right) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucket, sizeof(Toy_Ast));

	tmp->type = TOY_AST_BINARY;
	tmp->binary.flag = flag;
	tmp->binary.left = *handle; //left-recursive
	tmp->binary.right = right;

	(*handle) = tmp;
}

void Toy_private_emitAstGroup(Toy_Bucket** bucket, Toy_Ast** handle) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucket, sizeof(Toy_Ast));

	tmp->type = TOY_AST_GROUP;
	tmp->group.child = (*handle);

	(*handle) = tmp;
}

void Toy_private_emitAstPass(Toy_Bucket** bucket, Toy_Ast** handle) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucket, sizeof(Toy_Ast));

	tmp->type = TOY_AST_PASS;

	(*handle) = tmp;
}

void Toy_private_emitAstError(Toy_Bucket** bucket, Toy_Ast** handle) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucket, sizeof(Toy_Ast));

	tmp->type = TOY_AST_ERROR;

	(*handle) = tmp;
}

void Toy_private_emitAstEnd(Toy_Bucket** bucket, Toy_Ast** handle) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partitionBucket(bucket, sizeof(Toy_Ast));

	tmp->type = TOY_AST_END;

	(*handle) = tmp;
}
