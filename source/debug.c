#include "debug.h"

#include "keyword_types.h"
#include "lexer.h"
#include "parser.h"
#include "compiler.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

//utils
static unsigned char printByte(const char* tb, int* count) {
	unsigned char ret = *(unsigned char*)(tb + *count);
	printf("%u ", ret);
	*count += 1;
	return ret;
}

static unsigned short printShort(const char* tb, int* count) {
	unsigned short ret = *(unsigned short*)(tb + *count);
	printf("%d ", ret);
	*count += 2;
	return ret;
}

static int printInt(const char* tb, int* count) {
	int ret = *(int*)(tb + *count);
	printf("%d ", ret);
	*count += 4;
	return ret;
}

static float printFloat(const char* tb, int* count) {
	float ret = *(float*)(tb + *count);
	printf("%f ", ret);
	*count += 4;
	return ret;
}

static const char* printString(const char* tb, int* count) {
	const char* ret = tb + *count;
	*count += printf("%s ", ret); //return includes the space, but not the null terminator
	return ret;
}

static void consumeByte(unsigned char byte, const char* str, int* count) {
	if (byte != str[*count]) {
		printf("Failed to consume the correct byte");
	}
	*count += 1;
}

static void consumeShort(unsigned short bytes, const char* str, int* count) {
	if (bytes != *(unsigned short*)(str + *count)) {
		printf("Failed to consume the correct byte");
	}
	*count += 2;
}

void dissectBytecode(const char* tb, int size) {
	int count = 0;

	//header
	printf("--header--\n");
	printByte(tb, &count);
	printByte(tb, &count);
	printByte(tb, &count);
	printString(tb, &count);
	consumeByte(OP_SECTION_END, tb, &count);

	printf("\n");

	//data
	printf("--data--\n");
	const short literalCount = printShort(tb, &count);

	for (int i = 0; i < literalCount; i++) {
		const unsigned char literalType = printByte(tb, &count);

		switch(literalType) {
			case LITERAL_NULL:
				//NO-OP
				printf("(null)");
			break;

			case LITERAL_BOOLEAN: {
				const bool b = printByte(tb, &count);
				printf("(boolean %s)", b ? "true" : "false");
			}
			break;

			case LITERAL_INTEGER: {
				const int d = printInt(tb, &count);
				printf("(integer %d)", d);
			}
			break;

			case LITERAL_FLOAT: {
				const float f = printFloat(tb, &count);
				printf("(float %f)", f);
			}
			break;

			case LITERAL_STRING: {
				const s = printString(tb, &count);
				printf("(string)");
			}
			break;
		}

		printf("\n");
	}

	consumeByte(OP_SECTION_END, tb, &count);

	//code
	printf("--bytecode--\n");
	while(tb[count] != OP_EOF) {
		const opcode = printByte(tb, &count);

		switch (opcode) {
			case OP_PRINT:
				printf("print:\n");
			break;

			case OP_LITERAL: {
				printf("literal ");
				printByte(tb, &count);
				printf("\n");
			}
			break;

			case OP_LITERAL_LONG: {
				printf("long literal ");
				printShort(tb, &count);
				printf("\n");
			}
			break;

			case OP_NEGATE: {
				printf("negate\n");
			}
			break;

			case OP_SECTION_END: {
				printf("--SECTION END--");
			}
			break;

			default:
				printf("Unknown opcode found\n");
		}
	}

	consumeByte(OP_EOF, tb, &count);
}