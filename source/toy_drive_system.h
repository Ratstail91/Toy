#include "toy_common.h"

#include "toy_literal.h"
#include "toy_interpreter.h"

//file system API - these need to be set by the host
TOY_API void Toy_initDriveSystem();
TOY_API void Toy_freeDriveSystem();

//file system API - for use with libs
TOY_API void Toy_setDrivePath(char* drive, char* path);
TOY_API Toy_Literal Toy_getDrivePathLiteral(Toy_Interpreter* interpreter, Toy_Literal* drivePathLiteral);
