#include "lib_fileio.h"
#include "toy_memory.h"

#include "../hal/hal_file.h"

typedef struct Toy_File
{
	hal_file* fp;
	hal_file_code error;
	int size;
} Toy_File;

static int nativeOpen(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count < 1 || arguments->count > 2) {
		interpreter->errorOutput("Incorrect number of arguments to open\n");
		return -1;
	}

	Toy_Literal modeLiteral = arguments->count == 2? Toy_popLiteralArray(arguments) : TOY_TO_STRING_LITERAL(Toy_createRefString("r"));
	Toy_Literal filenameLiteral = Toy_popLiteralArray(arguments);

	// parse the filename (if it's an identifier)
	Toy_Literal filenameLiteralIdn = filenameLiteral;
	if (TOY_IS_IDENTIFIER(filenameLiteral) && Toy_parseIdentifierToValue(interpreter, &filenameLiteral)) {
		Toy_freeLiteral(filenameLiteralIdn);
	}

	// check the filename type
	if (!TOY_IS_STRING(filenameLiteral)) {
		interpreter->errorOutput("open(string, string) incorrect type for the first parameter expected: string\n");
		Toy_freeLiteral(filenameLiteral);
	}

	// parse the mode (if it's an identifier)
	Toy_Literal modeLiteralIdn = modeLiteral;
	if (TOY_IS_IDENTIFIER(modeLiteral) && Toy_parseIdentifierToValue(interpreter, &modeLiteral)) {
		Toy_freeLiteral(modeLiteralIdn);
	}

	// check the mode type
	if (!TOY_IS_STRING(modeLiteral)) {
		interpreter->errorOutput("open(string, string) incorrect type for the second parameter expected: string\n");
		Toy_freeLiteral(modeLiteral);
	}

	const char* filename = Toy_toCString(TOY_AS_STRING(filenameLiteral));
	const char* mode = Toy_toCString(TOY_AS_STRING(modeLiteral));

	// build file object
	Toy_File* file = TOY_ALLOCATE(Toy_File, 1);
	file->error = HAL_SUCCESS;
	file->fp = NULL;
	file->size = 0;

	// attempt to open file
	file->error = hal_file_manager.open(&file->fp, filename, mode);

	// set size
	// if (!error) {
	// 	fseek(fp, 0, SEEK_END);
		
	// 	// pervent integer overflow as ftell returns a long
	// 	size = ftell(fp) > INT_MAX? INT_MAX : ftell(fp);
		
	// 	fseek(fp, 0, SEEK_SET);
	// }

	// result
	Toy_Literal fileLiteral = TOY_TO_OPAQUE_LITERAL(file, 900);

	Toy_pushLiteralArray(&interpreter->stack, fileLiteral);
	
	// cleanup
	Toy_freeLiteral(fileLiteral);
	Toy_freeLiteral(filenameLiteral);
	Toy_freeLiteral(modeLiteral);

	return 1;
}

static int nativeClose(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	// if (arguments->count != 1) {
	// 	interpreter->errorOutput("Incorrect number of arguments to open\n");
	// 	return -1;
	// }

	// Toy_Literal fileLiteral = Toy_popLiteralArray(arguments);

	// // parse the file (if it's an identifier)
	// Toy_Literal fileLiteralIdn = fileLiteral;
	// if (TOY_IS_IDENTIFIER(fileLiteral) && Toy_parseIdentifierToValue(interpreter, &fileLiteral)) {
	// 	Toy_freeLiteral(fileLiteralIdn);
	// }

	// // check the file type
	// if (!TOY_IS_STRING(fileLiteral)) {
	// 	interpreter->errorOutput("close(File) incorrect type for the first parameter expected: File\n");
	// 	Toy_freeLiteral(fileLiteral);
	// }

	// FILE* fp = (FILE*)TOY_AS_OPAQUE(fileLiteral);

	// Toy_freeLiteral(fileLiteral);

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
	if (!TOY_IS_OPAQUE(selfLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to read\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(valueLiteral);
		
		return -1;
	}

	Toy_File* file = (Toy_File*)TOY_AS_OPAQUE(selfLiteral);

	char buffer[256] = {0};

	hal_file_manager.read(file->fp, buffer, 256);

	Toy_RefString* result = Toy_createRefStringLength(buffer, 256);

	Toy_pushLiteralArray(&interpreter->stack, TOY_TO_STRING_LITERAL(result));

	Toy_freeLiteral(valueLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

//call the hook
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
	
	createToyVariable(&variables[0], "MAX_FILENAME_SIZE", HAL_MAX_FILENAME_SIZE);
	createToyVariable(&variables[1], "MAX_FILES_OPEN", HAL_MAX_FILES_OPEN);
	createToyVariable(&variables[2], "END_OF_FILE", HAL_EOF);

	if (scopeConflict(interpreter, variables, VARIABLES_SIZE)) {
		return -1;
	}

	exposeVariablesToScope(interpreter, variables, VARIABLES_SIZE);

	deleteToyVariables(variables, VARIABLES_SIZE);

	return 0;
}
