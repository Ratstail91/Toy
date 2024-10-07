#pragma once

#include "toy_common.h"

//handle callbacks for printing to the terminal, or elsewhere
typedef void (*Toy_callbackType)(const char*);

TOY_API void Toy_print(const char* msg); //print keyword
TOY_API void Toy_error(const char* msg); //runtime errors
TOY_API void Toy_assertFailure(const char* msg); //assert keyword failures

TOY_API void Toy_setPrintCallback(Toy_callbackType cb);
TOY_API void Toy_setErrorCallback(Toy_callbackType cb);
TOY_API void Toy_setAssertFailureCallback(Toy_callbackType cb);

TOY_API void Toy_resetPrintCallback();
TOY_API void Toy_resetErrorCallback();
TOY_API void Toy_resetAssertFailureCallback();

