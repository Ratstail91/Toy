#include "toy_compiler.h"

#include "toy_memory.h"

#include "toy_literal.h"
#include "toy_literal_array.h"
#include "toy_literal_dictionary.h"

#include "toy_console_colors.h"

#include <stdio.h>

void Toy_initCompiler(Toy_Compiler* compiler) {
	Toy_initLiteralArray(&compiler->literalCache);
	compiler->bytecode = NULL;
	compiler->capacity = 0;
	compiler->count = 0;
	compiler->panic = false;
}

//separated out, so it can be recursive
static int writeLiteralTypeToCache(Toy_LiteralArray* literalCache, Toy_Literal literal) {
	bool shouldFree = false;

	//if it's a compound type, recurse and store the results
	if (TOY_AS_TYPE(literal).typeOf == TOY_LITERAL_ARRAY || TOY_AS_TYPE(literal).typeOf == TOY_LITERAL_DICTIONARY) {
		//I don't like storing types in an array, but it's the easiest and most straight forward method
		Toy_LiteralArray* store = TOY_ALLOCATE(Toy_LiteralArray, 1);
		Toy_initLiteralArray(store);

		//store the base literal in the store
		Toy_pushLiteralArray(store, literal);

		for (int i = 0; i < TOY_AS_TYPE(literal).count; i++) {
			//write the values to the cache, and the indexes to the store
			int subIndex = writeLiteralTypeToCache(literalCache, ((Toy_Literal*)(TOY_AS_TYPE(literal).subtypes))[i]);

			Toy_Literal lit = TOY_TO_INTEGER_LITERAL(subIndex);
			Toy_pushLiteralArray(store, lit);
			Toy_freeLiteral(lit);
		}

		//push the store to the cache, tweaking the type
		shouldFree = true;
		literal = TOY_TO_ARRAY_LITERAL(store);
		literal.type = TOY_LITERAL_TYPE_INTERMEDIATE; //NOTE: tweaking the type usually isn't a good idea
	}

	//optimisation: check if exactly this literal array exists
	int index = Toy_findLiteralIndex(literalCache, literal);
	if (index < 0) {
		index = Toy_pushLiteralArray(literalCache, literal);
	}

	if (shouldFree) {
		Toy_freeLiteral(literal);
	}
	return index;
}

static int writeNodeCompoundToCache(Toy_Compiler* compiler, Toy_ASTNode* node) {
	int index = -1;

	//for both, stored as an array
	Toy_LiteralArray* store = TOY_ALLOCATE(Toy_LiteralArray, 1);
	Toy_initLiteralArray(store);

	//emit an array or a dictionary definition
	if (node->compound.literalType == TOY_LITERAL_DICTIONARY) {
		//ensure each literal key and value are in the cache, individually
		for (int i = 0; i < node->compound.count; i++) {
			//keys
			switch(node->compound.nodes[i].pair.left->type) {
				case TOY_AST_NODE_LITERAL: {
					//keys are literals
					int key = Toy_findLiteralIndex(&compiler->literalCache, node->compound.nodes[i].pair.left->atomic.literal);
					if (key < 0) {
						key = Toy_pushLiteralArray(&compiler->literalCache, node->compound.nodes[i].pair.left->atomic.literal);
					}

					Toy_Literal literal =  TOY_TO_INTEGER_LITERAL(key);
					Toy_pushLiteralArray(store, literal);
					Toy_freeLiteral(literal);
				}
				break;

				case TOY_AST_NODE_COMPOUND: {
						int key = writeNodeCompoundToCache(compiler, node->compound.nodes[i].pair.left);

						Toy_Literal literal = TOY_TO_INTEGER_LITERAL(key);
						Toy_pushLiteralArray(store, literal);
						Toy_freeLiteral(literal);
				}
				break;

				default:
					fprintf(stderr, TOY_CC_ERROR "[internal] Unrecognized key node type in writeNodeCompoundToCache()\n" TOY_CC_RESET);
					return -1;
			}

			//values
			switch(node->compound.nodes[i].pair.right->type) {
				case TOY_AST_NODE_LITERAL: {
					//values are literals
					int val = Toy_findLiteralIndex(&compiler->literalCache, node->compound.nodes[i].pair.right->atomic.literal);
					if (val < 0) {
						val = Toy_pushLiteralArray(&compiler->literalCache, node->compound.nodes[i].pair.right->atomic.literal);
					}

					Toy_Literal literal = TOY_TO_INTEGER_LITERAL(val);
					Toy_pushLiteralArray(store, literal);
					Toy_freeLiteral(literal);
				}
				break;

				case TOY_AST_NODE_COMPOUND: {
						int val = writeNodeCompoundToCache(compiler, node->compound.nodes[i].pair.right);

						Toy_Literal literal = TOY_TO_INTEGER_LITERAL(val);
						Toy_pushLiteralArray(store, literal);
						Toy_freeLiteral(literal);
				}
				break;

				default:
					fprintf(stderr, TOY_CC_ERROR "[internal] Unrecognized value node type in writeNodeCompoundToCache()\n" TOY_CC_RESET);
					return -1;
			}
		}

		//push the store to the cache, with instructions about how pack it
		Toy_Literal literal = TOY_TO_DICTIONARY_LITERAL(store);
		literal.type = TOY_LITERAL_DICTIONARY_INTERMEDIATE; //god damn it - nested in a dictionary
		index = Toy_pushLiteralArray(&compiler->literalCache, literal);
		Toy_freeLiteral(literal);
	}

	else if (node->compound.literalType == TOY_LITERAL_ARRAY) {
		//ensure each literal value is in the cache, individually
		for (int i = 0; i < node->compound.count; i++) {
			switch(node->compound.nodes[i].type) {
				case TOY_AST_NODE_LITERAL: {
					//values
					int val = Toy_findLiteralIndex(&compiler->literalCache, node->compound.nodes[i].atomic.literal);
					if (val < 0) {
						val = Toy_pushLiteralArray(&compiler->literalCache, node->compound.nodes[i].atomic.literal);
					}

					Toy_Literal literal = TOY_TO_INTEGER_LITERAL(val);
					Toy_pushLiteralArray(store, literal);
					Toy_freeLiteral(literal);
				}
				break;

				case TOY_AST_NODE_COMPOUND: {
					int val = writeNodeCompoundToCache(compiler, &node->compound.nodes[i]);

					Toy_Literal literal = TOY_TO_INTEGER_LITERAL(val);
					index = Toy_pushLiteralArray(store, literal);
					Toy_freeLiteral(literal);
				}
				break;

				default:
					fprintf(stderr, TOY_CC_ERROR "[internal] Unrecognized node type in writeNodeCompoundToCache()\n" TOY_CC_RESET);
					return -1;
			}
		}

		//push the store to the cache, with instructions about how pack it
		Toy_Literal literal = TOY_TO_ARRAY_LITERAL(store);
		literal.type = TOY_LITERAL_ARRAY_INTERMEDIATE; //god damn it - nested in an array
		index = Toy_pushLiteralArray(&compiler->literalCache, literal);
		Toy_freeLiteral(literal);
	}
	else {
		fprintf(stderr, TOY_CC_ERROR "[internal] Unrecognized compound type in writeNodeCompoundToCache()\n" TOY_CC_RESET);
		return -1;
	}

	return index;
}

