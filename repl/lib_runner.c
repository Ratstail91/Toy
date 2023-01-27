#include "lib_runner.h"

#include "toy_memory.h"
#include "toy_interpreter.h"

#include "repl_tools.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct Toy_Runner {
	Toy_Interpreter interpreter;
	unsigned char* bytecode;
	size_t size;

	bool dirty;
} Toy_Runner;

//Toy native functions
static int nativeLoadScript(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to loadScript\n");
		return -1;
	}

	//get the file path literal with a handle
	Toy_Literal drivePathLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal filePathLiteral = Toy_getFilePathLiteral(interpreter, &drivePathLiteral);

	if (TOY_IS_NULL(filePathLiteral)) {
		Toy_freeLiteral(filePathLiteral);
		Toy_freeLiteral(drivePathLiteral);
		return -1;
	}

	Toy_freeLiteral(drivePathLiteral);

	//use raw types - easier
	char* filePath = Toy_toCString(TOY_AS_STRING(filePathLiteral));
	int filePathLength = Toy_lengthRefString(TOY_AS_STRING(filePathLiteral));

	//check for file extensions
	if (!(filePath[filePathLength - 5] == '.' && filePath[filePathLength - 4] == 't' && filePath[filePathLength - 3] == 'o' && filePath[filePathLength - 2] == 'y')) {
		interpreter->errorOutput("Bad script file extension (expected .toy)\n");
		Toy_freeLiteral(filePathLiteral);
		return -1;
	}

	//check for break-out attempts
	for (int i = 0; i < filePathLength - 1; i++) {
		if (filePath[i] == '.' && filePath[i + 1] == '.') {
			interpreter->errorOutput("Parent directory access not allowed\n");
			Toy_freeLiteral(filePathLiteral);
			return -1;
		}
	}

	//load and compile the bytecode
	size_t fileSize = 0;
	char* source = Toy_readFile(filePath, &fileSize);

	if (!source) {
		interpreter->errorOutput("Failed to load source file\n");
		Toy_freeLiteral(filePathLiteral);
		return -1;
	}

	unsigned char* bytecode = Toy_compileString(source, &fileSize);
	free((void*)source);

	if (!bytecode) {
		interpreter->errorOutput("Failed to compile source file\n");
		Toy_freeLiteral(filePathLiteral);
		return -1;
	}

	//build the runner object
	Toy_Runner* runner = TOY_ALLOCATE(Toy_Runner, 1);
	Toy_setInterpreterPrint(&runner->interpreter, interpreter->printOutput);
	Toy_setInterpreterAssert(&runner->interpreter, interpreter->assertOutput);
	Toy_setInterpreterError(&runner->interpreter, interpreter->errorOutput);
	runner->interpreter.hooks = interpreter->hooks;
	runner->interpreter.scope = NULL;
	Toy_resetInterpreter(&runner->interpreter);
	runner->bytecode = bytecode;
	runner->size = fileSize;
	runner->dirty = false;

	//build the opaque object, and push it to the stack
	Toy_Literal runnerLiteral = TOY_TO_OPAQUE_LITERAL(runner, TOY_OPAQUE_TAG_RUNNER);
	Toy_pushLiteralArray(&interpreter->stack, runnerLiteral);

	//free the drive path
	Toy_freeLiteral(filePathLiteral);

	return 1;
}

