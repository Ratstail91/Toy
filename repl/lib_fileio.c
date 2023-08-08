#include "lib_fileio.h"
#include "toy_memory.h"
#include "drive_system.h"

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

typedef struct Toy_File
{
	FILE* fp;
	int error;
	int size;
} Toy_File;

static int nativeOpen(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count < 1 || arguments->count > 2) {
		interpreter->errorOutput("Incorrect number of arguments to open\n");
		return -1;
	}

	Toy_Literal modeLiteral = arguments->count == 2? Toy_popLiteralArray(arguments) : TOY_TO_STRING_LITERAL(Toy_createRefString("r"));
	Toy_Literal drivePathLiteral = Toy_popLiteralArray(arguments);

	// parse the drivePath (if it's an identifier)
	Toy_Literal drivePathLiteralIdn = drivePathLiteral;
	if (TOY_IS_IDENTIFIER(drivePathLiteral) && Toy_parseIdentifierToValue(interpreter, &drivePathLiteral)) {
		Toy_freeLiteral(drivePathLiteralIdn);
	}

	// check the drivePath type
	if (!TOY_IS_STRING(drivePathLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to open\n");
		Toy_freeLiteral(drivePathLiteral);
		Toy_freeLiteral(modeLiteral);

		return -1;
	}

	Toy_Literal filePathLiteral = Toy_getDrivePathLiteral(interpreter, &drivePathLiteral);
	
	if (TOY_IS_NULL(filePathLiteral)) {
		interpreter->errorOutput("File not found in the specified drive\n");
		Toy_freeLiteral(drivePathLiteral);
		Toy_freeLiteral(filePathLiteral);
		Toy_freeLiteral(modeLiteral);

		return -1;
	}

	// parse the mode (if it's an identifier)
	Toy_Literal modeLiteralIdn = modeLiteral;
	if (TOY_IS_IDENTIFIER(modeLiteral) && Toy_parseIdentifierToValue(interpreter, &modeLiteral)) {
		Toy_freeLiteral(modeLiteralIdn);
	}

	// check the mode type
	if (!TOY_IS_STRING(modeLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to open\n");
		Toy_freeLiteral(drivePathLiteral);
		Toy_freeLiteral(filePathLiteral);
		Toy_freeLiteral(modeLiteral);

		return -1;
	}

	const char* filePath = Toy_toCString(TOY_AS_STRING(filePathLiteral));
	size_t filePathLength = Toy_lengthRefString(TOY_AS_STRING(filePathLiteral));

	const char* mode = Toy_toCString(TOY_AS_STRING(modeLiteral));

	// build file object
	Toy_File* file = TOY_ALLOCATE(Toy_File, 1);
	file->error = 0;
	file->fp = NULL;
	file->size = 0;

	// attempt to open file
	file->fp = fopen(filePath, mode);
	if (file->fp == NULL) {
		TOY_FREE(Toy_File, file);
		fprintf(stderr, "Error code: %d\n", errno);
		fprintf(stderr, "File not found: %s\n", filePath);
		file->error = 1;
	}

	// set size
	if (!file->error) {
		fseek(file->fp, 0, SEEK_END);
		
		// pervent integer overflow as ftell returns a long
		file->size = ftell(file->fp) > INT_MAX? INT_MAX : ftell(file->fp);
		
		fseek(file->fp, 0, SEEK_SET);
	}

	// result
	Toy_Literal fileLiteral = TOY_TO_OPAQUE_LITERAL(file, 900);

	Toy_pushLiteralArray(&interpreter->stack, fileLiteral);
	
	// cleanup
	Toy_freeLiteral(fileLiteral);
	Toy_freeLiteral(drivePathLiteral);
	Toy_freeLiteral(filePathLiteral);
	Toy_freeLiteral(modeLiteral);

	return 1;
}

static int nativeClose(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to close\n");
		return -1;
	}

	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	// parse the self (if it's an identifier)
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	// check self type
	if (!(TOY_IS_OPAQUE(selfLiteral) || TOY_GET_OPAQUE_TAG(selfLiteral) == 900)) {
		interpreter->errorOutput("Incorrect argument type passed to close\n");
		Toy_freeLiteral(selfLiteral);
		
		return -1;
	}

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);

	int result = fclose(file->fp);

	// return the result
	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result > 0);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeRead(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to read\n");
		return -1;
	}

	Toy_Literal valueLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	// parse the value (if it's an identifier)
	Toy_Literal valueLiteralIdn = valueLiteral;
	if (TOY_IS_IDENTIFIER(valueLiteral) && Toy_parseIdentifierToValue(interpreter, &valueLiteral)) {
		Toy_freeLiteral(valueLiteralIdn);
	}

	// check the value type
	if (!TOY_IS_TYPE(valueLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to read\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(valueLiteral);

		return -1;
	}

	// parse the self (if it's an identifier)
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	// check self type
	if (!(TOY_IS_OPAQUE(selfLiteral) || TOY_GET_OPAQUE_TAG(selfLiteral) == 900)) {
		interpreter->errorOutput("Incorrect argument type passed to read\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(valueLiteral);
		
		return -1;
	}

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);

	char buffer[256] = {0};

	// fscanf(file->fp, buffer);

	Toy_RefString* result = Toy_createRefStringLength(buffer, 256);

	Toy_pushLiteralArray(&interpreter->stack, TOY_TO_STRING_LITERAL(result));

	Toy_freeLiteral(valueLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

// call the hook
typedef struct Natives {
	char* name;
	Toy_NativeFn fn;
} Natives;

typedef struct Variable {
	Toy_Literal key;
	Toy_Literal identifier;
	Toy_Literal literal;
} Variable;

// Helper function create a variable
void createToyVariable(Variable* variable, char* key, int literal) {
	variable->key = TOY_TO_STRING_LITERAL(Toy_createRefString(key));
	variable->identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString(key));
	variable->literal = TOY_TO_INTEGER_LITERAL(literal);
}

// Helper function to clean up variables
void deleteToyVariables(Variable variables[], int size) {
	for (int i = 0; i < size; i++) {
		Toy_freeLiteral(variables[i].key);
		Toy_freeLiteral(variables[i].identifier);
		Toy_freeLiteral(variables[i].literal);
	}
	
}

bool scopeConflict(Toy_Interpreter* interpreter, Variable variables[], int size) {
	for (int i = 0; i < size; i++) {
		if (Toy_isDeclaredScopeVariable(interpreter->scope, variables[i].literal)) {
			interpreter->errorOutput("Can't override an existing variable\n");
			
			deleteToyVariables(variables, size);
			
			return true;
		}
	}

	return false;
}

void exposeVariablesToScope(Toy_Interpreter* interpreter, Variable variables[], int size) {
	Toy_Literal intType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_INTEGER, true);
	
	for (int i = 0; i < size; i++) {
		Toy_declareScopeVariable(interpreter->scope, variables[i].identifier, intType);
		Toy_setScopeVariable(interpreter->scope, variables[i].identifier, variables[i].literal, false);
	}

	Toy_freeLiteral(intType);
}