static int writeNodeCollectionToCache(Toy_Compiler* compiler, Toy_ASTNode* node) {
	Toy_LiteralArray* store = TOY_ALLOCATE(Toy_LiteralArray, 1);
	Toy_initLiteralArray(store);

	//ensure each literal value is in the cache, individually
	for (int i = 0; i < node->fnCollection.count; i++) {
		switch(node->fnCollection.nodes[i].type) {
			case TOY_AST_NODE_VAR_DECL: {
				//write each piece of the declaration to the cache
				int identifierIndex = Toy_pushLiteralArray(&compiler->literalCache, node->fnCollection.nodes[i].varDecl.identifier); //store without duplication optimisation
				int typeIndex = writeLiteralTypeToCache(&compiler->literalCache, node->fnCollection.nodes[i].varDecl.typeLiteral);

				Toy_Literal identifierLiteral =  TOY_TO_INTEGER_LITERAL(identifierIndex);
				Toy_pushLiteralArray(store, identifierLiteral);
				Toy_freeLiteral(identifierLiteral);

				Toy_Literal typeLiteral = TOY_TO_INTEGER_LITERAL(typeIndex);
				Toy_pushLiteralArray(store, typeLiteral);
				Toy_freeLiteral(typeLiteral);
			}
			break;

			case TOY_AST_NODE_LITERAL: {
				//write each piece of the declaration to the cache
				int typeIndex = writeLiteralTypeToCache(&compiler->literalCache, node->fnCollection.nodes[i].atomic.literal);

				Toy_Literal typeLiteral = TOY_TO_INTEGER_LITERAL(typeIndex);
				Toy_pushLiteralArray(store, typeLiteral);
				Toy_freeLiteral(typeLiteral);
			}
			break;

			default:
				fprintf(stderr, TOY_CC_ERROR "[internal] Unrecognized node type in writeNodeCollectionToCache()\n" TOY_CC_RESET);
				return -1;
		}
	}

	//store the store
	Toy_Literal literal = TOY_TO_ARRAY_LITERAL(store);
	int storeIndex = Toy_pushLiteralArray(&compiler->literalCache, literal);
	Toy_freeLiteral(literal);

	return storeIndex;
}

static int writeLiteralToCompiler(Toy_Compiler* compiler, Toy_Literal literal) {
	//get the index
	int index = Toy_findLiteralIndex(&compiler->literalCache, literal);

	if (index < 0) {
		if (TOY_IS_TYPE(literal)) {
			//check for the type literal as value
			index = writeLiteralTypeToCache(&compiler->literalCache, literal);
		}
		else {
			index = Toy_pushLiteralArray(&compiler->literalCache, literal);
		}
	}

	//push the literal to the bytecode
	if (index >= 256) {
		//push a "long" index
		compiler->bytecode[compiler->count++] = TOY_OP_LITERAL_LONG; //1 byte
		memcpy(compiler->bytecode + compiler->count, &index, sizeof(unsigned short)); //2 bytes

		compiler->count += sizeof(unsigned short);
	}
	else {
		//push the index
		compiler->bytecode[compiler->count++] = TOY_OP_LITERAL; //1 byte
		compiler->bytecode[compiler->count++] = (unsigned char)index; //1 byte
	}

	return index;
}

