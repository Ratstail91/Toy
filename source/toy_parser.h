#pragma once

#include "toy_common.h"
#include "toy_lexer.h"
#include "toy_ast_node.h"

//DOCS: parsers are bound to a lexer, and turn the outputted tokens into AST nodes
typedef struct {
	Toy_Lexer* lexer;
	bool error; //I've had an error
	bool panic; //I am processing an error

	//track the last two outputs from the lexer
	Toy_Token current;
	Toy_Token previous;
} Toy_Parser;

TOY_API void Toy_initParser(Toy_Parser* parser, Toy_Lexer* lexer);
TOY_API void Toy_freeParser(Toy_Parser* parser);
TOY_API Toy_ASTNode* Toy_scanParser(Toy_Parser* parser);
