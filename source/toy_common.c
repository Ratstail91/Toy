#include "toy_common.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

//test variable sizes based on platform
#define STATIC_ASSERT(test_for_true) static_assert((test_for_true), "(" #test_for_true ") failed")

STATIC_ASSERT(sizeof(char) == 1);
STATIC_ASSERT(sizeof(short) == 2);
STATIC_ASSERT(sizeof(int) == 4);
STATIC_ASSERT(sizeof(float) == 4);
STATIC_ASSERT(sizeof(unsigned char) == 1);
STATIC_ASSERT(sizeof(unsigned short) == 2);
STATIC_ASSERT(sizeof(unsigned int) == 4);

//declare the singleton
Toy_CommandLine Toy_commandLine;

void Toy_initCommandLine(int argc, const char* argv[]) {
	//default values
	Toy_commandLine.error = false;
	Toy_commandLine.help = false;
	Toy_commandLine.version = false;
	Toy_commandLine.binaryfile = NULL;
	Toy_commandLine.sourcefile = NULL;
	Toy_commandLine.compilefile = NULL;
	Toy_commandLine.outfile = "out.tb";
	Toy_commandLine.source = NULL;
	Toy_commandLine.initialfile = NULL;
	Toy_commandLine.enablePrintNewline = true;
	Toy_commandLine.verbose = false;

	for (int i = 1; i < argc; i++) { //start at 1 to skip the program name
		Toy_commandLine.error = true; //error state by default, set to false by successful flags

		if (!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help")) {
			Toy_commandLine.help = true;
			Toy_commandLine.error = false;
			continue;
		}

		if (!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version")) {
			Toy_commandLine.version = true;
			Toy_commandLine.error = false;
			continue;
		}

		if (!strcmp(argv[i], "-d") || !strcmp(argv[i], "--debug")) {
			Toy_commandLine.verbose = true;
			Toy_commandLine.error = false;
			continue;
		}

		if ((!strcmp(argv[i], "-f") || !strcmp(argv[i], "--sourcefile")) && i + 1 < argc) {
			Toy_commandLine.sourcefile = (char*)argv[i + 1];
			i++;
			Toy_commandLine.error = false;
			continue;
		}

		if ((!strcmp(argv[i], "-i") || !strcmp(argv[i], "--input")) && i + 1 < argc) {
			Toy_commandLine.source = (char*)argv[i + 1];
			i++;
			Toy_commandLine.error = false;
			continue;
		}

		if ((!strcmp(argv[i], "-c") || !strcmp(argv[i], "--compile")) && i + 1 < argc) {
			Toy_commandLine.compilefile = (char*)argv[i + 1];
			i++;
			Toy_commandLine.error = false;
			continue;
		}

		if ((!strcmp(argv[i], "-o") || !strcmp(argv[i], "--output")) && i + 1 < argc) {
			Toy_commandLine.outfile = (char*)argv[i + 1];
			i++;
			Toy_commandLine.error = false;
			continue;
		}

		if ((!strcmp(argv[i], "-t") || !strcmp(argv[i], "--initial")) && i + 1 < argc) {
			Toy_commandLine.initialfile = (char*)argv[i + 1];
			i++;
			Toy_commandLine.error = false;
			continue;
		}

		if (!strcmp(argv[i], "-n")) {
			Toy_commandLine.enablePrintNewline = false;
			Toy_commandLine.error = false;
			continue;
		}

		//option without a flag + ending in .tb = binary input
		if (i < argc) {
			if (strncmp(&(argv[i][strlen(argv[i]) - 3]), ".tb", 3) == 0) {
				Toy_commandLine.binaryfile = (char*)argv[i];
				Toy_commandLine.error = false;
				continue;
			}
		}

		//don't keep reading in an error state
		return;
	}
}

void Toy_usageCommandLine(int argc, const char* argv[]) {
	printf("Usage: %s [ file.tb | -h | -v | -d | -f file.toy | -i source | -c file.toy -o out.tb | -t file.toy ]\n\n", argv[0]);
}

void Toy_helpCommandLine(int argc, const char* argv[]) {
	Toy_usageCommandLine(argc, argv);

	printf("<file.tb>\t\t\tBinary input file in tb format, must be version %d.%d.%d.\n\n", TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH);
	printf("-h\t| --help\t\tShow this help then exit.\n\n");
	printf("-v\t| --version\t\tShow version and copyright information then exit.\n\n");
	printf("-d\t| --debug\t\tBe verbose when operating.\n\n");
	printf("-f\t| --file filename\tParse, compile and execute the source file.\n\n");
	printf("-i\t| --input source\tParse, compile and execute this given string of source code.\n\n");
	printf("-c\t| --compile filename\tParse and compile the specified source file into an output file.\n\n");
	printf("-o\t| --output outfile\tName of the output file built with --compile (default: out.tb).\n\n");
	printf("-t\t| --initial filename\tStart the repl as normal, after first running the given file.\n\n");
	printf("-n\t|\t\t\tDisable the newline character at the end of the print statement.\n\n");
}

void Toy_copyrightCommandLine(int argc, const char* argv[]) {
	printf("Toy Programming Language Interpreter Version %d.%d.%d (built on %s)\n\n", TOY_VERSION_MAJOR, TOY_VERSION_MINOR, TOY_VERSION_PATCH, TOY_VERSION_BUILD);
	printf("Copyright (c) 2020-2023 Kayne Ruse, KR Game Studios\n\n");
	printf("This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.\n\n");
	printf("Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:\n\n");
	printf("1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.\n\n");
	printf("2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.\n\n");
	printf("3. This notice may not be removed or altered from any source distribution.\n\n");
}
