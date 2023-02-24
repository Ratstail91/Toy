#pragma once

#include "toy_common.h"
#include "toy_interpreter.h"

int Toy_hookRunner(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias);

#define TOY_OPAQUE_TAG_RUNNER 100

//platform/compiler-specific instructions - because MSVC + Box Engine are dumber than a bag of rocks
#if defined(__linux__) || defined(__MINGW32__) || defined(__GNUC__)

#define LIB_RUNNER_API extern

#elif defined(_MSC_VER)

#ifndef LIB_RUNNER_EXPORT
#define LIB_RUNNER_API __declspec(dllimport)
#else
#define LIB_RUNNER_API __declspec(dllexport)
#endif

#else

#define LIB_RUNNER_API extern

#endif

//file system API - these need to be set by the host
LIB_RUNNER_API void Toy_initDriveDictionary();
LIB_RUNNER_API void Toy_freeDriveDictionary();
LIB_RUNNER_API Toy_LiteralDictionary* Toy_getDriveDictionary();

//file system API - for use with other libs
LIB_RUNNER_API Toy_Literal Toy_getFilePathLiteral(Toy_Interpreter* interpreter, Toy_Literal* drivePathLiteral);
