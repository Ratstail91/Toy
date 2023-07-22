#pragma once

#include "toy_interpreter.h"

#define TOY_OPAQUE_TAG_RANDOM 200

int Toy_hookRandom(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias);
