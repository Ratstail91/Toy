#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TOY_VERSION_MAJOR 0
#define TOY_VERSION_MINOR 7
#define TOY_VERSION_PATCH 1
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
} Command;

extern Command command;

void initCommand(int argc, const char* argv[]);

void usageCommand(int argc, const char* argv[]);
void helpCommand(int argc, const char* argv[]);
void copyrightCommand(int argc, const char* argv[]);
#endif
