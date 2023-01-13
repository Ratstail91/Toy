#include "compiler.h"

#include "memory.h"

#include "literal.h"
#include "literal_array.h"
#include "literal_dictionary.h"

#include "console_colors.h"

#include <stdio.h>

void initCompiler(Compiler* compiler) {
	initLiteralArray(&compiler->literalCache);
	compiler->bytecode = NULL;
	compiler->capacity = 0;
	compiler->count = 0;
}

//separated out, so it can be recursive
static int writeLiteralTypeToCache(LiteralArray* literalCache, Literal literal) {
	bool shouldFree = false;

	//if it's a compound type, recurse and store the results
	if (AS_TYPE(literal).typeOf == LITERAL_ARRAY || AS_TYPE(literal).typeOf == LITERAL_DICTIONARY) {
		//I don't like storing types in an array, but it's the easiest and most straight forward method
		LiteralArray* store = ALLOCATE(LiteralArray, 1);
		initLiteralArray(store);

		//store the base literal in the store
		pushLiteralArray(store, literal);

		for (int i = 0; i < AS_TYPE(literal).count; i++) {
			//write the values to the cache, and the indexes to the store
			int subIndex = writeLiteralTypeToCache(literalCache, ((Literal*)(AS_TYPE(literal).subtypes))[i]);

			Literal lit = TO_INTEGER_LITERAL(subIndex);
			pushLiteralArray(store, lit);
			freeLiteral(lit);
		}

		//push the store to the cache, tweaking the type
		shouldFree = true;
		literal = TO_ARRAY_LITERAL(store);
		literal.type = LITERAL_TYPE_INTERMEDIATE; //NOTE: tweaking the type usually isn't a good idea
	}

	//optimisation: check if exactly this literal array exists
	int index = findLiteralIndex(literalCache, literal);
	if (index < 0) {
		index = pushLiteralArray(literalCache, literal);
	}

	if (shouldFree) {
		freeLiteral(literal);
	}
	return index;
}

static int writeNodeCompoundToCache(Compiler* compiler, ASTNode* node) {
	int index = -1;

	//for both, stored as an array
	LiteralArray* store = ALLOCATE(LiteralArray, 1);
	initLiteralArray(store);

	//emit an array or a dictionary definition
	if (node->compound.literalType == LITERAL_DICTIONARY) {
		//ensure each literal key and value are in the cache, individually
		for (int i = 0; i < node->compound.count; i++) {
			//keys
			switch(node->compound.nodes[i].pair.left->type) {
				case AST_NODE_LITERAL: {
					//keys are literals
					int key = findLiteralIndex(&compiler->literalCache, node->compound.nodes[i].pair.left->atomic.literal);
					if (key < 0) {
						key = pushLiteralArray(&compiler->literalCache, node->compound.nodes[i].pair.left->atomic.literal);
					}

					Literal literal =  TO_INTEGER_LITERAL(key);
					pushLiteralArray(store, literal);
					freeLiteral(literal);
				}
				break;

				case AST_NODE_COMPOUND: {
						int key = writeNodeCompoundToCache(compiler, node->compound.nodes[i].pair.left);

						Literal literal = TO_INTEGER_LITERAL(key);
						pushLiteralArray(store, literal);
						freeLiteral(literal);
				}
				break;

				default:
					fprintf(stderr, ERROR "[internal] Unrecognized key node type in writeNodeCompoundToCache()\n" RESET);
					return -1;
			}

			//values
			switch(node->compound.nodes[i].pair.right->type) {
				case AST_NODE_LITERAL: {
					//values are literals
					int val = findLiteralIndex(&compiler->literalCache, node->compound.nodes[i].pair.right->atomic.literal);
					if (val < 0) {
						val = pushLiteralArray(&compiler->literalCache, node->compound.nodes[i].pair.right->atomic.literal);
					}

					Literal literal = TO_INTEGER_LITERAL(val);
					pushLiteralArray(store, literal);
					freeLiteral(literal);
				}
				break;

				case AST_NODE_COMPOUND: {
						int val = writeNodeCompoundToCache(compiler, node->compound.nodes[i].pair.right);

						Literal literal = TO_INTEGER_LITERAL(val);
						pushLiteralArray(store, literal);
						freeLiteral(literal);
				}
				break;

				default:
					fprintf(stderr, ERROR "[internal] Unrecognized value node type in writeNodeCompoundToCache()\n" RESET);
					return -1;
			}
		}

		//push the store to the cache, with instructions about how pack it
		Literal literal = TO_DICTIONARY_LITERAL(store);
		literal.type = LITERAL_DICTIONARY_INTERMEDIATE; //god damn it
		index = pushLiteralArray(&compiler->literalCache, literal);
		freeLiteral(literal);
	}
	else if (node->compound.literalType == LITERAL_ARRAY) {
		//ensure each literal value is in the cache, individually
		for (int i = 0; i < node->compound.count; i++) {
			switch(node->compound.nodes[i].type) {
				case AST_NODE_LITERAL: {
					//values
					int val = findLiteralIndex(&compiler->literalCache, node->compound.nodes[i].atomic.literal);
					if (val < 0) {
						val = pushLiteralArray(&compiler->literalCache, node->compound.nodes[i].atomic.literal);
					}

					Literal literal = TO_INTEGER_LITERAL(val);
					pushLiteralArray(store, literal);
					freeLiteral(literal);
				}
				break;

				case AST_NODE_COMPOUND: {
					int val = writeNodeCompoundToCache(compiler, &node->compound.nodes[i]);

					Literal literal = TO_INTEGER_LITERAL(val);
					index = pushLiteralArray(store, literal);
					freeLiteral(literal);
				}
				break;

				default:
					fprintf(stderr, ERROR "[internal] Unrecognized node type in writeNodeCompoundToCache()" RESET);
					return -1;
			}
		}

		//push the store to the cache, with instructions about how pack it
		Literal literal = TO_ARRAY_LITERAL(store);
		index = pushLiteralArray(&compiler->literalCache, literal);
		freeLiteral(literal);
	}
	else {
		fprintf(stderr, ERROR "[internal] Unrecognized compound type in writeNodeCompoundToCache()" RESET);
	}

	return index;
}

