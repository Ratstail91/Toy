#include "lib_fileio.h"
#include "toy_memory.h"
#include "drive_system.h"

#include <limits.h>
#include <stdio.h>

typedef struct Toy_File
{
	FILE* fp;
	Toy_RefString* mode;
	Toy_RefString* name;
} Toy_File;

Toy_File* createToyFile(const char* mode, const char* name) {
	Toy_File* file = TOY_ALLOCATE(Toy_File, 1);
	file->fp = NULL;
	file->mode = Toy_createRefString(mode);
	file->name = Toy_createRefString(name);

	return file;
}

void deleteToyFile(Toy_File* file) {
	Toy_deleteRefString(file->mode);
	Toy_deleteRefString(file->name);
	TOY_FREE(Toy_File, file);
}

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
	Toy_File* file = createToyFile(mode, filePath);

	// attempt to open file
	file->fp = fopen(filePath, mode);

	// result
	Toy_Literal fileLiteral = TOY_TO_NULL_LITERAL;
	if (file->fp == NULL) {
		deleteToyFile(file);
	}
	else {
		fileLiteral = TOY_TO_OPAQUE_LITERAL(file, TOY_OPAQUE_TAG_FILE);
	}
	
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
	if (!(TOY_IS_OPAQUE(selfLiteral) || TOY_GET_OPAQUE_TAG(selfLiteral) == TOY_OPAQUE_TAG_FILE)) {
		interpreter->errorOutput("Incorrect argument type passed to close\n");
		Toy_freeLiteral(selfLiteral);
		
		return -1;
	}

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);

	int result = 0;
	if (file->fp != NULL) {
		result = fclose(file->fp);
		file->fp = NULL;
	}

	// return the result
	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result != EOF);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	deleteToyFile(file);
	Toy_freeLiteral(resultLiteral);
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
	if (!TOY_IS_OPAQUE(selfLiteral) && TOY_GET_OPAQUE_TAG(selfLiteral) != 900) {
		interpreter->errorOutput("Incorrect argument type passed to read\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(valueLiteral);
		
		return -1;
	}

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);

	Toy_Literal resultLiteral = TOY_TO_NULL_LITERAL;

	switch (valueLiteral.as.type.typeOf)
		{
		// case TOY_LITERAL_BOOLEAN:
		// {
		// 	char value[TOY_MAX_STRING_LENGTH] = {0};
		// 	fgets(value, sizeof(value) - 1, file->fp);
		// 	value[TOY_MAX_STRING_LENGTH] = '\0';

		// 	Toy_Literal stringLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(value));
		// 	resultLiteral = TOY_TO_BOOLEAN_LITERAL(TOY_IS_TRUTHY(stringLiteral));
		// 	Toy_freeLiteral(stringLiteral);

		// 	break;
		// }

		case TOY_LITERAL_INTEGER:
		{
			int value = 0;
			fscanf(file->fp, "%i", &value);

			resultLiteral = TOY_TO_INTEGER_LITERAL(value);

			break;
		}

		case TOY_LITERAL_FLOAT:
		{
			float value = 0.0f;
			fscanf(file->fp, "%f", &value);

			resultLiteral = TOY_TO_FLOAT_LITERAL(value);

			break;
		}

		case TOY_LITERAL_STRING:
		{
			char value[TOY_MAX_STRING_LENGTH] = {0};
			fread(value, sizeof(char), TOY_MAX_STRING_LENGTH - 1, file->fp);
			value[TOY_MAX_STRING_LENGTH - 1] = '\0';

			resultLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(value));

			break;
		}
		
		default:
			// TODO handle other types
			break;
		}

	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(valueLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeWrite(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to write\n");
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
	if (TOY_IS_NULL(valueLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to write\n");
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
	if (!TOY_IS_OPAQUE(selfLiteral) && TOY_GET_OPAQUE_TAG(selfLiteral) != 900) {
		interpreter->errorOutput("Incorrect argument type passed to write\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(valueLiteral);
		
		return -1;
	}

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);

	int result = 0;
	switch (valueLiteral.type)
	{
	case TOY_LITERAL_INTEGER:
	{
		result = fprintf(file->fp, "%i", TOY_AS_INTEGER(valueLiteral));
		break;
	}

	case TOY_LITERAL_FLOAT:
	{
		result = fprintf(file->fp, "%f", TOY_AS_FLOAT(valueLiteral));
		break;
	}

	case TOY_LITERAL_STRING:
	{
		result = fprintf(file->fp, "%s", Toy_toCString(TOY_AS_STRING(valueLiteral)));
		break;
	}
	
	default:
		// TODO handle other types
		break;
	}

	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result > 0);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(valueLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeRename(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to rename\n");
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
	if (!TOY_IS_STRING(valueLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to rename\n");
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
	if (!TOY_IS_OPAQUE(selfLiteral) && !(TOY_GET_OPAQUE_TAG(selfLiteral) == 900)) {
		interpreter->errorOutput("Incorrect argument type passed to read\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(valueLiteral);
		
		return -1;
	}

	Toy_Literal filePathLiteral = Toy_getDrivePathLiteral(interpreter, &valueLiteral);

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);
	const char* newName = Toy_toCString(TOY_AS_STRING(filePathLiteral));

	// close the file
	if (file->fp != NULL) {
		fclose(file->fp);
		file->fp = NULL;
	}

	// rename the file
	int result = rename(Toy_toCString(file->name), newName);

	// open file again
	file->fp = fopen(newName, Toy_toCString(file->mode));

	// update the file object's name
	Toy_deleteRefString(file->name);
	file->name = Toy_createRefString(newName);

	// return result
	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result == 0);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(filePathLiteral);
	Toy_freeLiteral(valueLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeSeek(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 3) {
		interpreter->errorOutput("Incorrect number of arguments to seek\n");
		return -1;
	}

	Toy_Literal offsetLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal originLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	// parse the offset (if it's an identifier)
	Toy_Literal offsetLiteralIdn = offsetLiteral;
	if (TOY_IS_IDENTIFIER(offsetLiteral) && Toy_parseIdentifierToValue(interpreter, &offsetLiteral)) {
		Toy_freeLiteral(offsetLiteralIdn);
	}

	// check the offset type
	if (!TOY_IS_INTEGER(offsetLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to seek\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(offsetLiteral);
		Toy_freeLiteral(originLiteral);

		return -1;
	}

	// parse the origin (if it's an identifier)
	Toy_Literal originLiteralIdn = originLiteral;
	if (TOY_IS_IDENTIFIER(originLiteral) && Toy_parseIdentifierToValue(interpreter, &originLiteral)) {
		Toy_freeLiteral(originLiteralIdn);
	}

	// check the origin type
	if (!TOY_IS_STRING(originLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to seek\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(offsetLiteral);
		Toy_freeLiteral(originLiteral);

		return -1;
	}

	// parse the self (if it's an identifier)
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	// check self type
	if (!TOY_IS_OPAQUE(selfLiteral) && TOY_GET_OPAQUE_TAG(selfLiteral) != 900) {
		interpreter->errorOutput("Incorrect argument type passed to seek\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(offsetLiteral);
		Toy_freeLiteral(originLiteral);
		
		return -1;
	}

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);
	int offset = TOY_AS_INTEGER(offsetLiteral);
	Toy_RefString* orginString = Toy_copyRefString(TOY_AS_STRING(originLiteral));

	int origin = 0;
	if (Toy_equalsRefStringCString(orginString, "bgn")) {
		origin = SEEK_SET;
	}
	else if (Toy_equalsRefStringCString(orginString, "cur")) {
		origin = SEEK_CUR;
	}
	else if (Toy_equalsRefStringCString(orginString, "end")) {
		origin = SEEK_END;
	}

	int result = origin >= SEEK_SET && origin <= SEEK_END? 
		fseek(file->fp, offset, origin) : -1;

	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result == 0);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(offsetLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeError(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to error\n");
		return -1;
	}

	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	// parse the self (if it's an identifier)
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	// check self type
	if (!(TOY_IS_OPAQUE(selfLiteral) || TOY_GET_OPAQUE_TAG(selfLiteral) == TOY_OPAQUE_TAG_FILE)) {
		interpreter->errorOutput("Incorrect argument type passed to error\n");
		Toy_freeLiteral(selfLiteral);
		
		return -1;
	}

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);

	int result = ferror(file->fp);

	// return the result
	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result != 0);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeCompleted(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to error\n");
		return -1;
	}

	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	// parse the self (if it's an identifier)
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	// check self type
	if (!(TOY_IS_OPAQUE(selfLiteral) || TOY_GET_OPAQUE_TAG(selfLiteral) == TOY_OPAQUE_TAG_FILE)) {
		interpreter->errorOutput("Incorrect argument type passed to error\n");
		Toy_freeLiteral(selfLiteral);
		
		return -1;
	}

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);

	int result = feof(file->fp);

	// return the result
	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result != 0);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativePosition(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to size\n");
		return -1;
	}

	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	// parse the self (if it's an identifier)
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	// check self type
	if (!(TOY_IS_OPAQUE(selfLiteral) || TOY_GET_OPAQUE_TAG(selfLiteral) == TOY_OPAQUE_TAG_FILE)) {
		interpreter->errorOutput("Incorrect argument type passed to size\n");
		Toy_freeLiteral(selfLiteral);
		
		return -1;
	}

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);
	
	// pervent integer overflow as ftell returns a long
	int size = ftell(file->fp) > INT_MAX? INT_MAX : ftell(file->fp);
		
	// return the result
	Toy_Literal resultLiteral = TOY_TO_INTEGER_LITERAL(size);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeSize(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to size\n");
		return -1;
	}

	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	// parse the self (if it's an identifier)
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	// check self type
	if (!(TOY_IS_OPAQUE(selfLiteral) || TOY_GET_OPAQUE_TAG(selfLiteral) == TOY_OPAQUE_TAG_FILE)) {
		interpreter->errorOutput("Incorrect argument type passed to size\n");
		Toy_freeLiteral(selfLiteral);
		
		return -1;
	}

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);

	int size = 0;
	fseek(file->fp, 0, SEEK_END);
		
	// pervent integer overflow as ftell returns a long
	if (ftell(file->fp) > INT_MAX) {
		size = INT_MAX;
	}
	else {
		size = ftell(file->fp);
	}
	
	fseek(file->fp, 0, SEEK_SET);

	// return the result
	Toy_Literal resultLiteral = TOY_TO_INTEGER_LITERAL(size);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeMode(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to mode\n");
		return -1;
	}

	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	// parse the self (if it's an identifier)
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	// check self type
	if (!(TOY_IS_OPAQUE(selfLiteral) || TOY_GET_OPAQUE_TAG(selfLiteral) == TOY_OPAQUE_TAG_FILE)) {
		interpreter->errorOutput("Incorrect argument type passed to mode\n");
		Toy_freeLiteral(selfLiteral);
		
		return -1;
	}

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);

	// return the result
	Toy_Literal resultLiteral = TOY_TO_STRING_LITERAL(Toy_copyRefString(file->mode));
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(resultLiteral);
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

// Helper function create a int variable
void createToyVariableInt(Variable* variable, char* key, int literal) {
	variable->key = TOY_TO_STRING_LITERAL(Toy_createRefString(key));
	variable->identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString(key));
	variable->literal = TOY_TO_INTEGER_LITERAL(literal);
}

// Helper function create a file variable
void createToyVariableFile(Variable* variable, char* key, Toy_File* literal) {
	variable->key = TOY_TO_STRING_LITERAL(Toy_createRefString(key));
	variable->identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString(key));
	variable->literal = TOY_TO_OPAQUE_LITERAL(literal, TOY_OPAQUE_TAG_FILE);
}