static int nativeLoadScriptBytecode(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to loadScriptBytecode\n");
		return -1;
	}

	//get the argument
	Toy_Literal drivePathLiteral = Toy_popLiteralArray(arguments);
	Toy_RefString* drivePath = Toy_copyRefString(TOY_AS_STRING(drivePathLiteral));

	//get the drive and path as a string (can't trust that pesky strtok - custom split) TODO: move this to refstring library
	int driveLength = 0;
	while (Toy_toCString(drivePath)[driveLength] != ':') {
		if (driveLength >= Toy_lengthRefString(drivePath)) {
			interpreter->errorOutput("Incorrect drive path format given to loadScriptBytecode\n");
			Toy_deleteRefString(drivePath);
			Toy_freeLiteral(drivePathLiteral);
			return -1;
		}

		driveLength++;
	}

	Toy_RefString* drive = Toy_createRefStringLength(Toy_toCString(drivePath), driveLength);
	Toy_RefString* path = Toy_createRefStringLength( &Toy_toCString(drivePath)[driveLength + 1], Toy_lengthRefString(drivePath) - driveLength );

	//get the real drive file path
	Toy_Literal driveLiteral = TOY_TO_STRING_LITERAL(drive); //NOTE: driveLiteral takes ownership of the refString
	Toy_Literal realDriveLiteral = Toy_getLiteralDictionary(Toy_getDriveDictionary(), driveLiteral);

	if (!TOY_IS_STRING(realDriveLiteral)) {
		interpreter->errorOutput("Incorrect literal type found for drive: ");
		Toy_printLiteralCustom(realDriveLiteral, interpreter->errorOutput);
		interpreter->errorOutput("\n");
		Toy_freeLiteral(realDriveLiteral);
		Toy_freeLiteral(driveLiteral);
		Toy_deleteRefString(path);
		Toy_deleteRefString(drivePath);
		Toy_freeLiteral(drivePathLiteral);
		return -1;
	}

	//get the final real file path (concat) TODO: move this concat to refstring library
	Toy_RefString* realDrive = Toy_copyRefString(TOY_AS_STRING(realDriveLiteral));
	int realLength = Toy_lengthRefString(realDrive) + Toy_lengthRefString(path);

	char* filePath = TOY_ALLOCATE(char, realLength + 1); //+1 for null
	snprintf(filePath, realLength, "%s%s", Toy_toCString(realDrive), Toy_toCString(path));

	//clean up the drivepath stuff
	Toy_deleteRefString(realDrive);
	Toy_freeLiteral(realDriveLiteral);
	Toy_freeLiteral(driveLiteral);
	Toy_deleteRefString(path);
	Toy_deleteRefString(drivePath);
	Toy_freeLiteral(drivePathLiteral);

	//check for file extensions
	if (!(filePath[realLength - 4] == '.' && filePath[realLength - 3] == 't' && filePath[realLength - 2] == 'b')) {
		interpreter->errorOutput("Bad binary file extension (expected .tb)\n");
		TOY_FREE_ARRAY(char, filePath, realLength);
		return -1;
	}

	//check for break-out attempts
	for (int i = 0; i < realLength - 1; i++) {
		if (filePath[i] == '.' && filePath[i + 1] == '.') {
			interpreter->errorOutput("Parent directory access not allowed\n");
			TOY_FREE_ARRAY(char, filePath, realLength);
			return -1;
		}
	}

	//load the bytecode
	size_t fileSize = 0;
	unsigned char* bytecode = (unsigned char*)Toy_readFile(filePath, &fileSize);

	if (!bytecode) {
		interpreter->errorOutput("Failed to load bytecode file\n");
		return -1;
	}

	//build the runner object
	Toy_Runner* runner = TOY_ALLOCATE(Toy_Runner, 1);
	Toy_setInterpreterPrint(&runner->interpreter, interpreter->printOutput);
	Toy_setInterpreterAssert(&runner->interpreter, interpreter->assertOutput);
	Toy_setInterpreterError(&runner->interpreter, interpreter->errorOutput);
	runner->interpreter.hooks = interpreter->hooks;
	runner->interpreter.scope = NULL;
	Toy_resetInterpreter(&runner->interpreter);
	runner->bytecode = bytecode;
	runner->size = fileSize;
	runner->dirty = false;

	//build the opaque object, and push it to the stack
	Toy_Literal runnerLiteral = TOY_TO_OPAQUE_LITERAL(runner, TOY_OPAQUE_TAG_RUNNER);
	Toy_pushLiteralArray(&interpreter->stack, runnerLiteral);

	TOY_FREE_ARRAY(char, filePath, realLength);

	return 1;
}