//NOTE: jumpOfsets are included, because function arg and return indexes are embedded in the code body i.e. need to include their sizes in the jump
//NOTE: rootNode should NOT include groupings and blocks
static Toy_Opcode Toy_writeCompilerWithJumps(Toy_Compiler* compiler, Toy_ASTNode* node, void* breakAddressesPtr, void* continueAddressesPtr, int jumpOffsets, Toy_ASTNode* rootNode) {
	//grow if the bytecode space is too small
	if (compiler->count + 32 > compiler->capacity) {
		int oldCapacity = compiler->capacity;

		compiler->capacity = TOY_GROW_CAPACITY_FAST(oldCapacity);
		compiler->bytecode = TOY_GROW_ARRAY(unsigned char, compiler->bytecode, oldCapacity, compiler->capacity);
	}

	//determine node type
	switch(node->type) {
		case TOY_AST_NODE_ERROR: {
			fprintf(stderr, TOY_CC_ERROR "[internal] TOY_AST_NODEERROR encountered in Toy_writeCompilerWithJumps()\n" TOY_CC_RESET);
			compiler->bytecode[compiler->count++] = TOY_OP_EOF; //1 byte
		}
		break;

		case TOY_AST_NODE_LITERAL: {
			writeLiteralToCompiler(compiler, node->atomic.literal);
		}
		break;

		case TOY_AST_NODE_UNARY: {
			//pass to the child node, then embed the unary command (print, negate, etc.)
			Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, node->unary.child, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);

			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			compiler->bytecode[compiler->count++] = (unsigned char)node->unary.opcode; //1 byte
		}
		break;

		//all infixes come here
		case TOY_AST_NODE_BINARY: {
			//pass to the child nodes, then embed the binary command (math, etc.)
			Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, node->binary.left, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);

			//special case for when indexing and assigning
			if (override != TOY_OP_EOF && node->binary.opcode >= TOY_OP_VAR_ASSIGN && node->binary.opcode <= TOY_OP_VAR_MODULO_ASSIGN) {
				Toy_writeCompilerWithJumps(compiler, node->binary.right, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);

				//Special case if there's an index on both sides of the sign, just set it as indexing
				if (node->binary.left->type == TOY_AST_NODE_BINARY && node->binary.right->type == TOY_AST_NODE_BINARY && node->binary.left->binary.opcode == TOY_OP_INDEX && node->binary.right->binary.opcode == TOY_OP_INDEX) {
					compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_INDEX;
				}

				compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_INDEX_ASSIGN; //1 byte WARNING: enum trickery
				compiler->bytecode[compiler->count++] = (unsigned char)node->binary.opcode; //1 byte
				return TOY_OP_EOF;
			}

			//compensate for... yikes
			if (override != TOY_OP_EOF) {
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//return this if...
			Toy_Opcode ret = Toy_writeCompilerWithJumps(compiler, node->binary.right, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);

			if (node->binary.opcode == TOY_OP_INDEX && rootNode->type == TOY_AST_NODE_BINARY && (rootNode->binary.opcode >= TOY_OP_VAR_ASSIGN && rootNode->binary.opcode <= TOY_OP_VAR_MODULO_ASSIGN) && rootNode->binary.right != node) { //range-based check for assignment type; make sure the index is on the left of the assignment symbol
				return TOY_OP_INDEX_ASSIGN_INTERMEDIATE;
			}

			//loopy logic - if opcode == index or dot
			if (node->binary.opcode == TOY_OP_INDEX || node->binary.opcode == TOY_OP_DOT) {
				return node->binary.opcode;
			}

			if (ret != TOY_OP_EOF && (node->binary.opcode == TOY_OP_VAR_ASSIGN || node->binary.opcode == TOY_OP_AND || node->binary.opcode == TOY_OP_OR || (node->binary.opcode >= TOY_OP_COMPARE_EQUAL && node->binary.opcode <= TOY_OP_INVERT))) {
				compiler->bytecode[compiler->count++] = (unsigned char)ret; //1 byte
				ret = TOY_OP_EOF; //untangle in this case
			}

			compiler->bytecode[compiler->count++] = (unsigned char)node->binary.opcode; //1 byte

			return ret;
		}
		break;

		case TOY_AST_NODE_TERNARY: {
			// TODO: a ?: b;

			//process the condition
			Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, node->ternary.condition, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//cache the point to insert the jump distance at
			compiler->bytecode[compiler->count++] = TOY_OP_IF_FALSE_JUMP; //1 byte
			int jumpToElse = compiler->count;
			compiler->count += sizeof(unsigned short); //2 bytes

			//write the then path
			override = Toy_writeCompilerWithJumps(compiler, node->pathIf.thenPath, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			int jumpToEnd = 0;

			//insert jump to end
			compiler->bytecode[compiler->count++] = TOY_OP_JUMP; //1 byte
			jumpToEnd = compiler->count;
			compiler->count += sizeof(unsigned short); //2 bytes

			//update the jumpToElse to point here
			unsigned short tmpVal = compiler->count + jumpOffsets;
			memcpy(compiler->bytecode + jumpToElse, &tmpVal, sizeof(tmpVal)); //2 bytes

			//write the else path
			Toy_Opcode override2 = Toy_writeCompilerWithJumps(compiler, node->pathIf.elsePath, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
			if (override2 != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override2; //1 byte
			}

			//update the jumpToEnd to point here
			tmpVal = compiler->count + jumpOffsets;
			memcpy(compiler->bytecode + jumpToEnd, &tmpVal, sizeof(tmpVal)); //2 bytes
		}
		break;

		case TOY_AST_NODE_GROUPING: {
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_GROUPING_BEGIN; //1 byte
			Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, node->grouping.child, breakAddressesPtr, continueAddressesPtr, jumpOffsets, node->grouping.child);
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_GROUPING_END; //1 byte
		}
		break;

		case TOY_AST_NODE_BLOCK: {
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_SCOPE_BEGIN; //1 byte

			for (int i = 0; i < node->block.count; i++) {
				Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, &(node->block.nodes[i]), breakAddressesPtr, continueAddressesPtr, jumpOffsets, &(node->block.nodes[i]));
				if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
					compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
				}
			}

			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_SCOPE_END; //1 byte
		}
		break;

		case TOY_AST_NODE_COMPOUND: {
			int index = writeNodeCompoundToCache(compiler, node);

			if (index < 0) {
				compiler->panic = true;
				return TOY_OP_EOF;
			}

			//push the node opcode to the bytecode
			if (index >= 256) {
				//push a "long" index
				compiler->bytecode[compiler->count++] = TOY_OP_LITERAL_LONG; //1 byte
				memcpy(compiler->bytecode + compiler->count, &index, sizeof(unsigned short));

				compiler->count += sizeof(unsigned short);
			}
			else {
				//push the index
				compiler->bytecode[compiler->count++] = TOY_OP_LITERAL; //1 byte
				compiler->bytecode[compiler->count++] = (unsigned char)index; //1 byte
			}
		}
		break;

		case TOY_AST_NODE_PAIR:
			fprintf(stderr, TOY_CC_ERROR "[internal] TOY_AST_NODEPAIR encountered in Toy_writeCompilerWithJumps()\n" TOY_CC_RESET);
			compiler->bytecode[compiler->count++] = TOY_OP_EOF; //1 byte
		break;

		case TOY_AST_NODE_VAR_DECL: {
			//first, embed the expression (leaves it on the stack)
			Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, node->varDecl.expression, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//write each piece of the declaration to the bytecode
			int identifierIndex = Toy_findLiteralIndex(&compiler->literalCache, node->varDecl.identifier);
			if (identifierIndex < 0) {
				identifierIndex = Toy_pushLiteralArray(&compiler->literalCache, node->varDecl.identifier);
			}

			int typeIndex = writeLiteralTypeToCache(&compiler->literalCache, node->varDecl.typeLiteral);

			//embed the info into the bytecode
			if (identifierIndex >= 256 || typeIndex >= 256) {
				//push a "long" declaration
				compiler->bytecode[compiler->count++] = TOY_OP_VAR_DECL_LONG; //1 byte

				*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)identifierIndex; //2 bytes
				compiler->count += sizeof(unsigned short);

				*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)typeIndex; //2 bytes
				compiler->count += sizeof(unsigned short);
			}
			else {
				//push a declaration
				compiler->bytecode[compiler->count++] = TOY_OP_VAR_DECL; //1 byte
				compiler->bytecode[compiler->count++] = (unsigned char)identifierIndex; //1 byte
				compiler->bytecode[compiler->count++] = (unsigned char)typeIndex; //1 byte
			}
		}
		break;

		case TOY_AST_NODE_FN_DECL: {
			//run a compiler over the function
			Toy_Compiler* fnCompiler = TOY_ALLOCATE(Toy_Compiler, 1);
			Toy_initCompiler(fnCompiler);
			Toy_writeCompiler(fnCompiler, node->fnDecl.arguments); //can be empty, but not NULL
			Toy_writeCompiler(fnCompiler, node->fnDecl.returns); //can be empty, but not NULL
			Toy_Opcode override = Toy_writeCompilerWithJumps(fnCompiler, node->fnDecl.block, NULL, NULL, -4, rootNode); //can be empty, but not NULL
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//adopt the panic state if anything happened
			if (fnCompiler->panic) {
				compiler->panic = true;
			}

			//create the function in the literal cache (by storing the compiler object)
			Toy_Literal fnLiteral = TOY_TO_FUNCTION_LITERAL(fnCompiler, 0);
			fnLiteral.type = TOY_LITERAL_FUNCTION_INTERMEDIATE; //NOTE: changing type

			//push the name
			int identifierIndex = Toy_findLiteralIndex(&compiler->literalCache, node->fnDecl.identifier);
			if (identifierIndex < 0) {
				identifierIndex = Toy_pushLiteralArray(&compiler->literalCache, node->fnDecl.identifier);
			}

			//push to function (functions are never equal)
			int fnIndex = Toy_pushLiteralArray(&compiler->literalCache, fnLiteral);

			//embed the info into the bytecode
			if (identifierIndex >= 256 || fnIndex >= 256) {
				//push a "long" declaration
				compiler->bytecode[compiler->count++] = TOY_OP_FN_DECL_LONG; //1 byte

				*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)identifierIndex; //2 bytes
				compiler->count += sizeof(unsigned short);

				*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)fnIndex; //2 bytes
				compiler->count += sizeof(unsigned short);
			}
			else {
				//push a declaration
				compiler->bytecode[compiler->count++] = TOY_OP_FN_DECL; //1 byte
				compiler->bytecode[compiler->count++] = (unsigned char)identifierIndex; //1 byte
				compiler->bytecode[compiler->count++] = (unsigned char)fnIndex; //1 byte
			}
		}
		break;

		case TOY_AST_NODE_FN_COLLECTION: {
			//embed these in the bytecode...
			unsigned short index = (unsigned short)writeNodeCollectionToCache(compiler, node);
			
			if (index == (unsigned short)-1) {
				compiler->panic = true;
				return TOY_OP_EOF;
			}

			memcpy(compiler->bytecode + compiler->count, &index, sizeof(index));
			compiler->count += sizeof(unsigned short);
		}
		break;

		case TOY_AST_NODE_FN_CALL: {
			//NOTE: assume the function definition/name is above us

			for (int i = 0; i < node->fnCall.arguments->fnCollection.count; i++) { //reverse order, to count from the beginning in the interpreter
				//sub-calls
				if (node->fnCall.arguments->fnCollection.nodes[i].type != TOY_AST_NODE_LITERAL) {
					Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, &node->fnCall.arguments->fnCollection.nodes[i], breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
					if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
						compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
					}
					continue;
				}

				//write each argument to the bytecode
				int argumentsIndex = Toy_findLiteralIndex(&compiler->literalCache, node->fnCall.arguments->fnCollection.nodes[i].atomic.literal);
				if (argumentsIndex < 0) {
					argumentsIndex = Toy_pushLiteralArray(&compiler->literalCache, node->fnCall.arguments->fnCollection.nodes[i].atomic.literal);
				}

				//push the node opcode to the bytecode
				if (argumentsIndex >= 256) {
					//push a "long" index
					compiler->bytecode[compiler->count++] = TOY_OP_LITERAL_LONG; //1 byte

					*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)argumentsIndex; //2 bytes
					compiler->count += sizeof(unsigned short);
				}
				else {
					//push the index
					compiler->bytecode[compiler->count++] = TOY_OP_LITERAL; //1 byte
					compiler->bytecode[compiler->count++] = (unsigned char)argumentsIndex; //1 byte
				}
			}

			//push the argument COUNT to the top of the stack
			Toy_Literal argumentsCountLiteral =  TOY_TO_INTEGER_LITERAL(node->fnCall.argumentCount); //argumentCount is set elsewhere to support dot operator
			int argumentsCountIndex = Toy_findLiteralIndex(&compiler->literalCache, argumentsCountLiteral);
			if (argumentsCountIndex < 0) {
				argumentsCountIndex = Toy_pushLiteralArray(&compiler->literalCache, argumentsCountLiteral);
			}
			Toy_freeLiteral(argumentsCountLiteral);

			if (argumentsCountIndex >= 256) {
				//push a "long" index
				compiler->bytecode[compiler->count++] = TOY_OP_LITERAL_LONG; //1 byte

				*((unsigned short*)(compiler->bytecode + compiler->count)) = (unsigned short)argumentsCountIndex; //2 bytes
				compiler->count += sizeof(unsigned short);
			}
			else {
				//push the index
				compiler->bytecode[compiler->count++] = TOY_OP_LITERAL; //1 byte
				compiler->bytecode[compiler->count++] = (unsigned char)argumentsCountIndex; //1 byte
			}

			//call the function
			//DO NOT call the collection, this is done in binary
		}
		break;

		case TOY_AST_NODE_IF: {
			//process the condition
			Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, node->pathIf.condition, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//cache the point to insert the jump distance at
			compiler->bytecode[compiler->count++] = TOY_OP_IF_FALSE_JUMP; //1 byte
			int jumpToElse = compiler->count;
			compiler->count += sizeof(unsigned short); //2 bytes

			//write the then path
			override = Toy_writeCompilerWithJumps(compiler, node->pathIf.thenPath, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			int jumpToEnd = 0;

			if (node->pathIf.elsePath) {
				//insert jump to end
				compiler->bytecode[compiler->count++] = TOY_OP_JUMP; //1 byte
				jumpToEnd = compiler->count;
				compiler->count += sizeof(unsigned short); //2 bytes
			}

			//update the jumpToElse to point here
			unsigned short tmpVal = compiler->count + jumpOffsets;
			memcpy(compiler->bytecode + jumpToElse, &tmpVal, sizeof(tmpVal)); //2 bytes

			if (node->pathIf.elsePath) {
				//if there's an else path, write it and 
				Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, node->pathIf.elsePath, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
				if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
					compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
				}

				//update the jumpToEnd to point here
				tmpVal = compiler->count + jumpOffsets;
				memcpy(compiler->bytecode + jumpToEnd, &tmpVal, sizeof(tmpVal)); //2 bytes
			}
		}
		break;

		case TOY_AST_NODE_WHILE: {
			//for breaks and continues
			Toy_LiteralArray breakAddresses;
			Toy_LiteralArray continueAddresses;

			Toy_initLiteralArray(&breakAddresses);
			Toy_initLiteralArray(&continueAddresses);

			//cache the jump point
			unsigned short jumpToStart = compiler->count;

			//process the condition
			Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, node->pathWhile.condition, &breakAddresses, &continueAddresses, jumpOffsets, rootNode);
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//if false, jump to end
			compiler->bytecode[compiler->count++] = TOY_OP_IF_FALSE_JUMP; //1 byte
			unsigned short jumpToEnd = compiler->count;
			compiler->count += sizeof(unsigned short); //2 bytes

			//write the body
			override = Toy_writeCompilerWithJumps(compiler, node->pathWhile.thenPath, &breakAddresses, &continueAddresses, jumpOffsets, rootNode);
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//jump to condition
			compiler->bytecode[compiler->count++] = TOY_OP_JUMP; //1 byte
			unsigned short tmpVal = jumpToStart + jumpOffsets;
			memcpy(compiler->bytecode + compiler->count, &tmpVal, sizeof(tmpVal));
			compiler->count += sizeof(unsigned short); //2 bytes

			//jump from condition
			tmpVal = compiler->count + jumpOffsets;
			memcpy(compiler->bytecode + jumpToEnd, &tmpVal, sizeof(tmpVal));

			//set the breaks and continues
			for (int i = 0; i < breakAddresses.count; i++) {
				int point = TOY_AS_INTEGER(breakAddresses.literals[i]);
				tmpVal = compiler->count + jumpOffsets;
				memcpy(compiler->bytecode + point, &tmpVal, sizeof(tmpVal));
			}

			for (int i = 0; i < continueAddresses.count; i++) {
				int point = TOY_AS_INTEGER(continueAddresses.literals[i]);
				tmpVal = jumpToStart + jumpOffsets;
				memcpy(compiler->bytecode + point, &tmpVal, sizeof(tmpVal));
			}

			//clear the stack after use
			compiler->bytecode[compiler->count++] = TOY_OP_POP_STACK; //1 byte

			//cleanup
			Toy_freeLiteralArray(&breakAddresses);
			Toy_freeLiteralArray(&continueAddresses);
		}
		break;

		case TOY_AST_NODE_FOR: {
			//for breaks and continues
			Toy_LiteralArray breakAddresses;
			Toy_LiteralArray continueAddresses;

			Toy_initLiteralArray(&breakAddresses);
			Toy_initLiteralArray(&continueAddresses);

			compiler->bytecode[compiler->count++] = TOY_OP_SCOPE_BEGIN; //1 byte

			//initial setup
			Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, node->pathFor.preClause, &breakAddresses, &continueAddresses, jumpOffsets, rootNode);
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//conditional
			unsigned short jumpToStart = compiler->count;
			override = Toy_writeCompilerWithJumps(compiler, node->pathFor.condition, &breakAddresses, &continueAddresses, jumpOffsets, rootNode);
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			//if false jump to end
			compiler->bytecode[compiler->count++] = TOY_OP_IF_FALSE_JUMP; //1 byte
			unsigned short jumpToEnd = compiler->count;
			compiler->count += sizeof(unsigned short); //2 bytes

			//write the body
			compiler->bytecode[compiler->count++] = TOY_OP_SCOPE_BEGIN; //1 byte
			override = Toy_writeCompilerWithJumps(compiler, node->pathFor.thenPath, &breakAddresses, &continueAddresses, jumpOffsets, rootNode);
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}
			compiler->bytecode[compiler->count++] = TOY_OP_SCOPE_END; //1 byte

			//for-breaks actually jump to the bottom
			int jumpToIncrement = compiler->count;

			//evaluate third clause, restart
			override = Toy_writeCompilerWithJumps(compiler, node->pathFor.postClause, &breakAddresses, &continueAddresses, jumpOffsets, rootNode);
			if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
				compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
			}

			compiler->bytecode[compiler->count++] = TOY_OP_JUMP; //1 byte
			unsigned short tmpVal = jumpToStart + jumpOffsets;
			memcpy(compiler->bytecode + compiler->count, &tmpVal, sizeof(tmpVal));
			compiler->count += sizeof(unsigned short); //2 bytes

			tmpVal = compiler->count + jumpOffsets;
			memcpy(compiler->bytecode + jumpToEnd, &tmpVal, sizeof(tmpVal));

			compiler->bytecode[compiler->count++] = TOY_OP_SCOPE_END; //1 byte

			//set the breaks and continues
			for (int i = 0; i < breakAddresses.count; i++) {
				int point = TOY_AS_INTEGER(breakAddresses.literals[i]);
				tmpVal = compiler->count + jumpOffsets;
				memcpy(compiler->bytecode + point, &tmpVal, sizeof(tmpVal));
			}

			for (int i = 0; i < continueAddresses.count; i++) {
				int point = TOY_AS_INTEGER(continueAddresses.literals[i]);
				tmpVal = jumpToIncrement + jumpOffsets;
				memcpy(compiler->bytecode + point, &tmpVal, sizeof(tmpVal));
			}

			//clear the stack after use
			compiler->bytecode[compiler->count++] = TOY_OP_POP_STACK; //1 byte

			//cleanup
			Toy_freeLiteralArray(&breakAddresses);
			Toy_freeLiteralArray(&continueAddresses);
		}
		break;

		case TOY_AST_NODE_BREAK: {
			if (!breakAddressesPtr) {
				fprintf(stderr, TOY_CC_ERROR "[internal] Can't place a break statement here\n" TOY_CC_RESET);
				break;
			}

			//insert into bytecode
			compiler->bytecode[compiler->count++] = TOY_OP_JUMP; //1 byte

			//push to the breakAddresses array
			Toy_Literal literal = TOY_TO_INTEGER_LITERAL(compiler->count);
			Toy_pushLiteralArray((Toy_LiteralArray*)breakAddressesPtr, literal);
			Toy_freeLiteral(literal);

			compiler->count += sizeof(unsigned short); //2 bytes
		}
		break;

		case TOY_AST_NODE_CONTINUE: {
			if (!continueAddressesPtr) {
				fprintf(stderr, TOY_CC_ERROR "[internal] Can't place a continue statement here\n" TOY_CC_RESET);
				break;
			}

			//insert into bytecode
			compiler->bytecode[compiler->count++] = TOY_OP_JUMP; //1 byte

			//push to the continueAddresses array
			Toy_Literal literal = TOY_TO_INTEGER_LITERAL(compiler->count);
			Toy_pushLiteralArray((Toy_LiteralArray*)continueAddressesPtr, literal);
			Toy_freeLiteral(literal);

			compiler->count += sizeof(unsigned short); //2 bytes
		}
		break;

		case TOY_AST_NODE_FN_RETURN: {
			//read each returned literal onto the stack, and return the number of values to return
			for (int i = 0; i < node->returns.returns->fnCollection.count; i++) {
				Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, &node->returns.returns->fnCollection.nodes[i], breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
				if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
					compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
				}
			}

			//push the return, with the number of literals
			compiler->bytecode[compiler->count++] = TOY_OP_FN_RETURN; //1 byte

			memcpy(compiler->bytecode + compiler->count, &node->returns.returns->fnCollection.count, sizeof(unsigned short));
			compiler->count += sizeof(unsigned short);
		}
		break;

		case TOY_AST_NODE_PREFIX_INCREMENT: {
			//push the literal to the stack (twice: add + assign)
			writeLiteralToCompiler(compiler, node->prefixIncrement.identifier);
			writeLiteralToCompiler(compiler, node->prefixIncrement.identifier);

			//push the increment / decrement
			Toy_Literal increment = TOY_TO_INTEGER_LITERAL(1);
			writeLiteralToCompiler(compiler, increment);

			//push the add opcode
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_ADDITION; //1 byte

			//push the assign
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_VAR_ASSIGN; //1 byte

			//leave the result on the stack
			writeLiteralToCompiler(compiler, node->prefixIncrement.identifier);
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_LITERAL_RAW; //1 byte
		}
		break;

		case TOY_AST_NODE_PREFIX_DECREMENT: {
			//push the literal to the stack (twice: add + assign)
			writeLiteralToCompiler(compiler, node->prefixDecrement.identifier);
			writeLiteralToCompiler(compiler, node->prefixDecrement.identifier);

			//push the increment / decrement
			Toy_Literal increment = TOY_TO_INTEGER_LITERAL(1);
			writeLiteralToCompiler(compiler, increment);

			//push the subtract opcode
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_SUBTRACTION; //1 byte

			//push the assign
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_VAR_ASSIGN; //1 byte

			//leave the result on the stack
			writeLiteralToCompiler(compiler, node->prefixDecrement.identifier);
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_LITERAL_RAW; //1 byte
		}
		break;

		case TOY_AST_NODE_POSTFIX_INCREMENT: {
			//push the identifier's VALUE to the stack
			writeLiteralToCompiler(compiler, node->postfixIncrement.identifier);
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_LITERAL_RAW; //1 byte

			//push the identifier (twice: add + assign)
			writeLiteralToCompiler(compiler, node->postfixIncrement.identifier);
			writeLiteralToCompiler(compiler, node->postfixIncrement.identifier);

			//push the increment / decrement
			Toy_Literal increment = TOY_TO_INTEGER_LITERAL(1);
			writeLiteralToCompiler(compiler, increment);

			//push the add opcode
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_ADDITION; //1 byte

			//push the assign
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_VAR_ASSIGN; //1 byte
		}
		break;

		case TOY_AST_NODE_POSTFIX_DECREMENT: {
			//push the identifier's VALUE to the stack
			writeLiteralToCompiler(compiler, node->postfixDecrement.identifier);
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_LITERAL_RAW; //1 byte

			//push the identifier (twice: add + assign)
			writeLiteralToCompiler(compiler, node->postfixDecrement.identifier);
			writeLiteralToCompiler(compiler, node->postfixDecrement.identifier);

			//push the increment / decrement
			Toy_Literal increment = TOY_TO_INTEGER_LITERAL(1);
			writeLiteralToCompiler(compiler, increment);

			//push the subtract opcode
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_SUBTRACTION; //1 byte

			//push the assign
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_VAR_ASSIGN; //1 byte
		}
		break;

		case TOY_AST_NODE_IMPORT: {
			//push the identifier, and the alias
			writeLiteralToCompiler(compiler, node->import.identifier);
			writeLiteralToCompiler(compiler, node->import.alias);

			//push the import opcode
			compiler->bytecode[compiler->count++] = (unsigned char)TOY_OP_IMPORT; //1 byte
		}
		break;

		case TOY_AST_NODE_INDEX: {
			//pass to the child nodes, then embed the opcode

			//first
			if (!node->index.first) {
				writeLiteralToCompiler(compiler, TOY_TO_NULL_LITERAL);
			}
			else {
				Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, node->index.first, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
				if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
					compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
				}
			}

			//second
			if (!node->index.second) {
				writeLiteralToCompiler(compiler, TOY_TO_NULL_LITERAL);
			}
			else {
				Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, node->index.second, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
				if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
					compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
				}
			}

			//third
			if (!node->index.third) {
				writeLiteralToCompiler(compiler, TOY_TO_NULL_LITERAL);
			}
			else {
				Toy_Opcode override = Toy_writeCompilerWithJumps(compiler, node->index.third, breakAddressesPtr, continueAddressesPtr, jumpOffsets, rootNode);
				if (override != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
					compiler->bytecode[compiler->count++] = (unsigned char)override; //1 byte
				}
			}

			// compiler->bytecode[compiler->count++] = (unsigned char)OP_INDEX; //1 byte

			return TOY_OP_INDEX_ASSIGN; //override binary's instruction IF it is assign
		}
		break;

		case TOY_AST_NODE_PASS: {
			return TOY_OP_PASS;
		}
		break;
	}

	return TOY_OP_EOF;
}