int Toy_hookFileIO(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias) {
	//build the natives list
	Natives natives[] = {
		// Access
		{"open", nativeOpen},
		{"close", nativeClose},

		//
		{"read", nativeRead},
		{NULL, NULL}
	};
	
	// store the library in an aliased dictionary
	if (!TOY_IS_NULL(alias)) {
		// make sure the name isn't taken
		if (Toy_isDeclaredScopeVariable(interpreter->scope, alias)) {
			interpreter->errorOutput("Can't override an existing variable\n");
			Toy_freeLiteral(alias);
			return -1;
		}

		// TODO
	}

	// default
	for (int i = 0; natives[i].name; i++) {
		Toy_injectNativeFn(interpreter, natives[i].name, natives[i].fn);
	}

	const int VARIABLES_SIZE = 3;
	Variable variables[VARIABLES_SIZE];
	
	createToyVariable(&variables[0], "MAX_FILENAME_SIZE", FILENAME_MAX);
	createToyVariable(&variables[1], "MAX_FILES_OPEN", FOPEN_MAX);
	createToyVariable(&variables[2], "END_OF_FILE", EOF);

	if (scopeConflict(interpreter, variables, VARIABLES_SIZE)) {
		return -1;
	}

	exposeVariablesToScope(interpreter, variables, VARIABLES_SIZE);

	deleteToyVariables(variables, VARIABLES_SIZE);

	return 0;
}
