#include "compiler.h"

#include "memory.h"

#include <stdio.h>

void initCompiler(Compiler* compiler) {
	initLiteralArray(&compiler->literalCache);
	compiler->bytecode = NULL;
	compiler->capacity = 0;
	compiler->count = 0;

	//default atomic literals
	Literal n = TO_NULL_LITERAL;
	Literal t = TO_BOOLEAN_LITERAL(true);
	Literal f = TO_BOOLEAN_LITERAL(false);

	pushLiteralArray(&compiler->literalCache, n);
	pushLiteralArray(&compiler->literalCache, t);
	pushLiteralArray(&compiler->literalCache, f);
}

void writeCompiler(Compiler* compiler, Node* node) {
	//grow if the bytecode space is too small
	if (compiler->capacity < compiler->count + 1) {
		int oldCapacity = compiler->capacity;

		compiler->capacity = GROW_CAPACITY(oldCapacity);
		compiler->bytecode = GROW_ARRAY(unsigned char, compiler->bytecode, oldCapacity, compiler->capacity);
	}

	//determine node type
	switch(node->type) {
		//TODO: more types, like variables, etc.
		case NODE_ERROR: {
			printf("[internal] NODE_ERROR encountered in writeCompiler()");
			compiler->bytecode[compiler->count++] = OP_EOF; //1 byte
		}
		break;

		case NODE_LITERAL: {
			//ensure the literal is in the cache
			int index = findLiteralIndex(&compiler->literalCache, node->atomic.literal);
			if (index < 0) {
				index = pushLiteralArray(&compiler->literalCache, node->atomic.literal);
			}

			//push the node opcode to the bytecode
			if (index >= 256) {
				//push a "long" index
				compiler->bytecode[compiler->count++] = OP_LITERAL_LONG; //1 byte
				*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)index; //2 bytes

				compiler->count += sizeof(unsigned short);
			}
			else {
				//push the index
				compiler->bytecode[compiler->count++] = OP_LITERAL; //1 byte
				compiler->bytecode[compiler->count++] = (unsigned char)index; //1 byte
			}
		}
		break;

		case NODE_UNARY:
			//pass to the child node, then embed the unary command (print, negate, etc.)
			writeCompiler(compiler, node->unary.child);
			compiler->bytecode[compiler->count++] = (unsigned char)node->unary.opcode; //1 byte
		break;

		case NODE_BINARY:
			//pass to the child nodes, then embed the binary command (math, etc.)
			writeCompiler(compiler, node->binary.left);
			writeCompiler(compiler, node->binary.right);
			compiler->bytecode[compiler->count++] = (unsigned char)node->binary.opcode; //1 byte
		break;

		case NODE_GROUPING:
			compiler->bytecode[compiler->count++] = (unsigned char)OP_GROUPING_BEGIN; //1 byte
			writeCompiler(compiler, node->grouping.child);
			compiler->bytecode[compiler->count++] = (unsigned char)OP_GROUPING_END; //1 byte
		break;
	}
}

void freeCompiler(Compiler* compiler) {
	freeLiteralArray(&compiler->literalCache);
	FREE(unsigned char, compiler->bytecode);
	compiler->bytecode = NULL;
	compiler->capacity = 0;
	compiler->count = 0;
}

static void emitByte(unsigned char** collationPtr, int* capacityPtr, int* countPtr, unsigned char byte) {
	//grow the array
	if (*countPtr + 1 > *capacityPtr) {
		int oldCapacity = *capacityPtr;
		*capacityPtr = GROW_CAPACITY(*capacityPtr);
		*collationPtr = GROW_ARRAY(unsigned char, *collationPtr, oldCapacity, *capacityPtr);
	}

	//append to the collation
	(*collationPtr)[(*countPtr)++] = byte;
}

static void emitShort(unsigned char** collationPtr, int* capacityPtr, int* countPtr, unsigned short bytes) {
	char* ptr = (char*)&bytes;

	emitByte(collationPtr, capacityPtr, countPtr, *ptr);
	ptr++;
	emitByte(collationPtr, capacityPtr, countPtr, *ptr);
}

