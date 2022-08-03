#pragma once

#include "common.h"
#include "lexer.h"

void printToken(Token* token);

//for processing the command line arguments
typedef struct {
	bool error;
	bool help;
	bool version;
	char* filename;
	char* source;
	bool verbose;
} Command;

extern Command command;

void initCommand(int argc, const char* argv[]);

void usageCommand(int argc, const char* argv[]);
void helpCommand(int argc, const char* argv[]);
void copyrightCommand(int argc, const char* argv[]);