static int nativeRunScript(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _runScript\n");
		return -1;
	}

	//get the runner object
	Toy_Literal runnerLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal runnerIdn = runnerLiteral;
	if (TOY_IS_IDENTIFIER(runnerLiteral) && Toy_parseIdentifierToValue(interpreter, &runnerLiteral)) {
		Toy_freeLiteral(runnerIdn);
	}

	if (TOY_GET_OPAQUE_TAG(runnerLiteral) != TOY_OPAQUE_TAG_RUNNER) {
		interpreter->errorOutput("Unrecognized opaque literal in _runScript\n");
		return -1;
	}

	Toy_Runner* runner = TOY_AS_OPAQUE(runnerLiteral);

	//run
	if (runner->dirty) {
		interpreter->errorOutput("Can't re-run a dirty script (try resetting it first)\n");
		Toy_freeLiteral(runnerLiteral);
		return -1;
	}

	unsigned char* bytecodeCopy = TOY_ALLOCATE(unsigned char, runner->size);
	memcpy(bytecodeCopy, runner->bytecode, runner->size); //need a COPY of the bytecode, because the interpreter eats it

	Toy_runInterpreter(&runner->interpreter, bytecodeCopy, runner->size);
	runner->dirty = true;

	//cleanup
	Toy_freeLiteral(runnerLiteral);

	return 0;
}

static int nativeGetScriptVar(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to _getScriptVar\n");
		return -1;
	}

	//get the runner object
	Toy_Literal varName = Toy_popLiteralArray(arguments);
	Toy_Literal runnerLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal varNameIdn = varName;
	if (TOY_IS_IDENTIFIER(varName) && Toy_parseIdentifierToValue(interpreter, &varName)) {
		Toy_freeLiteral(varNameIdn);
	}

	Toy_Literal runnerIdn = runnerLiteral;
	if (TOY_IS_IDENTIFIER(runnerLiteral) && Toy_parseIdentifierToValue(interpreter, &runnerLiteral)) {
		Toy_freeLiteral(runnerIdn);
	}

	if (TOY_GET_OPAQUE_TAG(runnerLiteral) != TOY_OPAQUE_TAG_RUNNER) {
		interpreter->errorOutput("Unrecognized opaque literal in _runScript\n");
		return -1;
	}

	Toy_Runner* runner = TOY_AS_OPAQUE(runnerLiteral);

	//dirty check
	if (!runner->dirty) {
		interpreter->errorOutput("Can't access variable from a non-dirty script (try running it first)\n");
		Toy_freeLiteral(runnerLiteral);
		return -1;
	}

	//get the desired variable
	Toy_Literal varIdn = TOY_TO_IDENTIFIER_LITERAL(Toy_copyRefString(TOY_AS_STRING(varName)));
	Toy_Literal result = TOY_TO_NULL_LITERAL;
	Toy_getScopeVariable(runner->interpreter.scope, varIdn, &result);

	Toy_pushLiteralArray(&interpreter->stack, result);

	//cleanup
	Toy_freeLiteral(result);
	Toy_freeLiteral(varIdn);
	Toy_freeLiteral(varName);
	Toy_freeLiteral(runnerLiteral);

	return 1;
}

