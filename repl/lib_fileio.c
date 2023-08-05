#include "lib_fileio.h"
#include "toy_memory.h"

#include "hal_file.h"

typedef struct Toy_File
{
	hal_file* fp;
	int error;
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

	static int tag = 0;
	bool error = false;
	hal_file* fp = NULL;

	// attempt to open file
	hal_file_code code = hal_file_manager.open(&fp, filename, mode);
	if (code != HAL_SUCCESS) {
		error = true;
	}

	// set size
	int size = 0;
	
	// if (!error) {
	// 	fseek(fp, 0, SEEK_END);
		
	// 	// pervent integer overflow as ftell returns a long
	// 	size = ftell(fp) > INT_MAX? INT_MAX : ftell(fp);
		
	// 	fseek(fp, 0, SEEK_SET);
	// }

	// result
	Toy_LiteralDictionary* result = TOY_ALLOCATE(Toy_LiteralDictionary, 1);
	Toy_initLiteralDictionary(result);

	Toy_Literal fileKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("_file"));
	Toy_Literal fileLiteral = TOY_TO_OPAQUE_LITERAL(fp, tag);

	Toy_Literal errorKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("error"));
	Toy_Literal errorLiteral = TOY_TO_BOOLEAN_LITERAL(error);

	Toy_Literal sizeKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("size"));
	Toy_Literal sizeLiteral = TOY_TO_INTEGER_LITERAL(size);

	Toy_setLiteralDictionary(result, fileKeyLiteral, fileLiteral);
	Toy_setLiteralDictionary(result, errorKeyLiteral, errorLiteral);
	Toy_setLiteralDictionary(result, sizeKeyLiteral, sizeLiteral);

	Toy_Literal name = TOY_TO_STRING_LITERAL(Toy_createRefString("File"));
	Toy_Literal resultLiteral = TOY_TO_DICTIONARY_LITERAL(result);

	Toy_Literal type = TOY_TO_TYPE_LITERAL(TOY_LITERAL_DICTIONARY, true);
	Toy_Literal stringType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_STRING, true);
	Toy_Literal anyType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_ANY, true);
	TOY_TYPE_PUSH_SUBTYPE(&type, stringType);
	TOY_TYPE_PUSH_SUBTYPE(&type, anyType);


	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);
	
	// cleanup
	Toy_freeLiteral(type);
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(filenameLiteral);
	Toy_freeLiteral(modeLiteral);

	tag++;

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

		{NULL, NULL}
	};
	

	//default
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
