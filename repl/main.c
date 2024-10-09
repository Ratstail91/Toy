#include "toy.h"
#include "toy_print.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utilities
unsigned char* readFile(char* path, int* size) {
	//open the file
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
		*size = -1; //missing file error
		return NULL;
	}

	//determine the file's length
	fseek(file, 0L, SEEK_END);
	*size = ftell(file);
	rewind(file);

	//make some space
	unsigned char* buffer = malloc(*size + 1);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	//
	if (fread(buffer, sizeof(unsigned char), *size, file) < *size) {
		fclose(file);
		*size = -2; //singal a read error
		return NULL;
	}

	fclose(file);

	buffer[(*size)++] = '\0';
	return buffer;
}

int dir(char* dest, const char* src) {
	//extract the directory from src, and store it in dest

#if defined(_WIN32) || defined(_WIN64)
	char* p = strrchr(src, '\\');
#else
	char* p = strrchr(src, '/');
#endif

	int len = p != NULL ? p - src + 1 : 0;
	strncpy(dest, src, len);
	dest[len] = '\0';

	return len;
}

#define APPEND(dest, src) \
	strncpy((dest) + (strlen(dest)), (src), strlen((src)) + 1);

#if defined(_WIN32) || defined(_WIN64)
	#define FLIPSLASH(str) for (int i = 0; str[i]; i++) str[i] = str[i] == '/' ? '\\' : str[i];
#else
	#define FLIPSLASH(str) for (int i = 0; str[i]; i++) str[i] = str[i] == '\\' ? '/' : str[i];
#endif

//handle command line arguments
typedef struct CmdLine {
	bool error;
	bool help;
	bool version;
	char* infile;
	int infileLength;
} CmdLine;

void usageCmdLine(int argc, const char* argv[]) {
	printf("Usage: %s [ -h | -v | -f source.toy ]\n\n", argv[0]);
}

void helpCmdLine(int argc, const char* argv[]) {
	usageCmdLine(argc, argv);

	printf("  -h, --help\t\t\tShow this help then exit.\n");
	printf("  -v, --version\t\t\tShow version and copyright information then exit.\n");
	printf("  -f, --file infile\t\tParse, compile and execute the source file then exit.\n");
}

void versionCmdLine(int argc, const char* argv[]) {
	printf("The Toy Programming Language, Version %d.%d.%d %s\n\n", TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, TOY_VERSION_BUILD);

	//copy/pasted from the license file - there's a way to include it directly, but it's too finnicky to bother
	const char* license = "\
Copyright (c) 2020-2024 Kayne Ruse, KR Game Studios\n\
\n\
This software is provided 'as-is', without any express or implied\n\
warranty. In no event will the authors be held liable for any damages\n\
arising from the use of this software.\n\
\n\
Permission is granted to anyone to use this software for any purpose,\n\
including commercial applications, and to alter it and redistribute it\n\
freely, subject to the following restrictions:\n\
\n\
1. The origin of this software must not be misrepresented; you must not\n\
   claim that you wrote the original software. If you use this software\n\
   in a product, an acknowledgment in the product documentation would be\n\
   appreciated but is not required.\n\
2. Altered source versions must be plainly marked as such, and must not be\n\
   misrepresented as being the original software.\n\
3. This notice may not be removed or altered from any source distribution.\n\n";

	printf("%s",license);
}

CmdLine parseCmdLine(int argc, const char* argv[]) {
	CmdLine cmd = { .error = false, .help = false, .version = false, .infile = NULL, .infileLength = 0 };

	for (int i = 1; i < argc; i++) {
		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			cmd.help = true;
		}

		else if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
			cmd.version = true;
		}

		else if (!strcmp(argv[i], "-f") || !strcmp(argv[i], "--file")) {
			if (argc < i + 1) {
				cmd.error = true;
			}
			else {
				if (cmd.infile != NULL) { //don't leak
					free(cmd.infile);
				}

				i++;

				//total space to reserve - it's actually longer than needed, due to the exe name being removed
				cmd.infileLength = strlen(argv[0]) + strlen(argv[i]);
				cmd.infile = malloc(cmd.infileLength + 1);

				if (cmd.infile == NULL) {
					fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate space while parsing the command line, exiting\n" TOY_CC_RESET);
					exit(-1);
				}

				dir(cmd.infile, argv[0]);
				APPEND(cmd.infile, argv[i]);
				FLIPSLASH(cmd.infile);
			}
		}

		else {
			cmd.error = true;
		}
	}

	return cmd;
}

//repl function
static void errorAndContinueCallback(const char* msg) {
	fprintf(stderr, "%s\n", msg);
}

