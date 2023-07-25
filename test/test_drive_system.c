#include "drive_system.h"

#include "toy_console_colors.h"

#include <stdio.h>
#include <string.h>

int main() {
	{
		//test init and quit
		Toy_initDriveSystem();
		Toy_freeDriveSystem();
	}

	{
		//setup
		Toy_initDriveSystem();

		//test storing a value as a drive
		Toy_setDrivePath("drive", "folder");

		//cleanup
		Toy_freeDriveSystem();
	}

	{
		//setup
		Toy_initDriveSystem();

		//create a dummy interpreter (only needed for the error output function)
		Toy_Interpreter interpreter;
		Toy_initInterpreter(&interpreter);

		//prerequisite
		Toy_setDrivePath("drive", "path/to/drive");

		//create the argument literal
		Toy_Literal argumentLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("drive:/path/to/file"));

		//test retrieving a relative path, as a literal, from the drive system
		Toy_Literal resultLiteral = Toy_getDrivePathLiteral(&interpreter, &argumentLiteral);

		//assert the correct value was returned
		const char* cstring = Toy_toCString(TOY_AS_STRING(resultLiteral));
		if (strcmp(cstring, "path/to/drive/path/to/file") != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Incorrect value retrieved from drive system: %s" TOY_CC_RESET, cstring);
			return -1;
		}

		//cleanup
		Toy_freeLiteral(argumentLiteral);
		Toy_freeLiteral(resultLiteral);
		Toy_freeInterpreter(&interpreter);
		Toy_freeDriveSystem();
	}

	{
		//setup
		Toy_initDriveSystem();

		//test storing enough drives to trigger internal dictionary expansion
		Toy_setDrivePath("A", "folder");
		Toy_setDrivePath("B", "folder");
		Toy_setDrivePath("C", "folder");
		Toy_setDrivePath("D", "folder");
		Toy_setDrivePath("E", "folder");
		Toy_setDrivePath("F", "folder");
		Toy_setDrivePath("G", "folder");
		Toy_setDrivePath("H", "folder");
		Toy_setDrivePath("I", "folder");
		Toy_setDrivePath("J", "folder");

		//cleanup
		Toy_freeDriveSystem();
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}