// Helper function to clean up variables
void deleteToyVariables(Variable variables[], int size) {
	for (int i = 0; i < size; i++) {
		Toy_freeLiteral(variables[i].key);
		Toy_freeLiteral(variables[i].identifier);
		Toy_freeLiteral(variables[i].literal);
	}
	
}

// Helper to check for naming conflicts with existing variables
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

// Helper to place variables into scope should be called after scopeConflict
void exposeVariablesToScope(Toy_Interpreter* interpreter, Variable variables[], int size) {
	Toy_Literal intType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_INTEGER, false);
	Toy_Literal opaqueType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_OPAQUE, false);
	
	for (int i = 0; i < size; i++) {
		if (variables[i].literal.type == TOY_LITERAL_INTEGER) {
			Toy_declareScopeVariable(interpreter->scope, variables[i].identifier, intType);
		}
		else if (variables[i].literal.type == TOY_LITERAL_OPAQUE) {
			Toy_declareScopeVariable(interpreter->scope, variables[i].identifier, opaqueType);
		}

		Toy_setScopeVariable(interpreter->scope, variables[i].identifier, variables[i].literal, true);
	}

	Toy_freeLiteral(intType);
	Toy_freeLiteral(opaqueType);
}

int Toy_hookFileIO(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias) {
	// build the natives list
	Natives natives[] = {
		// access
		{"open", nativeOpen},
		{"close", nativeClose},

		// input/output
		{"read", nativeRead},
		{"write", nativeWrite},

		// operations
		{"rename", nativeRename},
		{"seek", nativeSeek},

		// accessors
		{"error", nativeError},
		{"completed", nativeCompleted},
		{"position", nativePosition},
		{"size", nativeSize},
		{"mode", nativeMode},

		{NULL, NULL}
	};

	// global variables
	const int VARIABLES_SIZE = 5;
	Variable variables[VARIABLES_SIZE];
	
	createToyVariableInt(&variables[0], "MAX_FILENAME_SIZE", FILENAME_MAX);
	createToyVariableInt(&variables[1], "MAX_FILES_OPEN", FOPEN_MAX);
	createToyVariableInt(&variables[2], "END_OF_FILE", EOF);

	Toy_File* outFile = createToyFile("w", "output");
	outFile->fp = stdout;

	createToyVariableFile(&variables[3], "output", outFile);

	Toy_File* inFile = createToyFile("r", "input");
	inFile->fp = stdin;

	createToyVariableFile(&variables[4], "input", inFile);

	// store the library in an aliased dictionary
	if (!TOY_IS_NULL(alias)) {
		// make sure the name isn't taken
		if (Toy_isDeclaredScopeVariable(interpreter->scope, alias)) {
			interpreter->errorOutput("Can't override an existing variable\n");
			Toy_freeLiteral(alias);
			return -1;
		}

		// create the dictionary to load up with functions
		Toy_LiteralDictionary* dictionary = TOY_ALLOCATE(Toy_LiteralDictionary, 1);
		Toy_initLiteralDictionary(dictionary);

		// load the dict with functions
		for (int i = 0; natives[i].name; i++) {
			Toy_Literal name = TOY_TO_STRING_LITERAL(Toy_createRefString(natives[i].name));
			Toy_Literal func = TOY_TO_FUNCTION_NATIVE_LITERAL(natives[i].fn);

			Toy_setLiteralDictionary(dictionary, name, func);

			Toy_freeLiteral(name);
			Toy_freeLiteral(func);
		}

		// set global variables
		for (int i = 0; i < VARIABLES_SIZE; i++) {
			Toy_setLiteralDictionary(dictionary, variables[i].key, variables[i].literal);
		}
		
		// build the type
		Toy_Literal type = TOY_TO_TYPE_LITERAL(TOY_LITERAL_DICTIONARY, true);
		Toy_Literal anyType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_ANY, true);
		Toy_Literal fnType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_FUNCTION_NATIVE, true);
		TOY_TYPE_PUSH_SUBTYPE(&type, anyType);
		TOY_TYPE_PUSH_SUBTYPE(&type, fnType);

		// set scope
		Toy_Literal dict = TOY_TO_DICTIONARY_LITERAL(dictionary);
		Toy_declareScopeVariable(interpreter->scope, alias, type);
		Toy_setScopeVariable(interpreter->scope, alias, dict, false);

		// cleanup
		Toy_freeLiteral(dict);
		Toy_freeLiteral(type);

		return 0;
	}

	// default
	for (int i = 0; natives[i].name; i++) {
		Toy_injectNativeFn(interpreter, natives[i].name, natives[i].fn);
	}

	if (scopeConflict(interpreter, variables, VARIABLES_SIZE)) {
		return -1;
	}

	exposeVariablesToScope(interpreter, variables, VARIABLES_SIZE);

	deleteToyVariables(variables, VARIABLES_SIZE);

	return 0;
}