int repl(const char* name) {
	Toy_setErrorCallback(errorAndContinueCallback);
	Toy_setAssertFailureCallback(errorAndContinueCallback);

	//vars to use
	unsigned int INPUT_BUFFER_SIZE = 4096;
	char inputBuffer[INPUT_BUFFER_SIZE];
	memset(inputBuffer, 0, INPUT_BUFFER_SIZE);

	Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

	Toy_VM vm;
	Toy_initVM(&vm);

	printf("%s> ", name); //shows the terminal prompt

	//read from the terminal
	while(fgets(inputBuffer, INPUT_BUFFER_SIZE, stdin)) {
		//work around fgets() adding a newline
		unsigned int length = strlen(inputBuffer);
		if (inputBuffer[length - 1] == '\n') {
			inputBuffer[--length] = '\0';
		}

		if (length == 0) {
			printf("%s> ", name); //shows the terminal prompt
			continue;
		}

		//end
		if (strlen(inputBuffer) == 4 && (strncmp(inputBuffer, "exit", 4) == 0 || strncmp(inputBuffer, "quit", 4) == 0)) {
			break;
		}

		//parse the input, prep the VM for run
		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, inputBuffer);
		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);
		Toy_Ast* ast = Toy_scanParser(&bucket, &parser); //Ast is in the bucket, so it doesn't need to be freed

		//parsing error, retry
		if (parser.error) {
			printf("%s> ", name); //shows the terminal prompt
			continue;
		}

		Toy_Bytecode bc = Toy_compileBytecode(ast);
		Toy_bindVM(&vm, bc.ptr);

		//run
		Toy_runVM(&vm);

		//free the bytecode, and leave the VM ready for the next loop
		Toy_resetVM(&vm);

		//count the bucket memory - hang on, this this garbage collection??
		Toy_Bucket* iter = bucket;
		int depth = 0;
		while (iter->next) {
			iter = iter->next;
			if (++depth >= 7) { //8 buckets in the chain total, about 8kb allocated
				Toy_freeBucket(&bucket);
				bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
				break;
			}
		}

		printf("%s> ", name); //shows the terminal prompt
	}

	//cleanp all memory
	Toy_freeVM(&vm);
	Toy_freeBucket(&bucket);

	return 0;
}

//callbacks
static void printCallback(const char* msg) {
	fprintf(stdout, "%s\n", msg);
}

static void errorAndExitCallback(const char* msg) {
	fprintf(stderr, "%s\n", msg);
	exit(-1);
}

//main file
int main(int argc, const char* argv[]) {
	Toy_setPrintCallback(printCallback);
	Toy_setErrorCallback(errorAndExitCallback);
	Toy_setAssertFailureCallback(errorAndExitCallback);

	//repl
	if (argc == 1) {
		return repl(argv[0]);
	}

	//if there's args, process them
	CmdLine cmd = parseCmdLine(argc, argv);

	if (cmd.error) {
		usageCmdLine(argc, argv);
	}
	else if (cmd.help) {
		helpCmdLine(argc, argv);
	}
	else if (cmd.version) {
		versionCmdLine(argc, argv);
	}
	else if (cmd.infile != NULL) {
		//run the given file
		int size;
		unsigned char* source = readFile(cmd.infile, &size);

		//check the file
		if (source == NULL) {
			if (size == 0) {
				fprintf(stderr, TOY_CC_ERROR "ERROR: Could not parse an empty file '%s', exiting\n" TOY_CC_RESET, cmd.infile);
				return -1;
			}

			else if (size == -1) {
				fprintf(stderr, TOY_CC_ERROR "ERROR: File not found '%s', exiting\n" TOY_CC_RESET, cmd.infile);
				return -1;
			}

			else {
				fprintf(stderr, TOY_CC_ERROR "ERROR: Unknown error while reading file '%s', exiting\n" TOY_CC_RESET, cmd.infile);
				return -1;
			}
		}

		free(cmd.infile);

		cmd.infile = NULL;
		cmd.infileLength = 0;

		Toy_Lexer lexer;
		Toy_bindLexer(&lexer, (char*)source);

		Toy_Parser parser;
		Toy_bindParser(&parser, &lexer);

		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);
		Toy_Ast* ast = Toy_scanParser(&bucket, &parser);

		Toy_Bytecode bc = Toy_compileBytecode(ast);

		//run the setup
		Toy_VM vm;
		Toy_initVM(&vm);
		Toy_bindVM(&vm, bc.ptr);

		//run
		Toy_runVM(&vm);

		//DEBUG: if there's anything left on the stack, print it
		if (vm.stack->count > 0) {
			printf("Debug output of the stack after execution\n\ntype\tvalue\n");
			for (int i = 0; i < vm.stack->count; i++) {
				Toy_Value v = ((Toy_Value*)(vm.stack + 1))[i];

				printf(" %d\t ", v.type);

				switch(v.type) {
					case TOY_VALUE_NULL:
						printf("null");
						break;

					case TOY_VALUE_BOOLEAN:
						printf("%s", TOY_VALUE_AS_BOOLEAN(v) ? "true" : "false");
						break;

					case TOY_VALUE_INTEGER:
						printf("%d", TOY_VALUE_AS_INTEGER(v));
						break;

					case TOY_VALUE_FLOAT:
						printf("%f", TOY_VALUE_AS_FLOAT(v));
						break;

					case TOY_VALUE_STRING: {
						Toy_String* str = TOY_VALUE_AS_STRING(v);

						//print based on type
						if (str->type == TOY_STRING_NODE) {
							char* buffer = Toy_getStringRawBuffer(str);
							printf("%s", buffer);
							free(buffer);
						}
						else if (str->type == TOY_STRING_LEAF) {
							printf("%s", str->as.leaf.data);
						}
						else if (str->type == TOY_STRING_NAME) {
							printf("%s", str->as.name.data);
						}
						break;
					}

					case TOY_VALUE_ARRAY:
					case TOY_VALUE_DICTIONARY:
					case TOY_VALUE_FUNCTION:
					case TOY_VALUE_OPAQUE:
						printf("???");
						break;
				}

				printf("\n");
			}
		}

		//cleanup
		Toy_freeVM(&vm);
		Toy_freeBucket(&bucket);
		free(source);
	}
	else {
		usageCmdLine(argc, argv);
	}

	return 0;
}
