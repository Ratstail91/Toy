#pragma once

#include "toy_interpreter.h"

#define TOY_OPAQUE_TAG_FILE 300

int Toy_hookFileIO(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias);