static int nativeCallScriptFn(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count < 2) {
		interpreter->errorOutput("Incorrect number of arguments to _callScriptFn\n");
		return -1;
	}

	//get the rest args
	Toy_LiteralArray tmp;
	Toy_initLiteralArray(&tmp);

	while (arguments->count > 2) {
		Toy_Literal lit = Toy_popLiteralArray(arguments);
		Toy_pushLiteralArray(&tmp, lit);
		Toy_freeLiteral(lit);
	}

	Toy_LiteralArray rest;
	Toy_initLiteralArray(&rest);

	while (tmp.count) { //correct the order of the rest args
		Toy_Literal lit = Toy_popLiteralArray(&tmp);
		Toy_pushLiteralArray(&rest, lit);
		Toy_freeLiteral(lit);
	}

	Toy_freeLiteralArray(&tmp);


	//get the runner object
	Toy_Literal varName = Toy_popLiteralArray(arguments);
	Toy_Literal runnerLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal varNameIdn = varName;
	if (TOY_IS_IDENTIFIER(varName) && Toy_parseIdentifierToValue(interpreter, &varName)) {
		Toy_freeLiteral(varNameIdn);
	}

	Toy_Literal runnerIdn = runnerLiteral;
	if (TOY_IS_IDENTIFIER(runnerLiteral) && Toy_parseIdentifierToValue(interpreter, &runnerLiteral)) {
		Toy_freeLiteral(runnerIdn);
	}

	if (TOY_GET_OPAQUE_TAG(runnerLiteral) != TOY_OPAQUE_TAG_RUNNER) {
		interpreter->errorOutput("Unrecognized opaque literal in _runScript\n");
		return -1;
	}

	Toy_Runner* runner = TOY_AS_OPAQUE(runnerLiteral);

	//dirty check
	if (!runner->dirty) {
		interpreter->errorOutput("Can't access fn from a non-dirty script (try running it first)\n");
		Toy_freeLiteral(runnerLiteral);
		Toy_freeLiteralArray(&rest);
		return -1;
	}

	//get the desired variable
	Toy_Literal varIdn = TOY_TO_IDENTIFIER_LITERAL(Toy_copyRefString(TOY_AS_STRING(varName)));
	Toy_Literal fn = TOY_TO_NULL_LITERAL;
	Toy_getScopeVariable(runner->interpreter.scope, varIdn, &fn);

	if (!TOY_IS_FUNCTION(fn)) {
		interpreter->errorOutput("Can't run a non-function literal\n");
		Toy_freeLiteral(fn);
		Toy_freeLiteral(varIdn);
		Toy_freeLiteral(varName);
		Toy_freeLiteral(runnerLiteral);
		Toy_freeLiteralArray(&rest);
	}

	//call
	Toy_LiteralArray resultArray;
	Toy_initLiteralArray(&resultArray);

	Toy_callLiteralFn(interpreter, fn, &rest, &resultArray);

	Toy_Literal result = TOY_TO_NULL_LITERAL;
	if (resultArray.count > 0) {
		result = Toy_popLiteralArray(&resultArray);
	}

	Toy_pushLiteralArray(&interpreter->stack, result);

	//cleanup
	Toy_freeLiteralArray(&resultArray);
	Toy_freeLiteral(result);
	Toy_freeLiteral(fn);
	Toy_freeLiteral(varIdn);
	Toy_freeLiteral(varName);
	Toy_freeLiteral(runnerLiteral);
	Toy_freeLiteralArray(&rest);

	return 1;
}

static int nativeResetScript(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _resetScript\n");
		return -1;
	}

	//get the runner object
	Toy_Literal runnerLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal runnerIdn = runnerLiteral;
	if (TOY_IS_IDENTIFIER(runnerLiteral) && Toy_parseIdentifierToValue(interpreter, &runnerLiteral)) {
		Toy_freeLiteral(runnerIdn);
	}

	if (TOY_GET_OPAQUE_TAG(runnerLiteral) != TOY_OPAQUE_TAG_RUNNER) {
		interpreter->errorOutput("Unrecognized opaque literal in _runScript\n");
		return -1;
	}

	Toy_Runner* runner = TOY_AS_OPAQUE(runnerLiteral);

	//reset
	if (!runner->dirty) {
		interpreter->errorOutput("Can't reset a non-dirty script (try running it first)\n");
		Toy_freeLiteral(runnerLiteral);
		return -1;
	}

	Toy_resetInterpreter(&runner->interpreter);
	runner->dirty = false;
	Toy_freeLiteral(runnerLiteral);

	return 0;
}

