#pragma once

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#define TOY_VERSION_MAJOR 1
#define TOY_VERSION_MINOR 1
#define TOY_VERSION_PATCH 5
#define TOY_VERSION_BUILD Toy_private_version_build()

const char* Toy_private_version_build();

//platform/compiler-specific instructions
#if defined(__linux__) || defined(__MINGW32__) || defined(__GNUC__)

#define TOY_API extern

#elif defined(_MSC_VER)

#ifndef TOY_EXPORT
#define TOY_API __declspec(dllimport)
#else
#define TOY_API __declspec(dllexport)
#endif

#else

#define TOY_API extern

#endif

#ifndef TOY_DISABLE_REPL

//for processing the command line arguments in the repl
typedef struct {
	bool error;
	bool help;
	bool version;
	char* binaryfile;
	char* sourcefile;
	char* compilefile;
	char* outfile; //defaults to out.tb
	char* source;
	char* initialfile;
	bool enablePrintNewline;
	bool parseBytecodeHeader;
	bool verbose;
} Toy_CommandLine;

//these are intended for the repl only, despite using the api prefix
TOY_API Toy_CommandLine Toy_commandLine;

TOY_API void Toy_initCommandLine(int argc, const char* argv[]);

TOY_API void Toy_usageCommandLine(int argc, const char* argv[]);
TOY_API void Toy_helpCommandLine(int argc, const char* argv[]);
TOY_API void Toy_copyrightCommandLine(int argc, const char* argv[]);

#endif