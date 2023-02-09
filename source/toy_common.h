#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TOY_VERSION_MAJOR 0
#define TOY_VERSION_MINOR 8
#define TOY_VERSION_PATCH 2
#define TOY_VERSION_BUILD __DATE__ " " __TIME__

//platform-specific specifications
#if defined(__linux__)
#define TOY_API extern

#elif defined(_WIN32) || defined(WIN32)
#define TOY_API

#else
#define TOY_API

#endif

#ifndef TOY_EXPORT
//for processing the command line arguments
typedef struct {
	bool error;
	bool help;
	bool version;
	char* binaryfile;
	char* sourcefile;
	char* compilefile;
	char* outfile; //defaults to out.tb
	char* source;
	bool verbose;
} Toy_CommandLine;

extern Toy_CommandLine Toy_commandLine;

void Toy_initCommandLine(int argc, const char* argv[]);

void Toy_usageCommandLine(int argc, const char* argv[]);
void Toy_helpCommandLine(int argc, const char* argv[]);
void Toy_copyrightCommandLine(int argc, const char* argv[]);
#endif
