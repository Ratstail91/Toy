#pragma once

#include "toy_common.h"
#include "lexer.h"
#include "ast_node.h"

//DOCS: parsers are bound to a lexer, and turn the outputted tokens into AST nodes
typedef struct {
	Lexer* lexer;
	bool error; //I've had an error
	bool panic; //I am processing an error

	//track the last two outputs from the lexer
	Token current;
	Token previous;
} Parser;

TOY_API void initParser(Parser* parser, Lexer* lexer);
TOY_API void freeParser(Parser* parser);
TOY_API ASTNode* scanParser(Parser* parser);
