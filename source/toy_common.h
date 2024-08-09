#pragma once

//for specified type sizes
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>


//TOY_API is platform-dependant, and marks publicly usable API functions
#if defined(__linux__) || defined(__MINGW32__) || defined(__GNUC__)

//GCC support
#define TOY_API extern

#elif defined(_MSC_VER)

//MSVC support
#ifndef TOY_EXPORT
#define TOY_API __declspec(dllimport)
#else
#define TOY_API __declspec(dllexport)
#endif

#else

//generic
#define TOY_API extern

#endif

//bytecode version specifiers, embedded as the header
#define TOY_VERSION_MAJOR 2
#define TOY_VERSION_MINOR 0
#define TOY_VERSION_PATCH 0

//defined as a function, for technical reasons
#define TOY_VERSION_BUILD Toy_private_version_build()
TOY_API const char* Toy_private_version_build();

