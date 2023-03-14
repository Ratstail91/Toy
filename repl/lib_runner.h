#pragma once

#include "toy_interpreter.h"

int Toy_hookRunner(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias);

#define TOY_OPAQUE_TAG_RUNNER 100