static int writeNodeCollectionToCache(Compiler* compiler, ASTNode* node) {
	LiteralArray* store = ALLOCATE(LiteralArray, 1);
	initLiteralArray(store);

	//ensure each literal value is in the cache, individually
	for (int i = 0; i < node->fnCollection.count; i++) {
		switch(node->fnCollection.nodes[i].type) {
			case AST_NODE_VAR_DECL: {
				//write each piece of the declaration to the cache
				int identifierIndex = pushLiteralArray(&compiler->literalCache, node->fnCollection.nodes[i].varDecl.identifier); //store without duplication optimisation
				int typeIndex = writeLiteralTypeToCache(&compiler->literalCache, node->fnCollection.nodes[i].varDecl.typeLiteral);

				Literal identifierLiteral =  TO_INTEGER_LITERAL(identifierIndex);
				pushLiteralArray(store, identifierLiteral);
				freeLiteral(identifierLiteral);

				Literal typeLiteral = TO_INTEGER_LITERAL(typeIndex);
				pushLiteralArray(store, typeLiteral);
				freeLiteral(typeLiteral);
			}
			break;

			case AST_NODE_LITERAL: {
				//write each piece of the declaration to the cache
				int typeIndex = writeLiteralTypeToCache(&compiler->literalCache, node->fnCollection.nodes[i].atomic.literal);

				Literal typeLiteral = TO_INTEGER_LITERAL(typeIndex);
				pushLiteralArray(store, typeLiteral);
				freeLiteral(typeLiteral);
			}
			break;

			default:
				fprintf(stderr, ERROR "[internal] Unrecognized node type in writeNodeCollectionToCache()\n" RESET);
				return -1;
		}
	}

	//store the store
	Literal literal = TO_ARRAY_LITERAL(store);
	int storeIndex = pushLiteralArray(&compiler->literalCache, literal);
	freeLiteral(literal);

	return storeIndex;
}

static int writeLiteralToCompiler(Compiler* compiler, Literal literal) {
	//get the index
	int index = findLiteralIndex(&compiler->literalCache, literal);

	if (index < 0) {
		if (IS_TYPE(literal)) {
			//check for the type literal as value
			index = writeLiteralTypeToCache(&compiler->literalCache, literal);
		}
		else {
			index = pushLiteralArray(&compiler->literalCache, literal);
		}
	}

	//push the literal to the bytecode
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

	return index;
}

