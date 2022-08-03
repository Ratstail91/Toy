#include "debug.h"

#include "keyword_types.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void printToken(Token* token) {
	if (token->type == TOKEN_ERROR) {
		printf("Error\t%d\t%.*s\n", token->line, token->length, token->lexeme);
		return;
	}

	printf("\t%d\t%d\t", token->type, token->line);

	if (token->type == TOKEN_IDENTIFIER || token->type == TOKEN_LITERAL_INTEGER || token->type == TOKEN_LITERAL_FLOAT || token->type == TOKEN_LITERAL_STRING) {
		printf("%.*s\t", token->length, token->lexeme);
	} else {
		char* keyword = findKeywordByType(token->type);

		if (keyword != NULL) {
			printf("%s", keyword);
		} else {
			printf("-");
		}
	}

	printf("\n");
}

//declare the singleton
Command command;

void initCommand(int argc, const char* argv[]) {
	//default values
	command.error = false;
	command.help = false;
	command.version = false;
	command.filename = NULL;
	command.source = NULL;
	command.verbose = false;

	for (int i = 1; i < argc; i++) { //start at 1 to skip the program name
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			command.help = true;
			continue;
		}

		if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
			command.version = true;
			continue;
		}

		if ((!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file")) && i + 1 < argc) {
			command.filename = (char*)argv[i + 1];
			i++;
			continue;
		}

		if ((!strcmp(argv[i], "-i") || !strcmp(argv[i], "--input")) && i + 1 < argc) {
			command.source = (char*)argv[i + 1];
			i++;
			continue;
		}

		if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")) {
			command.verbose = true;
			continue;
		}

		command.error = true;
	}

	//no arguments
	if (argc == 1) {
		command.error = true;
	}
}

void usageCommand(int argc, const char* argv[]) {
	printf("Usage: %s [-h | -v | [-d][-f filename | -i source]]\n\n", argv[0]);
}

void helpCommand(int argc, const char* argv[]) {
	usageCommand(argc, argv);

	printf("-h | --help\t\tShow this help then exit.\n");
	printf("-v | --version\t\tShow version and copyright information then exit.\n");
	printf("-f | --file filename\tParse and execute the source file.\n");
	printf("-i | --input source\tParse and execute this given string of source code.\n");
	printf("-d | --debug\t\tBe verbose when operating.\n");
}

void copyrightCommand(int argc, const char* argv[]) {
	printf("Toy Programming Language Interpreter Version %d.%d.%d (built on %s)\n\n", TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, TOY_VERSION_BUILD);
	printf("Copyright (c) 2020-2022 Kayne Ruse, KR Game Studios\n\n");
	printf("This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.\n\n");
	printf("Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:\n\n");
	printf("1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.\n\n");
	printf("2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.\n\n");
	printf("3. This notice may not be removed or altered from any source distribution.\n\n");
}