void Toy_writeCompiler(Toy_Compiler* compiler, Toy_ASTNode* node) {
	Toy_Opcode op = Toy_writeCompilerWithJumps(compiler, node, NULL, NULL, 0, node); //pass in "node" as the root node

	if (op != TOY_OP_EOF) {//compensate for indexing & dot notation being screwy
		compiler->bytecode[compiler->count++] = (unsigned char)op; //1 byte
	}

	//TODO: could free up AST Nodes
}

void Toy_freeCompiler(Toy_Compiler* compiler) {
	Toy_freeLiteralArray(&compiler->literalCache);
	TOY_FREE_ARRAY(unsigned char, compiler->bytecode, compiler->capacity);
	compiler->bytecode = NULL;
	compiler->capacity = 0;
	compiler->count = 0;
	compiler->panic = false;
}

static void emitByte(unsigned char** collationPtr, int* capacityPtr, int* countPtr, unsigned char byte) {
	//grow the array
	if (*countPtr + 1 > *capacityPtr) {
		int oldCapacity = *capacityPtr;
		*capacityPtr = TOY_GROW_CAPACITY(*capacityPtr);
		*collationPtr = TOY_GROW_ARRAY(unsigned char, *collationPtr, oldCapacity, *capacityPtr);
	}

	//append to the collation
	(*collationPtr)[(*countPtr)++] = byte;
}