static int nativeFreeScript(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _freeScript\n");
		return -1;
	}

	//get the runner object
	Toy_Literal runnerLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal runnerIdn = runnerLiteral;
	if (TOY_IS_IDENTIFIER(runnerLiteral) && Toy_parseIdentifierToValue(interpreter, &runnerLiteral)) {
		Toy_freeLiteral(runnerIdn);
	}

	if (TOY_GET_OPAQUE_TAG(runnerLiteral) != TOY_OPAQUE_TAG_RUNNER) {
		interpreter->errorOutput("Unrecognized opaque literal in _freeScript\n");
		return -1;
	}

	Toy_Runner* runner = TOY_AS_OPAQUE(runnerLiteral);

	//clear out the runner object
	runner->interpreter.hooks = NULL;
	Toy_freeInterpreter(&runner->interpreter);
	TOY_FREE_ARRAY(unsigned char, runner->bytecode, runner->size);

	TOY_FREE(Toy_Runner, runner);

	Toy_freeLiteral(runnerLiteral);

	return 0;
}

static int nativeCheckScriptDirty(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _runScript\n");
		return -1;
	}

	//get the runner object
	Toy_Literal runnerLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal runnerIdn = runnerLiteral;
	if (TOY_IS_IDENTIFIER(runnerLiteral) && Toy_parseIdentifierToValue(interpreter, &runnerLiteral)) {
		Toy_freeLiteral(runnerIdn);
	}

	if (TOY_GET_OPAQUE_TAG(runnerLiteral) != TOY_OPAQUE_TAG_RUNNER) {
		interpreter->errorOutput("Unrecognized opaque literal in _runScript\n");
		return -1;
	}

	Toy_Runner* runner = TOY_AS_OPAQUE(runnerLiteral);

	//run
	Toy_Literal result = TOY_TO_BOOLEAN_LITERAL(runner->dirty);

	Toy_pushLiteralArray(&interpreter->stack, result);

	//cleanup
	Toy_freeLiteral(result);
	Toy_freeLiteral(runnerLiteral);

	return 0;
}

//call the hook
typedef struct Natives {
	char* name;
	Toy_NativeFn fn;
} Natives;

int Toy_hookRunner(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias) {
	//build the natives list
	Natives natives[] = {
		{"loadScript", nativeLoadScript},
		{"loadScriptBytecode", nativeLoadScriptBytecode},
		{"_runScript", nativeRunScript},
		{"_getScriptVar", nativeGetScriptVar},
		{"_callScriptFn", nativeCallScriptFn},
		{"_resetScript", nativeResetScript},
		{"_freeScript", nativeFreeScript},
		{"_checkScriptDirty", nativeCheckScriptDirty},
		{NULL, NULL}
	};

	//store the library in an aliased dictionary
	if (!TOY_IS_NULL(alias)) {
		//make sure the name isn't taken
		if (Toy_isDelcaredScopeVariable(interpreter->scope, alias)) {
			interpreter->errorOutput("Can't override an existing variable\n");
			Toy_freeLiteral(alias);
			return false;
		}

		//create the dictionary to load up with functions
		Toy_LiteralDictionary* dictionary = TOY_ALLOCATE(Toy_LiteralDictionary, 1);
		Toy_initLiteralDictionary(dictionary);

		//load the dict with functions
		for (int i = 0; natives[i].name; i++) {
			Toy_Literal name = TOY_TO_STRING_LITERAL(Toy_createRefString(natives[i].name));
			Toy_Literal func = TOY_TO_FUNCTION_LITERAL((void*)natives[i].fn, 0);
			func.type = TOY_LITERAL_FUNCTION_NATIVE;

			Toy_setLiteralDictionary(dictionary, name, func);

			Toy_freeLiteral(name);
			Toy_freeLiteral(func);
		}

		//build the type
		Toy_Literal type = TOY_TO_TYPE_LITERAL(TOY_LITERAL_DICTIONARY, true);
		Toy_Literal strType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_STRING, true);
		Toy_Literal fnType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_FUNCTION_NATIVE, true);
		TOY_TYPE_PUSH_SUBTYPE(&type, strType);
		TOY_TYPE_PUSH_SUBTYPE(&type, fnType);

		//set scope
		Toy_Literal dict = TOY_TO_DICTIONARY_LITERAL(dictionary);
		Toy_declareScopeVariable(interpreter->scope, alias, type);
		Toy_setScopeVariable(interpreter->scope, alias, dict, false);

		//cleanup
		Toy_freeLiteral(dict);
		Toy_freeLiteral(type);
		return 0;
	}

	//default
	for (int i = 0; natives[i].name; i++) {
		Toy_injectNativeFn(interpreter, natives[i].name, natives[i].fn);
	}

	return 0;
}

