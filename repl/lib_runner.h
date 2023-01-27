#pragma once

#include "toy_interpreter.h"

int Toy_hookRunner(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias);

//file system API - these need to be set by the host
void Toy_initDriveDictionary();
void Toy_freeDriveDictionary();
Toy_LiteralDictionary* Toy_getDriveDictionary();

#define TOY_OPAQUE_TAG_RUNNER 100

//file system API - for use with other libs
Toy_Literal Toy_getFilePathLiteral(Toy_Interpreter* interpreter, Toy_Literal* drivePathLiteral);
