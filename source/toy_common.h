#pragma once

/*!
# toy_common.h

This file is generally included in most header files within Toy, as it is where the TOY_API macro is defined. It also has some utilities intended for use only by the repl.

## Defined Macros
!*/

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*!
### TOY_API

This definition of this macro is platform-dependant, and used to enable cross-platform compilation of shared and static libraries.
!*/

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

/*!
### TOY_VERSION_MAJOR

The current major version of Toy. This value is embedded into the bytecode, and the interpreter will refuse to run bytecode with a major version that does not match it’s own version.

This value MUST fit into an unsigned char.
!*/

#define TOY_VERSION_MAJOR 1

/*!
### TOY_VERSION_MINOR

The current minor version of Toy. This value is embedded into the bytecode, and the interpreter will refuse to run bytecode with a minor version that is greater than its own minor version.

This value MUST fit into an unsigned char.
!*/

#define TOY_VERSION_MINOR 3

/*!
### TOY_VERSION_PATCH

The current patch version of Toy. This value is embedded into the bytecode.

This value MUST fit into an unsigned char.
!*/

#define TOY_VERSION_PATCH 2

/*!
### TOY_VERSION_BUILD

The current build version of Toy. This value is embedded into the bytecode.

This evaluates to a c-string, which contains build information such as compilation date and time of the interpreter. When in verbose mode, the compiler will display a warning if the build version of the bytecode does not match the build version of the interpreter.

This macro may also be used to store additonal information about forks of the Toy codebase.
!*/

#define TOY_VERSION_BUILD Toy_private_version_build()
TOY_API const char* Toy_private_version_build();

/*
The following code is intended only for use within the repl.
*/

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