//NOTE: jumpOfsets are included, because function arg and return indexes are embedded in the code body i.e. need to include their sizes in the jump
//NOTE: rootNode should NOT include groupings and blocks
static Opcode writeCompilerWithJumps(Compiler* compiler, ASTNode* node, void* breakAddressesPtr, void* continueAddressesPtr, int jumpOffsets, ASTNode* rootNode) {
	//grow if the bytecode space is too small
	if (compiler->count + 32 > compiler->capacity) {
		int oldCapacity = compiler->capacity;

		compiler->capacity = GROW_CAPACITY_FAST(oldCapacity);
		compiler->bytecode = GROW_ARRAY(unsigned char, compiler->bytecode, oldCapacity, compiler->capacity);
	}

	//determine node type
	switch(node->type) {
		case AST_NODE_ERROR: {
			fprintf(stderr, ERROR "[internal] AST_NODEERROR encountered in writeCompilerWithJumps()\n" RESET);
			compiler->bytecode[compiler->count++] = OP_EOF; //1 byte
		}
		break;

		case AST_NODE_LITERAL: {
			writeLiteralToCompiler(compiler, node->atomic.literal);
		}
		break;

		case AST_NODE_UNARY: {
			//pass to the child node, then embed the unary command (print, negate, etc.)
			Opcode override = writeCompilerWithJumps(compiler, node->unary.child, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);

			if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			compiler->bytecode[compiler->count++] = (unsigned char)node->unary.opcode; //1 byte
		}
		break;

		//all infixes come here
		case AST_NODE_BINARY: {
			//pass to the child nodes, then embed the binary command (math, etc.)
			Opcode override = writeCompilerWithJumps(compiler, node->binary.left, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);

			//special case for when indexing and assigning
			if (override != OP_EOF && node->binary.opcode >= OP_VAR_ASSIGN && node->binary.opcode <= OP_VAR_MODULO_ASSIGN) {
				writeCompilerWithJumps(compiler, node->binary.right, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
				compiler->bytecode[compiler->count++] = (unsigned char)OP_INDEX_ASSIGN; //1 byte WARNING: enum trickery
				compiler->bytecode[compiler->count++] = (unsigned char)node->binary.opcode; //1 byte
				return OP_EOF;
			}

			//compensate for... yikes
			if (override != OP_EOF) {
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//return this if...
			Opcode ret = writeCompilerWithJumps(compiler, node->binary.right, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);

			if (node->binary.opcode == OP_INDEX && rootNode->type == AST_NODE_BINARY && rootNode->binary.opcode == OP_VAR_ASSIGN) { //why var assign?
				return OP_INDEX_ASSIGN_INTERMEDIATE;
			}

			//loopy logic - if opcode == index or dot
			if (node->binary.opcode == OP_INDEX || node->binary.opcode == OP_DOT) {
				return node->binary.opcode;
			}

			if (ret != OP_EOF && (node->binary.opcode == OP_VAR_ASSIGN || node->binary.opcode == OP_AND || node->binary.opcode == OP_OR || (node->binary.opcode >= OP_COMPARE_EQUAL && node->binary.opcode <= OP_INVERT))) {
				compiler->bytecode[compiler->count++] = (unsigned char)ret; //1 byte
				ret = OP_EOF; //untangle in this case
			}

			compiler->bytecode[compiler->count++] = (unsigned char)node->binary.opcode; //1 byte

			return ret;
		}
		break;

		case AST_NODE_GROUPING: {
			compiler->bytecode[compiler->count++] = (unsigned char)OP_GROUPING_BEGIN; //1 byte
			Opcode override = writeCompilerWithJumps(compiler, node->grouping.child, breakAddressesPtr, continueAddressesPtr, jumpOffsets, node->grouping.child);
			if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}
			compiler->bytecode[compiler->count++] = (unsigned char)OP_GROUPING_END; //1 byte
		}
		break;

		case AST_NODE_BLOCK: {
			compiler->bytecode[compiler->count++] = (unsigned char)OP_SCOPE_BEGIN; //1 byte

			for (int i = 0; i < node->block.count; i++) {
				Opcode override = writeCompilerWithJumps(compiler, &(node->block.nodes[i]), breakAddressesPtr, continueAddressesPtr, jumpOffsets, &(node->block.nodes[i]));
				if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
					compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
				}
			}

			compiler->bytecode[compiler->count++] = (unsigned char)OP_SCOPE_END; //1 byte
		}
		break;

		case AST_NODE_COMPOUND: {
			int index = writeNodeCompoundToCache(compiler, node);

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

		case AST_NODE_PAIR:
			fprintf(stderr, ERROR "[internal] AST_NODEPAIR encountered in writeCompilerWithJumps()\n" RESET);
			compiler->bytecode[compiler->count++] = OP_EOF; //1 byte
		break;

		case AST_NODE_VAR_DECL: {
			//first, embed the expression (leaves it on the stack)
			Opcode override = writeCompilerWithJumps(compiler, node->varDecl.expression, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
			if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//write each piece of the declaration to the bytecode
			int identifierIndex = findLiteralIndex(&compiler->literalCache, node->varDecl.identifier);
			if (identifierIndex < 0) {
				identifierIndex = pushLiteralArray(&compiler->literalCache, node->varDecl.identifier);
			}

			int typeIndex = writeLiteralTypeToCache(&compiler->literalCache, node->varDecl.typeLiteral);

			//embed the info into the bytecode
			if (identifierIndex >= 256 || typeIndex >= 256) {
				//push a "long" declaration
				compiler->bytecode[compiler->count++] = OP_VAR_DECL_LONG; //1 byte

				*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)identifierIndex; //2 bytes
				compiler->count += sizeof(unsigned short);

				*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)typeIndex; //2 bytes
				compiler->count += sizeof(unsigned short);
			}
			else {
				//push a declaration
				compiler->bytecode[compiler->count++] = OP_VAR_DECL; //1 byte
				compiler->bytecode[compiler->count++] = (unsigned char)identifierIndex; //1 byte
				compiler->bytecode[compiler->count++] = (unsigned char)typeIndex; //1 byte
			}
		}
		break;

		case AST_NODE_FN_DECL: {
			//run a compiler over the function
			Compiler* fnCompiler = ALLOCATE(Compiler, 1);
			initCompiler(fnCompiler);
			writeCompiler(fnCompiler, node->fnDecl.arguments); //can be empty, but not NULL
			writeCompiler(fnCompiler, node->fnDecl.returns); //can be empty, but not NULL
			Opcode override = writeCompilerWithJumps(fnCompiler, node->fnDecl.block, NULL, NULL, -4, rootNode); //can be empty, but not NULL
			if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//create the function in the literal cache (by storing the compiler object)
			Literal fnLiteral = TO_FUNCTION_LITERAL(fnCompiler, 0);
			fnLiteral.type = LITERAL_FUNCTION_INTERMEDIATE; //NOTE: changing type

			//push the name
			int identifierIndex = findLiteralIndex(&compiler->literalCache, node->fnDecl.identifier);
			if (identifierIndex < 0) {
				identifierIndex = pushLiteralArray(&compiler->literalCache, node->fnDecl.identifier);
			}

			//push to function (functions are never equal)
			int fnIndex = pushLiteralArray(&compiler->literalCache, fnLiteral);

			//embed the info into the bytecode
			if (identifierIndex >= 256 || fnIndex >= 256) {
				//push a "long" declaration
				compiler->bytecode[compiler->count++] = OP_FN_DECL_LONG; //1 byte

				*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)identifierIndex; //2 bytes
				compiler->count += sizeof(unsigned short);

				*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)fnIndex; //2 bytes
				compiler->count += sizeof(unsigned short);
			}
			else {
				//push a declaration
				compiler->bytecode[compiler->count++] = OP_FN_DECL; //1 byte
				compiler->bytecode[compiler->count++] = (unsigned char)identifierIndex; //1 byte
				compiler->bytecode[compiler->count++] = (unsigned char)fnIndex; //1 byte
			}
		}
		break;

		case AST_NODE_FN_COLLECTION: {
			//embed these in the bytecode...
			int index = writeNodeCollectionToCache(compiler, node);

			AS_USHORT(compiler->bytecode[compiler->count]) = (unsigned short)index; //2 bytes
			compiler->count += sizeof(unsigned short);
		}
		break;

		case AST_NODE_FN_CALL: {
			//NOTE: assume the function definition/name is above us

			for (int i = 0; i < node->fnCall.arguments->fnCollection.count; i++) { //reverse order, to count from the beginning in the interpreter
				//sub-calls
				if (node->fnCall.arguments->fnCollection.nodes[i].type != AST_NODE_LITERAL) {
					Opcode override = writeCompilerWithJumps(compiler, &node->fnCall.arguments->fnCollection.nodes[i], breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
					if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
						compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
					}
					continue;
				}

				//write each argument to the bytecode
				int argumentsIndex = findLiteralIndex(&compiler->literalCache, node->fnCall.arguments->fnCollection.nodes[i].atomic.literal);
				if (argumentsIndex < 0) {
					argumentsIndex = pushLiteralArray(&compiler->literalCache, node->fnCall.arguments->fnCollection.nodes[i].atomic.literal);
				}

				//push the node opcode to the bytecode
				if (argumentsIndex >= 256) {
					//push a "long" index
					compiler->bytecode[compiler->count++] = OP_LITERAL_LONG; //1 byte

					*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)argumentsIndex; //2 bytes
					compiler->count += sizeof(unsigned short);
				}
				else {
					//push the index
					compiler->bytecode[compiler->count++] = OP_LITERAL; //1 byte
					compiler->bytecode[compiler->count++] = (unsigned char)argumentsIndex; //1 byte
				}
			}

			//push the argument COUNT to the top of the stack
			Literal argumentsCountLiteral =  TO_INTEGER_LITERAL(node->fnCall.argumentCount); //argumentCount is set elsewhere to support dot operator
			int argumentsCountIndex = findLiteralIndex(&compiler->literalCache, argumentsCountLiteral);
			if (argumentsCountIndex < 0) {
				argumentsCountIndex = pushLiteralArray(&compiler->literalCache, argumentsCountLiteral);
			}
			freeLiteral(argumentsCountLiteral);

			if (argumentsCountIndex >= 256) {
				//push a "long" index
				compiler->bytecode[compiler->count++] = OP_LITERAL_LONG; //1 byte

				*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)argumentsCountIndex; //2 bytes
				compiler->count += sizeof(unsigned short);
			}
			else {
				//push the index
				compiler->bytecode[compiler->count++] = OP_LITERAL; //1 byte
				compiler->bytecode[compiler->count++] = (unsigned char)argumentsCountIndex; //1 byte
			}

			//call the function
			//DO NOT call the collection, this is done in binary
		}
		break;

		case AST_NODE_IF: {
			//process the condition
			Opcode override = writeCompilerWithJumps(compiler, node->pathIf.condition, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
			if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//cache the point to insert the jump distance at
			compiler->bytecode[compiler->count++] = OP_IF_FALSE_JUMP; //1 byte
			int jumpToElse = compiler->count;
			compiler->count += sizeof(unsigned short); //2 bytes

			//write the then path
			override = writeCompilerWithJumps(compiler, node->pathIf.thenPath, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
			if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			int jumpToEnd = 0;

			if (node->pathIf.elsePath) {
				//insert jump to end
				compiler->bytecode[compiler->count++] = OP_JUMP; //1 byte
				jumpToEnd = compiler->count;
				compiler->count += sizeof(unsigned short); //2 bytes
			}

			//update the jumpToElse to point here
			AS_USHORT(compiler->bytecode[jumpToElse]) = compiler->count + jumpOffsets; //2 bytes

			if (node->pathIf.elsePath) {
				//if there's an else path, write it and 
				Opcode override = writeCompilerWithJumps(compiler, node->pathIf.elsePath, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
				if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
					compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
				}

				//update the jumpToEnd to point here
				AS_USHORT(compiler->bytecode[jumpToEnd]) = compiler->count + jumpOffsets; //2 bytes
			}
		}
		break;

		case AST_NODE_WHILE: {
			//for breaks and continues
			LiteralArray breakAddresses;
			LiteralArray continueAddresses;

			initLiteralArray(&breakAddresses);
			initLiteralArray(&continueAddresses);

			//cache the jump point
			unsigned short jumpToStart = compiler->count;

			//process the condition
			Opcode override = writeCompilerWithJumps(compiler, node->pathWhile.condition, &breakAddresses, &continueAddresses, jumpOffsets, rootNode);
			if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//if false, jump to end
			compiler->bytecode[compiler->count++] = OP_IF_FALSE_JUMP; //1 byte
			unsigned short jumpToEnd = compiler->count;
			compiler->count += sizeof(unsigned short); //2 bytes

			//write the body
			override = writeCompilerWithJumps(compiler, node->pathWhile.thenPath, &breakAddresses, &continueAddresses, jumpOffsets, rootNode);
			if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//jump to condition
			compiler->bytecode[compiler->count++] = OP_JUMP; //1 byte
			AS_USHORT(compiler->bytecode[compiler->count]) = jumpToStart + jumpOffsets;
			compiler->count += sizeof(unsigned short); //2 bytes

			//jump from condition
			AS_USHORT(compiler->bytecode[jumpToEnd]) = (unsigned short)compiler->count + jumpOffsets;

			//set the breaks and continues
			for (int i = 0; i < breakAddresses.count; i++) {
				int point = AS_INTEGER(breakAddresses.literals[i]);
				AS_USHORT(compiler->bytecode[point]) = (unsigned short)compiler->count + jumpOffsets;
			}

			for (int i = 0; i < continueAddresses.count; i++) {
				int point = AS_INTEGER(continueAddresses.literals[i]);
				AS_USHORT(compiler->bytecode[point]) = jumpToStart + jumpOffsets;
			}

			//clear the stack after use
			compiler->bytecode[compiler->count++] = OP_POP_STACK; //1 byte

			//cleanup
			freeLiteralArray(&breakAddresses);
			freeLiteralArray(&continueAddresses);
		}
		break;

		case AST_NODE_FOR: {
			//for breaks and continues
			LiteralArray breakAddresses;
			LiteralArray continueAddresses;

			initLiteralArray(&breakAddresses);
			initLiteralArray(&continueAddresses);

			compiler->bytecode[compiler->count++] = OP_SCOPE_BEGIN; //1 byte

			//initial setup
			Opcode override = writeCompilerWithJumps(compiler, node->pathFor.preClause, &breakAddresses, &continueAddresses, jumpOffsets, rootNode);
			if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//conditional
			unsigned short jumpToStart = compiler->count;
			override = writeCompilerWithJumps(compiler, node->pathFor.condition, &breakAddresses, &continueAddresses, jumpOffsets, rootNode);
			if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//if false jump to end
			compiler->bytecode[compiler->count++] = OP_IF_FALSE_JUMP; //1 byte
			unsigned short jumpToEnd = compiler->count;
			compiler->count += sizeof(unsigned short); //2 bytes

			//write the body
			compiler->bytecode[compiler->count++] = OP_SCOPE_BEGIN; //1 byte
			override = writeCompilerWithJumps(compiler, node->pathFor.thenPath, &breakAddresses, &continueAddresses, jumpOffsets, rootNode);
			if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}
			compiler->bytecode[compiler->count++] = OP_SCOPE_END; //1 byte

			//for-breaks actually jump to the bottom
			int jumpToIncrement = compiler->count;

			//evaluate third clause, restart
			override = writeCompilerWithJumps(compiler, node->pathFor.postClause, &breakAddresses, &continueAddresses, jumpOffsets, rootNode);
			if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			compiler->bytecode[compiler->count++] = OP_JUMP; //1 byte
			AS_USHORT(compiler->bytecode[compiler->count]) = jumpToStart + jumpOffsets;
			compiler->count += sizeof(unsigned short); //2 bytes

			AS_USHORT(compiler->bytecode[jumpToEnd]) = compiler->count + jumpOffsets;

			compiler->bytecode[compiler->count++] = OP_SCOPE_END; //1 byte

			//set the breaks and continues
			for (int i = 0; i < breakAddresses.count; i++) {
				int point = AS_INTEGER(breakAddresses.literals[i]);
				AS_USHORT(compiler->bytecode[point]) = compiler->count + jumpOffsets;
			}

			for (int i = 0; i < continueAddresses.count; i++) {
				int point = AS_INTEGER(continueAddresses.literals[i]);
				AS_USHORT(compiler->bytecode[point]) = jumpToIncrement + jumpOffsets;
			}

			//clear the stack after use
			compiler->bytecode[compiler->count++] = OP_POP_STACK; //1 byte

			//cleanup
			freeLiteralArray(&breakAddresses);
			freeLiteralArray(&continueAddresses);
		}
		break;

		case AST_NODE_BREAK: {
			if (!breakAddressesPtr) {
				fprintf(stderr, ERROR "ERROR: Can't place a break statement here\n" RESET);
				break;
			}

			//insert into bytecode
			compiler->bytecode[compiler->count++] = OP_JUMP; //1 byte

			//push to the breakAddresses array
			Literal literal = TO_INTEGER_LITERAL(compiler->count);
			pushLiteralArray((LiteralArray*)breakAddressesPtr, literal);
			freeLiteral(literal);

			compiler->count += sizeof(unsigned short); //2 bytes
		}
		break;

		case AST_NODE_CONTINUE: {
			if (!continueAddressesPtr) {
				fprintf(stderr, ERROR "ERROR: Can't place a continue statement here\n" RESET);
				break;
			}

			//insert into bytecode
			compiler->bytecode[compiler->count++] = OP_JUMP; //1 byte

			//push to the continueAddresses array
			Literal literal = TO_INTEGER_LITERAL(compiler->count);
			pushLiteralArray((LiteralArray*)continueAddressesPtr, literal);
			freeLiteral(literal);

			compiler->count += sizeof(unsigned short); //2 bytes
		}
		break;

		case AST_NODE_FN_RETURN: {
			//read each returned literal onto the stack, and return the number of values to return
			for (int i = 0; i < node->returns.returns->fnCollection.count; i++) {
				Opcode override = writeCompilerWithJumps(compiler, &node->returns.returns->fnCollection.nodes[i], breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
				if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
					compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
				}
			}

			//push the return, with the number of literals
			compiler->bytecode[compiler->count++] = OP_FN_RETURN; //1 byte

			*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)(node->returns.returns->fnCollection.count); //2 bytes
			compiler->count += sizeof(unsigned short);
		}
		break;

		case AST_NODE_PREFIX_INCREMENT: {
			//push the literal to the stack (twice: add + assign)
			writeLiteralToCompiler(compiler, node->prefixIncrement.identifier);
			writeLiteralToCompiler(compiler, node->prefixIncrement.identifier);

			//push the increment / decrement
			Literal increment = TO_INTEGER_LITERAL(1);
			writeLiteralToCompiler(compiler, increment);

			//push the add opcode
			compiler->bytecode[compiler->count++] = (unsigned char)OP_ADDITION; //1 byte

			//push the assign
			compiler->bytecode[compiler->count++] = (unsigned char)OP_VAR_ASSIGN; //1 byte

			//leave the result on the stack
			writeLiteralToCompiler(compiler, node->prefixIncrement.identifier);
			compiler->bytecode[compiler->count++] = (unsigned char)OP_LITERAL_RAW; //1 byte
		}
		break;

		case AST_NODE_PREFIX_DECREMENT: {
			//push the literal to the stack (twice: add + assign)
			writeLiteralToCompiler(compiler, node->prefixDecrement.identifier);
			writeLiteralToCompiler(compiler, node->prefixDecrement.identifier);

			//push the increment / decrement
			Literal increment = TO_INTEGER_LITERAL(1);
			writeLiteralToCompiler(compiler, increment);

			//push the subtract opcode
			compiler->bytecode[compiler->count++] = (unsigned char)OP_SUBTRACTION; //1 byte

			//push the assign
			compiler->bytecode[compiler->count++] = (unsigned char)OP_VAR_ASSIGN; //1 byte

			//leave the result on the stack
			writeLiteralToCompiler(compiler, node->prefixDecrement.identifier);
			compiler->bytecode[compiler->count++] = (unsigned char)OP_LITERAL_RAW; //1 byte
		}
		break;

		case AST_NODE_POSTFIX_INCREMENT: {
			//push the identifier's VALUE to the stack
			writeLiteralToCompiler(compiler, node->postfixIncrement.identifier);
			compiler->bytecode[compiler->count++] = (unsigned char)OP_LITERAL_RAW; //1 byte

			//push the identifier (twice: add + assign)
			writeLiteralToCompiler(compiler, node->postfixIncrement.identifier);
			writeLiteralToCompiler(compiler, node->postfixIncrement.identifier);

			//push the increment / decrement
			Literal increment = TO_INTEGER_LITERAL(1);
			writeLiteralToCompiler(compiler, increment);

			//push the add opcode
			compiler->bytecode[compiler->count++] = (unsigned char)OP_ADDITION; //1 byte

			//push the assign
			compiler->bytecode[compiler->count++] = (unsigned char)OP_VAR_ASSIGN; //1 byte
		}
		break;

		case AST_NODE_POSTFIX_DECREMENT: {
			//push the identifier's VALUE to the stack
			writeLiteralToCompiler(compiler, node->postfixDecrement.identifier);
			compiler->bytecode[compiler->count++] = (unsigned char)OP_LITERAL_RAW; //1 byte

			//push the identifier (twice: add + assign)
			writeLiteralToCompiler(compiler, node->postfixDecrement.identifier);
			writeLiteralToCompiler(compiler, node->postfixDecrement.identifier);

			//push the increment / decrement
			Literal increment = TO_INTEGER_LITERAL(1);
			writeLiteralToCompiler(compiler, increment);

			//push the subtract opcode
			compiler->bytecode[compiler->count++] = (unsigned char)OP_SUBTRACTION; //1 byte

			//push the assign
			compiler->bytecode[compiler->count++] = (unsigned char)OP_VAR_ASSIGN; //1 byte
		}
		break;

		case AST_NODE_IMPORT: {
			//push the identifier, and the alias
			writeLiteralToCompiler(compiler, node->import.identifier);
			writeLiteralToCompiler(compiler, node->import.alias);

			//push the import opcode
			compiler->bytecode[compiler->count++] = (unsigned char)OP_IMPORT; //1 byte
		}
		break;

		case AST_NODE_INDEX: {
			//pass to the child nodes, then embed the opcode

			//first
			if (!node->index.first) {
				writeLiteralToCompiler(compiler, TO_NULL_LITERAL);
			}
			else {
				Opcode override = writeCompilerWithJumps(compiler, node->index.first, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
				if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
					compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
				}
			}

			//second
			if (!node->index.second) {
				writeLiteralToCompiler(compiler, TO_NULL_LITERAL);
			}
			else {
				Opcode override = writeCompilerWithJumps(compiler, node->index.second, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
				if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
					compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
				}
			}

			//third
			if (!node->index.third) {
				writeLiteralToCompiler(compiler, TO_NULL_LITERAL);
			}
			else {
				Opcode override = writeCompilerWithJumps(compiler, node->index.third, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
				if (override != OP_EOF) {//compensate for indexing & dot notation being screwy
					compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
				}
			}

			// compiler->bytecode[compiler->count++] = (unsigned char)OP_INDEX; //1 byte

			return OP_INDEX_ASSIGN; //override binary's instruction IF it is assign
		}
		break;
	}

	return OP_EOF;
}

