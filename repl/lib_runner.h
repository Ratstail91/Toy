#pragma once

#include "interpreter.h"

int hookRunner(Interpreter* interpreter, Literal identifier, Literal alias);

//file system API - these need to be set by the host
void initDriveDictionary();
void freeDriveDictionary();
LiteralDictionary* getDriveDictionary();

#define OPAQUE_TAG_RUNNER 100