static void emitInt(unsigned char** collationPtr, int* capacityPtr, int* countPtr, int bytes) {
	char* ptr = (char*)&bytes;

	emitByte(collationPtr, capacityPtr, countPtr, *ptr);
	ptr++;
	emitByte(collationPtr, capacityPtr, countPtr, *ptr);
	ptr++;
	emitByte(collationPtr, capacityPtr, countPtr, *ptr);
	ptr++;
	emitByte(collationPtr, capacityPtr, countPtr, *ptr);
}

static void emitFloat(unsigned char** collationPtr, int* capacityPtr, int* countPtr, float bytes) {
	char* ptr = (char*)&bytes;

	emitByte(collationPtr, capacityPtr, countPtr, *ptr);
	ptr++;
	emitByte(collationPtr, capacityPtr, countPtr, *ptr);
	ptr++;
	emitByte(collationPtr, capacityPtr, countPtr, *ptr);
	ptr++;
	emitByte(collationPtr, capacityPtr, countPtr, *ptr);
}

//return the result
unsigned char* collateCompiler(Compiler* compiler, int* size) {
	int capacity = GROW_CAPACITY(0);
	int count = 0;
	unsigned char* collation = ALLOCATE(unsigned char, capacity);

	//embed the header with version information
	emitByte(&collation, &capacity, &count, TOY_VERSION_MAJOR);
	emitByte(&collation, &capacity, &count, TOY_VERSION_MINOR);
	emitByte(&collation, &capacity, &count, TOY_VERSION_PATCH);

	//embed the build info
	if (strlen(TOY_VERSION_BUILD) + count + 1 > capacity) {
		int oldCapacity = capacity;
		capacity = strlen(TOY_VERSION_BUILD) + count + 1; //full header size
		collation = GROW_ARRAY(unsigned char, collation, oldCapacity, capacity);
	}

	memcpy(&collation[count], TOY_VERSION_BUILD, strlen(TOY_VERSION_BUILD));
	count += strlen(TOY_VERSION_BUILD);
	collation[count++] = '\0'; //terminate the build string

	emitByte(&collation, &capacity, &count, OP_SECTION_END); //terminate header

	//embed the data section (first short is the number of literals)
	emitShort(&collation, &capacity, &count, compiler->literalCache.count);

	//emit each literal by type
	for (int i = 0; i < compiler->literalCache.count; i++) {
		//literal Opcode
		// emitShort(&collation, &capacity, &count, OP_LITERAL); //This isn't needed

		//literal type, followed by literal value
		switch(compiler->literalCache.literals[i].type) {
			case LITERAL_NULL:
				emitByte(&collation, &capacity, &count, LITERAL_NULL);
				//null has no following value
			break;

			case LITERAL_BOOLEAN:
				emitByte(&collation, &capacity, &count, LITERAL_BOOLEAN);
				emitByte(&collation, &capacity, &count, AS_BOOLEAN(compiler->literalCache.literals[i]));
			break;

			case LITERAL_INTEGER:
				emitByte(&collation, &capacity, &count, LITERAL_INTEGER);
				emitInt(&collation, &capacity, &count, AS_INTEGER(compiler->literalCache.literals[i]));
			break;

			case LITERAL_FLOAT:
				emitByte(&collation, &capacity, &count, LITERAL_FLOAT);
				emitFloat(&collation, &capacity, &count, AS_FLOAT(compiler->literalCache.literals[i]));
			break;

			case LITERAL_STRING: {
				emitByte(&collation, &capacity, &count, LITERAL_STRING);

				Literal str = compiler->literalCache.literals[i];

				for (int c = 0; c < STRLEN(str); c++) {
					emitByte(&collation, &capacity, &count, AS_STRING(str)[c]);
				}

				emitByte(&collation, &capacity, &count, '\0'); //terminate the string
			}
			break;
		}
	}

	emitByte(&collation, &capacity, &count, OP_SECTION_END); //terminate data

	//code section
	for (int i = 0; i < compiler->count; i++) {
		emitByte(&collation, &capacity, &count, compiler->bytecode[i]);
	}

	emitByte(&collation, &capacity, &count, OP_SECTION_END); //terminate code

	emitByte(&collation, &capacity, &count, OP_EOF); //terminate bytecode

	//finalize
	SHRINK_ARRAY(unsigned char, collation, capacity, count);

	*size = count;

	return collation;	
}