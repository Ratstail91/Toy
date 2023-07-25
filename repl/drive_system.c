#include "drive_system.h"

#include "toy_memory.h"
#include "toy_literal_dictionary.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//file system API
static Toy_LiteralDictionary driveDictionary;

void Toy_initDriveSystem() {
	Toy_initLiteralDictionary(&driveDictionary);
}

void Toy_freeDriveSystem() {
	Toy_freeLiteralDictionary(&driveDictionary);
}

TOY_API void Toy_setDrivePath(char* drive, char* path) {
	Toy_Literal driveLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(drive));
	Toy_Literal pathLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(path));

	Toy_setLiteralDictionary(&driveDictionary, driveLiteral, pathLiteral);

	Toy_freeLiteral(driveLiteral);
	Toy_freeLiteral(pathLiteral);
}

Toy_Literal Toy_getDrivePathLiteral(Toy_Interpreter* interpreter, Toy_Literal* drivePathLiteral) {
	//check argument types
	if (!TOY_IS_STRING(*drivePathLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to Toy_getDrivePathLiteral\n");
		return TOY_TO_NULL_LITERAL;
	}

	Toy_RefString* drivePath = Toy_copyRefString(TOY_AS_STRING(*drivePathLiteral));

	//get the drive and path as a string (can't trust that pesky strtok - custom split) TODO: move this to refstring library
	size_t driveLength = 0;
	while (Toy_toCString(drivePath)[driveLength] != ':') {
		if (driveLength >= Toy_lengthRefString(drivePath)) {
			interpreter->errorOutput("Incorrect drive path format given to Toy_getDrivePathLiteral\n");

			return TOY_TO_NULL_LITERAL;
		}

		driveLength++;
	}

	Toy_RefString* drive = Toy_createRefStringLength(Toy_toCString(drivePath), driveLength);
	Toy_RefString* filePath = Toy_createRefStringLength( &Toy_toCString(drivePath)[driveLength + 1], Toy_lengthRefString(drivePath) - driveLength );

	//get the real drive file path
	Toy_Literal driveLiteral = TOY_TO_STRING_LITERAL(drive); //NOTE: driveLiteral takes ownership of the refString
	Toy_Literal pathLiteral = Toy_getLiteralDictionary(&driveDictionary, driveLiteral);

	if (!TOY_IS_STRING(pathLiteral)) {
		interpreter->errorOutput("Incorrect literal type found for drive: ");
		Toy_printLiteralCustom(pathLiteral, interpreter->errorOutput);
		interpreter->errorOutput("\n");
		Toy_freeLiteral(driveLiteral);
		Toy_freeLiteral(pathLiteral);
		Toy_deleteRefString(filePath);
		Toy_deleteRefString(drivePath);

		return TOY_TO_NULL_LITERAL;
	}

	//get the final real file path (concat) TODO: move this concat to refstring library
	Toy_RefString* path = Toy_copyRefString(TOY_AS_STRING(pathLiteral));
	size_t fileLength = Toy_lengthRefString(path) + Toy_lengthRefString(filePath);

	char* file = TOY_ALLOCATE(char, fileLength + 1); //+1 for null
	snprintf(file, fileLength, "%s%s", Toy_toCString(path), Toy_toCString(filePath));

	//clean up the drive/path stuff
    Toy_deleteRefString(drivePath);
    Toy_deleteRefString(filePath);
    Toy_deleteRefString(path);
    Toy_freeLiteral(driveLiteral);
	Toy_freeLiteral(pathLiteral);

	//check for break-out attempts
	for (size_t i = 0; i < fileLength - 1; i++) {
		if (file[i] == '.' && file[i + 1] == '.') {
			interpreter->errorOutput("Parent directory access not allowed\n");
			TOY_FREE_ARRAY(char, file, fileLength + 1);
			return TOY_TO_NULL_LITERAL;
		}
	}

	Toy_Literal result = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(file, fileLength));

	TOY_FREE_ARRAY(char, file, fileLength + 1);

	return result;
}