void writeCompiler(Compiler* compiler, ASTNode* node) {
	Opcode op = writeCompilerWithJumps(compiler, node, NULL, NULL, 0, node); //pass in "node" as the root node

	if (op != OP_EOF) {//compensate for indexing & dot notation being screwy
		compiler->bytecode[compiler->count++] = (unsigned char)op; //1 byte
	}
}

void freeCompiler(Compiler* compiler) {
	freeLiteralArray(&compiler->literalCache);
	FREE_ARRAY(unsigned char, compiler->bytecode, compiler->capacity);
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
static unsigned char* collateCompilerHeaderOpt(Compiler* compiler, int* size, bool embedHeader) {
	int capacity = GROW_CAPACITY(0);
	int count = 0;
	unsigned char* collation = ALLOCATE(unsigned char, capacity);

	//for the function-section at the end of the main-collation
	int fnIndex = 0; //counts up for each fn
	int fnCapacity = GROW_CAPACITY(0);
	int fnCount = 0;
	unsigned char* fnCollation = ALLOCATE(unsigned char, fnCapacity);

	if (embedHeader) {
		//embed the header with version information
		emitByte(&collation, &capacity, &count, TOY_VERSION_MAJOR);
		emitByte(&collation, &capacity, &count, TOY_VERSION_MINOR);
		emitByte(&collation, &capacity, &count, TOY_VERSION_PATCH);

		//embed the build info
		if ((int)strlen(TOY_VERSION_BUILD) + count + 1 > capacity) {
			int oldCapacity = capacity;
			capacity = strlen(TOY_VERSION_BUILD) + count + 1; //full header size
			collation = GROW_ARRAY(unsigned char, collation, oldCapacity, capacity);
		}

		memcpy(&collation[count], TOY_VERSION_BUILD, strlen(TOY_VERSION_BUILD));
		count += strlen(TOY_VERSION_BUILD);
		collation[count++] = '\0'; //terminate the build string

		emitByte(&collation, &capacity, &count, OP_SECTION_END); //terminate header
	}

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

				for (int c = 0; c < AS_STRING(str)->length; c++) {
					emitByte(&collation, &capacity, &count, toCString(AS_STRING(str))[c]);
				}

				emitByte(&collation, &capacity, &count, '\0'); //terminate the string
			}
			break;

			case LITERAL_ARRAY: {
				emitByte(&collation, &capacity, &count, LITERAL_ARRAY);

				LiteralArray* ptr = AS_ARRAY(compiler->literalCache.literals[i]);

				//length of the array, as a short
				emitShort(&collation, &capacity, &count, ptr->count);

				//each element of the array
				for (int i = 0; i < ptr->count; i++) {
					emitShort(&collation, &capacity, &count, (unsigned short)AS_INTEGER(ptr->literals[i])); //shorts representing the indexes of the values
				}
			}
			break;

			case LITERAL_DICTIONARY_INTERMEDIATE: {
				emitByte(&collation, &capacity, &count, LITERAL_DICTIONARY);

				LiteralArray* ptr = AS_ARRAY(compiler->literalCache.literals[i]); //used an array for storage above

				//length of the array, as a short
				emitShort(&collation, &capacity, &count, ptr->count); //count is the array size, NOT the dictionary size

				//each element of the array
				for (int i = 0; i < ptr->count; i++) {
					emitShort(&collation, &capacity, &count, (unsigned short)AS_INTEGER(ptr->literals[i])); //shorts representing the indexes of the values
				}
			}
			break;

			case LITERAL_FUNCTION_INTERMEDIATE: {
				//extract the compiler
				Literal fn = compiler->literalCache.literals[i];
				void* fnCompiler = AS_FUNCTION(fn).bytecode; //store the compiler here for now

				//collate the function into bytecode (without header)
				int size = 0;
				unsigned char* bytes = collateCompilerHeaderOpt((Compiler*)fnCompiler, &size, false);

				//emit how long this section is, +1 for ending mark
				emitShort(&fnCollation, &fnCapacity, &fnCount, (unsigned short)size + 1);

				//write the fn to the fn collation
				for (int i = 0; i < size; i++) {
					emitByte(&fnCollation, &fnCapacity, &fnCount, bytes[i]);
				}

				emitByte(&fnCollation, &fnCapacity, &fnCount, OP_FN_END); //for marking the correct end-point of the function

				//embed the reference to the function implementation into the current collation (to be extracted later)
				emitByte(&collation, &capacity, &count, LITERAL_FUNCTION);
				emitShort(&collation, &capacity, &count, (unsigned short)(fnIndex++));

				freeCompiler((Compiler*)fnCompiler);
				FREE(compiler, fnCompiler);
				FREE_ARRAY(unsigned char, bytes, size);
			}
			break;

			case LITERAL_IDENTIFIER: {
				emitByte(&collation, &capacity, &count, LITERAL_IDENTIFIER);

				Literal identifier = compiler->literalCache.literals[i];

				for (int c = 0; c < AS_IDENTIFIER(identifier)->length; c++) {
					emitByte(&collation, &capacity, &count, toCString(AS_IDENTIFIER(identifier))[c]);
				}

				emitByte(&collation, &capacity, &count, '\0'); //terminate the string
			}
			break;

			case LITERAL_TYPE: {
				//push a raw type
				emitByte(&collation, &capacity, &count, LITERAL_TYPE);

				Literal typeLiteral = compiler->literalCache.literals[i];

				//what type this literal represents
				emitByte(&collation, &capacity, &count, AS_TYPE(typeLiteral).typeOf);
				emitByte(&collation, &capacity, &count, AS_TYPE(typeLiteral).constant); //if it's constant
			}
			break;

			case LITERAL_TYPE_INTERMEDIATE: {
				emitByte(&collation, &capacity, &count, LITERAL_TYPE_INTERMEDIATE);

				LiteralArray* ptr = AS_ARRAY(compiler->literalCache.literals[i]); //used an array for storage above

				//the base literal
				Literal typeLiteral = copyLiteral(ptr->literals[0]);

				//what type this literal represents
				emitByte(&collation, &capacity, &count, AS_TYPE(typeLiteral).typeOf);
				emitByte(&collation, &capacity, &count, AS_TYPE(typeLiteral).constant); //if it's constant

				//each element of the array, If they exist, representing sub-types already in the cache
				if (AS_TYPE(typeLiteral).typeOf == LITERAL_ARRAY || AS_TYPE(typeLiteral).typeOf == LITERAL_DICTIONARY) {
					//the type will represent how many to expect in the array
					for (int i = 1; i < ptr->count; i++) {
						emitShort(&collation, &capacity, &count, (unsigned short)AS_INTEGER(ptr->literals[i])); //shorts representing the indexes of the types
					}
				}

				freeLiteral(typeLiteral);
			}
			break;

			default:
				fprintf(stderr, ERROR "[internal] Unknown literal type encountered within literal cache: %d\n" RESET, compiler->literalCache.literals[i].type);
				return NULL;
		}
	}

	emitByte(&collation, &capacity, &count, OP_SECTION_END); //terminate data

	//embed the function section (beginning with function count, size)
	emitShort(&collation, &capacity, &count, fnIndex);
	emitShort(&collation, &capacity, &count, fnCount);

	for (int i = 0; i < fnCount; i++) {
		emitByte(&collation, &capacity, &count, fnCollation[i]);
	}

	emitByte(&collation, &capacity, &count, OP_SECTION_END); //terminate function section

	FREE_ARRAY(unsigned char, fnCollation, fnCapacity); //clear the function stuff

	//code section
	for (int i = 0; i < compiler->count; i++) {
		emitByte(&collation, &capacity, &count, compiler->bytecode[i]);
	}

	emitByte(&collation, &capacity, &count, OP_SECTION_END); //terminate code

	emitByte(&collation, &capacity, &count, OP_EOF); //terminate bytecode

	//finalize
	collation = SHRINK_ARRAY(unsigned char, collation, capacity, count);

	*size = count;

	return collation;
}

unsigned char* collateCompiler(Compiler* compiler, int* size) {
	return collateCompilerHeaderOpt(compiler, size, true);
}