static void Toy_emitShort(unsigned char** collationPtr, int* capacityPtr, int* countPtr, unsigned short bytes) {
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
static unsigned char* collateCompilerHeaderOpt(Toy_Compiler* compiler, size_t* size, bool embedHeader) {
	if (compiler->panic) {
		fprintf(stderr, TOY_CC_ERROR "[internal] Can't collate a panicked compiler\n" TOY_CC_RESET);
		return NULL;
	}

	int capacity = TOY_GROW_CAPACITY(0);
	int count = 0;
	unsigned char* collation = TOY_ALLOCATE(unsigned char, capacity);

	//for the function-section at the end of the main-collation
	int fnIndex = 0; //counts up for each fn
	int fnCapacity = TOY_GROW_CAPACITY(0);
	int fnCount = 0;
	unsigned char* fnCollation = TOY_ALLOCATE(unsigned char, fnCapacity);

	if (embedHeader) {
		//embed the header with version information
		emitByte(&collation, &capacity, &count, TOY_VERSION_MAJOR);
		emitByte(&collation, &capacity, &count, TOY_VERSION_MINOR);
		emitByte(&collation, &capacity, &count, TOY_VERSION_PATCH);

		//embed the build info
		if ((int)strlen(TOY_VERSION_BUILD) + count + 1 > capacity) {
			int oldCapacity = capacity;
			capacity = strlen(TOY_VERSION_BUILD) + count + 1; //full header size
			collation = TOY_GROW_ARRAY(unsigned char, collation, oldCapacity, capacity);
		}

		memcpy(&collation[count], TOY_VERSION_BUILD, strlen(TOY_VERSION_BUILD));
		count += strlen(TOY_VERSION_BUILD);
		collation[count++] = '\0'; //terminate the build string

		emitByte(&collation, &capacity, &count, TOY_OP_SECTION_END); //terminate header
	}

	//embed the data section (first short is the number of literals)
	Toy_emitShort(&collation, &capacity, &count, compiler->literalCache.count);

	//emit each literal by type
	for (int i = 0; i < compiler->literalCache.count; i++) {
		//literal Opcode
		// emitShort(&collation, &capacity, &count, OP_LITERAL); //This isn't needed

		//literal type, followed by literal value
		switch(compiler->literalCache.literals[i].type) {
			case TOY_LITERAL_NULL:
				emitByte(&collation, &capacity, &count, TOY_LITERAL_NULL);
				//null has no following value
			break;

			case TOY_LITERAL_BOOLEAN:
				emitByte(&collation, &capacity, &count, TOY_LITERAL_BOOLEAN);
				emitByte(&collation, &capacity, &count, TOY_AS_BOOLEAN(compiler->literalCache.literals[i]));
			break;

			case TOY_LITERAL_INTEGER:
				emitByte(&collation, &capacity, &count, TOY_LITERAL_INTEGER);
				emitInt(&collation, &capacity, &count, TOY_AS_INTEGER(compiler->literalCache.literals[i]));
			break;

			case TOY_LITERAL_FLOAT:
				emitByte(&collation, &capacity, &count, TOY_LITERAL_FLOAT);
				emitFloat(&collation, &capacity, &count, TOY_AS_FLOAT(compiler->literalCache.literals[i]));
			break;

			case TOY_LITERAL_STRING: {
				emitByte(&collation, &capacity, &count, TOY_LITERAL_STRING);

				Toy_Literal str = compiler->literalCache.literals[i];

				for (int c = 0; c < (int)Toy_lengthRefString(TOY_AS_STRING(str)); c++) {
					emitByte(&collation, &capacity, &count, Toy_toCString(TOY_AS_STRING(str))[c]);
				}

				emitByte(&collation, &capacity, &count, '\0'); //terminate the string
			}
			break;

			case TOY_LITERAL_ARRAY: {
				emitByte(&collation, &capacity, &count, TOY_LITERAL_ARRAY);

				Toy_LiteralArray* ptr = TOY_AS_ARRAY(compiler->literalCache.literals[i]);

				//length of the array, as a short
				Toy_emitShort(&collation, &capacity, &count, ptr->count);

				//each element of the array
				for (int i = 0; i < ptr->count; i++) {
					Toy_emitShort(&collation, &capacity, &count, (unsigned short)TOY_AS_INTEGER(ptr->literals[i])); //shorts representing the indexes of the values
				}
			}
			break;

			case TOY_LITERAL_ARRAY_INTERMEDIATE: {
				emitByte(&collation, &capacity, &count, TOY_LITERAL_ARRAY_INTERMEDIATE);

				Toy_LiteralArray* ptr = TOY_AS_ARRAY(compiler->literalCache.literals[i]);

				//length of the array, as a short
				Toy_emitShort(&collation, &capacity, &count, ptr->count);

				//each element of the array
				for (int i = 0; i < ptr->count; i++) {
					Toy_emitShort(&collation, &capacity, &count, (unsigned short)TOY_AS_INTEGER(ptr->literals[i])); //shorts representing the indexes of the values
				}
			}
			break;

			case TOY_LITERAL_DICTIONARY: {
				emitByte(&collation, &capacity, &count, TOY_LITERAL_DICTIONARY);

				Toy_LiteralArray* ptr = TOY_AS_ARRAY(compiler->literalCache.literals[i]); //used an array for storage above

				//length of the array, as a short
				Toy_emitShort(&collation, &capacity, &count, ptr->count); //count is the array size, NOT the dictionary size

				//each element of the array
				for (int i = 0; i < ptr->count; i++) {
					Toy_emitShort(&collation, &capacity, &count, (unsigned short)TOY_AS_INTEGER(ptr->literals[i])); //shorts representing the indexes of the values
				}
			}
			break;

			case TOY_LITERAL_DICTIONARY_INTERMEDIATE: {
				emitByte(&collation, &capacity, &count, TOY_LITERAL_DICTIONARY_INTERMEDIATE);

				Toy_LiteralArray* ptr = TOY_AS_ARRAY(compiler->literalCache.literals[i]); //used an array for storage above

				//length of the array, as a short
				Toy_emitShort(&collation, &capacity, &count, ptr->count); //count is the array size, NOT the dictionary size

				//each element of the array
				for (int i = 0; i < ptr->count; i++) {
					Toy_emitShort(&collation, &capacity, &count, (unsigned short)TOY_AS_INTEGER(ptr->literals[i])); //shorts representing the indexes of the values
				}
			}
			break;

			case TOY_LITERAL_FUNCTION_INTERMEDIATE: {
				//extract the compiler
				Toy_Literal fn = compiler->literalCache.literals[i];
				void* fnCompiler = TOY_AS_FUNCTION(fn).inner.bytecode; //store the compiler here for now

				//collate the function into bytecode (without header)
				size_t size = 0;
				unsigned char* bytes = collateCompilerHeaderOpt((Toy_Compiler*)fnCompiler, &size, false);

				//emit how long this section is, +1 for ending mark
				Toy_emitShort(&fnCollation, &fnCapacity, &fnCount, (unsigned short)size + 1);

				//write the fn to the fn collation
				for (size_t i = 0; i < size; i++) {
					emitByte(&fnCollation, &fnCapacity, &fnCount, bytes[i]);
				}

				emitByte(&fnCollation, &fnCapacity, &fnCount, TOY_OP_FN_END); //for marking the correct end-point of the function

				//embed the reference to the function implementation into the current collation (to be extracted later)
				emitByte(&collation, &capacity, &count, TOY_LITERAL_FUNCTION);
				Toy_emitShort(&collation, &capacity, &count, (unsigned short)(fnIndex++));

				Toy_freeCompiler((Toy_Compiler*)fnCompiler);
				TOY_FREE(compiler, fnCompiler);
				TOY_FREE_ARRAY(unsigned char, bytes, size);
			}
			break;

			case TOY_LITERAL_IDENTIFIER: {
				emitByte(&collation, &capacity, &count, TOY_LITERAL_IDENTIFIER);

				Toy_Literal identifier = compiler->literalCache.literals[i];

				for (int c = 0; c < (int)Toy_lengthRefString(TOY_AS_IDENTIFIER(identifier)); c++) {
					emitByte(&collation, &capacity, &count, Toy_toCString(TOY_AS_IDENTIFIER(identifier))[c]);
				}

				emitByte(&collation, &capacity, &count, '\0'); //terminate the string
			}
			break;

			case TOY_LITERAL_TYPE: {
				//push a raw type
				emitByte(&collation, &capacity, &count, TOY_LITERAL_TYPE);

				Toy_Literal typeLiteral = compiler->literalCache.literals[i];

				//what type this literal represents
				emitByte(&collation, &capacity, &count, TOY_AS_TYPE(typeLiteral).typeOf);
				emitByte(&collation, &capacity, &count, TOY_AS_TYPE(typeLiteral).constant); //if it's constant
			}
			break;

			case TOY_LITERAL_TYPE_INTERMEDIATE: {
				emitByte(&collation, &capacity, &count, TOY_LITERAL_TYPE_INTERMEDIATE);

				Toy_LiteralArray* ptr = TOY_AS_ARRAY(compiler->literalCache.literals[i]); //used an array for storage above

				//the base literal
				Toy_Literal typeLiteral = Toy_copyLiteral(ptr->literals[0]);

				//what type this literal represents
				emitByte(&collation, &capacity, &count, TOY_AS_TYPE(typeLiteral).typeOf);
				emitByte(&collation, &capacity, &count, TOY_AS_TYPE(typeLiteral).constant); //if it's constant

				//each element of the array, If they exist, representing sub-types already in the cache
				if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_ARRAY || TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_DICTIONARY) {
					//the type will represent how many to expect in the array
					for (int i = 1; i < ptr->count; i++) {
						Toy_emitShort(&collation, &capacity, &count, (unsigned short)TOY_AS_INTEGER(ptr->literals[i])); //shorts representing the indexes of the types
					}
				}

				Toy_freeLiteral(typeLiteral);
			}
			break;

			case TOY_LITERAL_INDEX_BLANK:
				emitByte(&collation, &capacity, &count, TOY_LITERAL_INDEX_BLANK);
				//blank has no following value
			break;

			default:
				fprintf(stderr, TOY_CC_ERROR "[internal] Unknown literal type encountered within literal cache: %d\n" TOY_CC_RESET, compiler->literalCache.literals[i].type);
				return NULL;
		}
	}

	emitByte(&collation, &capacity, &count, TOY_OP_SECTION_END); //terminate data

	//embed the function section (beginning with function count, size)
	Toy_emitShort(&collation, &capacity, &count, fnIndex);
	Toy_emitShort(&collation, &capacity, &count, fnCount);

	for (int i = 0; i < fnCount; i++) {
		emitByte(&collation, &capacity, &count, fnCollation[i]);
	}

	emitByte(&collation, &capacity, &count, TOY_OP_SECTION_END); //terminate function section

	TOY_FREE_ARRAY(unsigned char, fnCollation, fnCapacity); //clear the function stuff

	//code section
	for (int i = 0; i < compiler->count; i++) {
		emitByte(&collation, &capacity, &count, compiler->bytecode[i]);
	}

	emitByte(&collation, &capacity, &count, TOY_OP_SECTION_END); //terminate code

	emitByte(&collation, &capacity, &count, TOY_OP_EOF); //terminate bytecode

	//finalize
	collation = TOY_SHRINK_ARRAY(unsigned char, collation, capacity, count);

	*size = count;

	return collation;
}

//the whole point of the compiler is to alter bytecode, so leave it as non-const
unsigned char* Toy_collateCompiler(Toy_Compiler* compiler, size_t* size) {
	return collateCompilerHeaderOpt(compiler, size, true);
}
