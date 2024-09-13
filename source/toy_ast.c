#include "toy_ast.h"

void Toy_private_initAstBlock(Toy_Bucket** bucket, Toy_Ast** handle) {
	(*handle) = (Toy_Ast*)Toy_partBucket(bucket, sizeof(Toy_Ast));

	(*handle)->block.type = TOY_AST_BLOCK;
	(*handle)->block.child = NULL;
	(*handle)->block.next = NULL;
	(*handle)->block.tail = NULL;
}

void Toy_private_appendAstBlock(Toy_Bucket** bucket, Toy_Ast** handle, Toy_Ast* child) {
	//type check

	//first, check if we're an empty head
	if ((*handle)->block.child == NULL) {
		(*handle)->block.child = child;
		return; //NOTE: first call on an empty head skips any memory allocations
	}

	//run (or jump) until we hit the current tail
	Toy_Ast* iter = (*handle)->block.tail ? (*handle)->block.tail : (*handle);

	while(iter->block.next != NULL) {
		iter = iter->block.next;
	}

	//append a new link to the chain
	Toy_private_initAstBlock(bucket, &(iter->block.next));

	//store the child in the new link, prep the tail pointer
	iter->block.next->block.child = child;
	(*handle)->block.tail = iter->block.next;
}

void Toy_private_emitAstValue(Toy_Bucket** bucket, Toy_Ast** handle, Toy_Value value) {
	(*handle) = (Toy_Ast*)Toy_partBucket(bucket, sizeof(Toy_Ast));

	(*handle)->value.type = TOY_AST_VALUE;
	(*handle)->value.value = value;
}

//TODO: flag range checks
void Toy_private_emitAstUnary(Toy_Bucket** bucket, Toy_Ast** handle, Toy_AstFlag flag) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partBucket(bucket, sizeof(Toy_Ast));

	tmp->unary.type = TOY_AST_UNARY;
	tmp->unary.flag = flag;
	tmp->unary.child = *handle;

	(*handle) = tmp;
}

void Toy_private_emitAstBinary(Toy_Bucket** bucket, Toy_Ast** handle, Toy_AstFlag flag, Toy_Ast* right) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partBucket(bucket, sizeof(Toy_Ast));

	tmp->binary.type = TOY_AST_BINARY;
	tmp->binary.flag = flag;
	tmp->binary.left = *handle; //left-recursive
	tmp->binary.right = right;

	(*handle) = tmp;
}

void Toy_private_emitAstGroup(Toy_Bucket** bucket, Toy_Ast** handle) {
	Toy_Ast* tmp = (Toy_Ast*)Toy_partBucket(bucket, sizeof(Toy_Ast));

	tmp->group.type = TOY_AST_GROUP;
	tmp->group.child = (*handle);

	(*handle) = tmp;
}

void Toy_private_emitAstPass(Toy_Bucket** bucket, Toy_Ast** handle) {
	(*handle) = (Toy_Ast*)Toy_partBucket(bucket, sizeof(Toy_Ast));

	(*handle)->pass.type = TOY_AST_PASS;
}

void Toy_private_emitAstError(Toy_Bucket** bucket, Toy_Ast** handle) {
	(*handle) = (Toy_Ast*)Toy_partBucket(bucket, sizeof(Toy_Ast));

	(*handle)->error.type = TOY_AST_ERROR;
}

void Toy_private_emitAstEnd(Toy_Bucket** bucket, Toy_Ast** handle) {
	(*handle) = (Toy_Ast*)Toy_partBucket(bucket, sizeof(Toy_Ast));

	(*handle)->error.type = TOY_AST_END;
}