//file system API
static Toy_LiteralDictionary Toy_driveDictionary;

void Toy_initDriveDictionary() {
	Toy_initLiteralDictionary(&Toy_driveDictionary);
}

void Toy_freeDriveDictionary() {
	Toy_freeLiteralDictionary(&Toy_driveDictionary);
}

Toy_LiteralDictionary* Toy_getDriveDictionary() {
	return &Toy_driveDictionary;
}

Toy_Literal Toy_getFilePathLiteral(Toy_Interpreter* interpreter, Toy_Literal* drivePathLiteral) {
	//check argument types
	if (!TOY_IS_STRING(*drivePathLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to Toy_getFilePathLiteral\n");
		return TOY_TO_NULL_LITERAL;
	}

	Toy_RefString* drivePath = Toy_copyRefString(TOY_AS_STRING(*drivePathLiteral));

	//get the drive and path as a string (can't trust that pesky strtok - custom split) TODO: move this to refstring library
	int driveLength = 0;
	while (Toy_toCString(drivePath)[driveLength] != ':') {
		if (driveLength >= Toy_lengthRefString(drivePath)) {
			interpreter->errorOutput("Incorrect drive path format given to Toy_getFilePathLiteral\n");

			return TOY_TO_NULL_LITERAL;
		}

		driveLength++;
	}

	Toy_RefString* drive = Toy_createRefStringLength(Toy_toCString(drivePath), driveLength);
	Toy_RefString* path = Toy_createRefStringLength( &Toy_toCString(drivePath)[driveLength + 1], Toy_lengthRefString(drivePath) - driveLength );

	//get the real drive file path
	Toy_Literal driveLiteral = TOY_TO_STRING_LITERAL(drive); //NOTE: driveLiteral takes ownership of the refString
	Toy_Literal realDriveLiteral = Toy_getLiteralDictionary(Toy_getDriveDictionary(), driveLiteral);

	if (!TOY_IS_STRING(realDriveLiteral)) {
		interpreter->errorOutput("Incorrect literal type found for drive: ");
		Toy_printLiteralCustom(realDriveLiteral, interpreter->errorOutput);
		interpreter->errorOutput("\n");
		Toy_freeLiteral(realDriveLiteral);
		Toy_freeLiteral(driveLiteral);
		Toy_deleteRefString(path);
		Toy_deleteRefString(drivePath);

		return TOY_TO_NULL_LITERAL;
	}

	//get the final real file path (concat) TODO: move this concat to refstring library
	Toy_RefString* realDrive = Toy_copyRefString(TOY_AS_STRING(realDriveLiteral));
	int realLength = Toy_lengthRefString(realDrive) + Toy_lengthRefString(path);

	char* filePath = TOY_ALLOCATE(char, realLength + 1); //+1 for null
	snprintf(filePath, realLength, "%s%s", Toy_toCString(realDrive), Toy_toCString(path));

	//clean up the drivepath stuff
	Toy_deleteRefString(realDrive);
	Toy_freeLiteral(realDriveLiteral);
	Toy_freeLiteral(driveLiteral);
	Toy_deleteRefString(path);
	Toy_deleteRefString(drivePath);

	Toy_Literal result = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(filePath, realLength));

	TOY_FREE_ARRAY(char, filePath, realLength + 1);

	return result;
}