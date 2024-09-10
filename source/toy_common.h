#pragma once

//for specified type sizes
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>

//TOY_API is platform-dependant, and marks publicly usable API functions
#if defined(__GNUC__)
	#define TOY_API extern
#elif defined(_MSC_VER)
	#ifndef TOY_EXPORT
		#define TOY_API __declspec(dllimport)
	#else
		#define TOY_API __declspec(dllexport)
	#endif
#else
	//generic solution
	#define TOY_API extern
#endif

//TOY_BITNESS is used to encourage memory-cache friendliness
//source: https://www.linuxquestions.org/questions/programming-9/c-preprocessor-define-for-32-vs-64-bit-long-int-4175658579/
#if defined(__GNUC__)
	#if defined(__x86_64)
		#define TOY_BITNESS 64
	#else
		#define TOY_BITNESS 32
	#endif
#elif defined(_MSC_VER)
	#if defined(_M_X64)
		#define TOY_BITNESS 64
	#else
		#define TOY_BITNESS 32
	#endif
#else
	//generic solution
	#define TOY_BITNESS -1
#endif

//bytecode version specifiers, embedded as the header
#define TOY_VERSION_MAJOR 2
#define TOY_VERSION_MINOR 0
#define TOY_VERSION_PATCH 0

//defined as a function, for technical reasons
#define TOY_VERSION_BUILD Toy_private_version_build()
TOY_API const char* Toy_private_version_build();

