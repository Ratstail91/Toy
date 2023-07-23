#pragma once

#include "toy_interpreter.h"

#define TOY_OPAQUE_TAG_RUNNER 100

int Toy_hookRunner(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias);
