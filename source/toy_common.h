#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TOY_VERSION_MAJOR 0
#define TOY_VERSION_MINOR 8
#define TOY_VERSION_PATCH 3
#define TOY_VERSION_BUILD __DATE__ " " __TIME__

//platform/compiler-specific specifications
#if defined(__linux__)

#define TOY_API extern

#elif defined(_MSC_VER)

#ifndef TOY_EXPORT
#define TOY_API __declspec(dllimport)
#else
#define TOY_API __declspec(dllexport)
#endif

#elif defined(__MINGW32__)

#define TOY_API extern

#else

#define TOY_API extern

#endif

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

TOY_API Toy_CommandLine Toy_commandLine;

TOY_API void Toy_initCommandLine(int argc, const char* argv[]);

TOY_API void Toy_usageCommandLine(int argc, const char* argv[]);
TOY_API void Toy_helpCommandLine(int argc, const char* argv[]);
TOY_API void Toy_copyrightCommandLine(int argc, const char* argv[]);
