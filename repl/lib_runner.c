#include "lib_runner.h"

#include "memory.h"
#include "interpreter.h"

#include "repl_tools.h"

#include <stdio.h>
#include <stdlib.h>

typedef struct Runner {
	Interpreter interpreter;
	unsigned char* bytecode;
	size_t size;

	bool dirty;
} Runner;

//Toy native functions
static int nativeLoadScript(Interpreter* interpreter, LiteralArray* arguments) {
	//arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to loadScript\n");
		return -1;
	}

	//get the argument
	Literal drivePathLiteral = popLiteralArray(arguments);
	RefString* drivePath = copyRefString(AS_STRING(drivePathLiteral));

	//get the drive and path as a string (can't trust that pesky strtok - custom split) TODO: move this to refstring library
	int driveLength = 0;
	while (toCString(drivePath)[driveLength] != ':') {
		if (driveLength >= lengthRefString(drivePath)) {
			interpreter->errorOutput("Incorrect drive path format given to loadScript\n");
			deleteRefString(drivePath);
			freeLiteral(drivePathLiteral);
			return -1;
		}

		driveLength++;
	}

	RefString* drive = createRefStringLength(toCString(drivePath), driveLength);
	RefString* path = createRefStringLength( &toCString(drivePath)[driveLength + 1], lengthRefString(drivePath) - driveLength );

	//get the real drive file path
	Literal driveLiteral = TO_STRING_LITERAL(drive); //NOTE: driveLiteral takes ownership of the refString
	Literal realDriveLiteral = getLiteralDictionary(getDriveDictionary(), driveLiteral);

	if (!IS_STRING(realDriveLiteral)) {
		interpreter->errorOutput("Incorrect literal type found for drive: ");
		printLiteralCustom(realDriveLiteral, interpreter->errorOutput);
		interpreter->errorOutput("\n");
		freeLiteral(realDriveLiteral);
		freeLiteral(driveLiteral);
		deleteRefString(path);
		deleteRefString(drivePath);
		freeLiteral(drivePathLiteral);
		return -1;
	}

	//get the final real file path (concat) TODO: move this concat to refstring library
	RefString* realDrive = copyRefString(AS_STRING(realDriveLiteral));
	int realLength = lengthRefString(realDrive) + lengthRefString(path);

	char* filePath = ALLOCATE(char, realLength + 1); //+1 for null
	snprintf(filePath, realLength, "%s%s", toCString(realDrive), toCString(path));

	//clean up the drivepath stuff
	deleteRefString(realDrive);
	freeLiteral(realDriveLiteral);
	freeLiteral(driveLiteral);
	deleteRefString(path);
	deleteRefString(drivePath);
	freeLiteral(drivePathLiteral);

	//check for file extensions
	if (!(filePath[realLength - 5] == '.' && filePath[realLength - 4] == 't' && filePath[realLength - 3] == 'o' && filePath[realLength - 2] == 'y')) {
		interpreter->errorOutput("Bad script file extension (expected .toy)\n");
		FREE_ARRAY(char, filePath, realLength);
		return -1;
	}

	//check for break-out attempts
	for (int i = 0; i < realLength - 1; i++) {
		if (filePath[i] == '.' && filePath[i + 1] == '.') {
			interpreter->errorOutput("Parent directory access not allowed\n");
			FREE_ARRAY(char, filePath, realLength);
			return -1;
		}
	}

	//load and compile the bytecode
	size_t fileSize = 0;
	char* source = readFile(filePath, &fileSize);

	if (!source) {
		interpreter->errorOutput("Failed to load source file\n");
		return -1;
	}

	unsigned char* bytecode = compileString(source, &fileSize);
	free((void*)source);

	if (!bytecode) {
		interpreter->errorOutput("Failed to compile source file\n");
		return -1;
	}

	//build the runner object
	Runner* runner = ALLOCATE(Runner, 1);
	setInterpreterPrint(&runner->interpreter, interpreter->printOutput);
	setInterpreterAssert(&runner->interpreter, interpreter->assertOutput);
	setInterpreterError(&runner->interpreter, interpreter->errorOutput);
	runner->interpreter.hooks = interpreter->hooks;
	runner->interpreter.scope = NULL;
	resetInterpreter(&runner->interpreter);
	runner->bytecode = bytecode;
	runner->size = fileSize;
	runner->dirty = false;

	//build the opaque object, and push it to the stack
	Literal runnerLiteral = TO_OPAQUE_LITERAL(runner, OPAQUE_TAG_RUNNER);
	pushLiteralArray(&interpreter->stack, runnerLiteral);

	FREE_ARRAY(char, filePath, realLength);

	return 1;
}

static int nativeLoadScriptBytecode(Interpreter* interpreter, LiteralArray* arguments) {
	//arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to loadScriptBytecode\n");
		return -1;
	}

	//get the argument
	Literal drivePathLiteral = popLiteralArray(arguments);
	RefString* drivePath = copyRefString(AS_STRING(drivePathLiteral));

	//get the drive and path as a string (can't trust that pesky strtok - custom split) TODO: move this to refstring library
	int driveLength = 0;
	while (toCString(drivePath)[driveLength] != ':') {
		if (driveLength >= lengthRefString(drivePath)) {
			interpreter->errorOutput("Incorrect drive path format given to loadScriptBytecode\n");
			deleteRefString(drivePath);
			freeLiteral(drivePathLiteral);
			return -1;
		}

		driveLength++;
	}

	RefString* drive = createRefStringLength(toCString(drivePath), driveLength);
	RefString* path = createRefStringLength( &toCString(drivePath)[driveLength + 1], lengthRefString(drivePath) - driveLength );

	//get the real drive file path
	Literal driveLiteral = TO_STRING_LITERAL(drive); //NOTE: driveLiteral takes ownership of the refString
	Literal realDriveLiteral = getLiteralDictionary(getDriveDictionary(), driveLiteral);

	if (!IS_STRING(realDriveLiteral)) {
		interpreter->errorOutput("Incorrect literal type found for drive: ");
		printLiteralCustom(realDriveLiteral, interpreter->errorOutput);
		interpreter->errorOutput("\n");
		freeLiteral(realDriveLiteral);
		freeLiteral(driveLiteral);
		deleteRefString(path);
		deleteRefString(drivePath);
		freeLiteral(drivePathLiteral);
		return -1;
	}

	//get the final real file path (concat) TODO: move this concat to refstring library
	RefString* realDrive = copyRefString(AS_STRING(realDriveLiteral));
	int realLength = lengthRefString(realDrive) + lengthRefString(path);

	char* filePath = ALLOCATE(char, realLength + 1); //+1 for null
	snprintf(filePath, realLength, "%s%s", toCString(realDrive), toCString(path));

	//clean up the drivepath stuff
	deleteRefString(realDrive);
	freeLiteral(realDriveLiteral);
	freeLiteral(driveLiteral);
	deleteRefString(path);
	deleteRefString(drivePath);
	freeLiteral(drivePathLiteral);

	//check for file extensions
	if (!(filePath[realLength - 4] == '.' && filePath[realLength - 3] == 't' && filePath[realLength - 2] == 'b')) {
		interpreter->errorOutput("Bad binary file extension (expected .tb)\n");
		FREE_ARRAY(char, filePath, realLength);
		return -1;
	}

	//check for break-out attempts
	for (int i = 0; i < realLength - 1; i++) {
		if (filePath[i] == '.' && filePath[i + 1] == '.') {
			interpreter->errorOutput("Parent directory access not allowed\n");
			FREE_ARRAY(char, filePath, realLength);
			return -1;
		}
	}

	//load the bytecode
	size_t fileSize = 0;
	unsigned char* bytecode = (unsigned char*)readFile(filePath, &fileSize);

	if (!bytecode) {
		interpreter->errorOutput("Failed to load bytecode file\n");
		return -1;
	}

	//build the runner object
	Runner* runner = ALLOCATE(Runner, 1);
	setInterpreterPrint(&runner->interpreter, interpreter->printOutput);
	setInterpreterAssert(&runner->interpreter, interpreter->assertOutput);
	setInterpreterError(&runner->interpreter, interpreter->errorOutput);
	runner->interpreter.hooks = interpreter->hooks;
	runner->interpreter.scope = NULL;
	resetInterpreter(&runner->interpreter);
	runner->bytecode = bytecode;
	runner->size = fileSize;
	runner->dirty = false;

	//build the opaque object, and push it to the stack
	Literal runnerLiteral = TO_OPAQUE_LITERAL(runner, OPAQUE_TAG_RUNNER);
	pushLiteralArray(&interpreter->stack, runnerLiteral);

	FREE_ARRAY(char, filePath, realLength);

	return 1;
}

static int nativeRunScript(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _runScript\n");
		return -1;
	}

	//get the runner object
	Literal runnerLiteral = popLiteralArray(arguments);
	Literal idn = runnerLiteral;

	if (parseIdentifierToValue(interpreter, &runnerLiteral)) {
		freeLiteral(idn);
	}

	if (OPAQUE_TAG(runnerLiteral) != OPAQUE_TAG_RUNNER) {
		interpreter->errorOutput("Unrecognized opaque literal in _runScript\n");
		return -1;
	}

	Runner* runner = AS_OPAQUE(runnerLiteral);

	//run
	if (runner->dirty) {
		interpreter->errorOutput("Can't re-run a dirty script (try resetting it first)\n");
		freeLiteral(runnerLiteral);
		return -1;
	}

	unsigned char* bytecodeCopy = ALLOCATE(unsigned char, runner->size);
	memcpy(bytecodeCopy, runner->bytecode, runner->size); //need a COPY of the bytecode, because the interpreter eats it

	runInterpreter(&runner->interpreter, bytecodeCopy, runner->size);
	runner->dirty = true;

	//cleanup
	freeLiteral(runnerLiteral);

	return 0;
}

static int nativeGetScriptVar(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to _getScriptVar\n");
		return -1;
	}

	//get the runner object
	Literal varName = popLiteralArray(arguments);
	Literal runnerLiteral = popLiteralArray(arguments);

	if (IS_IDENTIFIER(varName)) {
		Literal idn = varName;
		parseIdentifierToValue(interpreter, &varName);
		freeLiteral(idn);
	}

	if (IS_IDENTIFIER(runnerLiteral)) {
		Literal idn = runnerLiteral;
		parseIdentifierToValue(interpreter, &runnerLiteral);
		freeLiteral(idn);
	}

	if (OPAQUE_TAG(runnerLiteral) != OPAQUE_TAG_RUNNER) {
		interpreter->errorOutput("Unrecognized opaque literal in _runScript\n");
		return -1;
	}

	Runner* runner = AS_OPAQUE(runnerLiteral);

	//dirty check
	if (!runner->dirty) {
		interpreter->errorOutput("Can't access variable from a non-dirty script (try running it first)\n");
		freeLiteral(runnerLiteral);
		return -1;
	}

	//get the desired variable
	Literal varIdn = TO_IDENTIFIER_LITERAL(copyRefString(AS_STRING(varName)));
	Literal result = TO_NULL_LITERAL;
	getScopeVariable(runner->interpreter.scope, varIdn, &result);

	pushLiteralArray(&interpreter->stack, result);

	//cleanup
	freeLiteral(result);
	freeLiteral(varIdn);
	freeLiteral(varName);
	freeLiteral(runnerLiteral);

	return 1;
}

static int nativeCallScriptFn(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count < 2) {
		interpreter->errorOutput("Incorrect number of arguments to _callScriptFn\n");
		return -1;
	}

	//get the rest args
	LiteralArray tmp;
	initLiteralArray(&tmp);

	while (arguments->count > 2) {
		Literal lit = popLiteralArray(arguments);
		pushLiteralArray(&tmp, lit);
		freeLiteral(lit);
	}

	LiteralArray rest;
	initLiteralArray(&rest);

	while (tmp.count) { //correct the order of the rest args
		Literal lit = popLiteralArray(&tmp);
		pushLiteralArray(&rest, lit);
		freeLiteral(lit);
	}

	freeLiteralArray(&tmp);


	//get the runner object
	Literal varName = popLiteralArray(arguments);
	Literal runnerLiteral = popLiteralArray(arguments);

	if (IS_IDENTIFIER(varName)) {
		Literal idn = varName;
		parseIdentifierToValue(interpreter, &varName);
		freeLiteral(idn);
	}

	if (IS_IDENTIFIER(runnerLiteral)) {
		Literal idn = runnerLiteral;
		parseIdentifierToValue(interpreter, &runnerLiteral);
		freeLiteral(idn);
	}

	if (OPAQUE_TAG(runnerLiteral) != OPAQUE_TAG_RUNNER) {
		interpreter->errorOutput("Unrecognized opaque literal in _runScript\n");
		return -1;
	}

	Runner* runner = AS_OPAQUE(runnerLiteral);

	//dirty check
	if (!runner->dirty) {
		interpreter->errorOutput("Can't access fn from a non-dirty script (try running it first)\n");
		freeLiteral(runnerLiteral);
		freeLiteralArray(&rest);
		return -1;
	}

	//get the desired variable
	Literal varIdn = TO_IDENTIFIER_LITERAL(copyRefString(AS_STRING(varName)));
	Literal fn = TO_NULL_LITERAL;
	getScopeVariable(runner->interpreter.scope, varIdn, &fn);

	if (!IS_FUNCTION(fn)) {
		interpreter->errorOutput("Can't run a non-function literal\n");
		freeLiteral(fn);
		freeLiteral(varIdn);
		freeLiteral(varName);
		freeLiteral(runnerLiteral);
		freeLiteralArray(&rest);
	}

	//call
	LiteralArray resultArray;
	initLiteralArray(&resultArray);

	callLiteralFn(interpreter, fn, &rest, &resultArray);

	Literal result = TO_NULL_LITERAL;
	if (resultArray.count > 0) {
		result = popLiteralArray(&resultArray);
	}

	pushLiteralArray(&interpreter->stack, result);

	//cleanup
	freeLiteralArray(&resultArray);
	freeLiteral(result);
	freeLiteral(fn);
	freeLiteral(varIdn);
	freeLiteral(varName);
	freeLiteral(runnerLiteral);
	freeLiteralArray(&rest);

	return 1;
}

static int nativeResetScript(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _resetScript\n");
		return -1;
	}

	//get the runner object
	Literal runnerLiteral = popLiteralArray(arguments);

	if (IS_IDENTIFIER(runnerLiteral)) {
		Literal idn = runnerLiteral;
		parseIdentifierToValue(interpreter, &runnerLiteral);
		freeLiteral(idn);
	}

	if (OPAQUE_TAG(runnerLiteral) != OPAQUE_TAG_RUNNER) {
		interpreter->errorOutput("Unrecognized opaque literal in _runScript\n");
		return -1;
	}

	Runner* runner = AS_OPAQUE(runnerLiteral);

	//reset
	if (!runner->dirty) {
		interpreter->errorOutput("Can't reset a non-dirty script (try running it first)\n");
		freeLiteral(runnerLiteral);
		return -1;
	}

	resetInterpreter(&runner->interpreter);
	runner->dirty = false;
	freeLiteral(runnerLiteral);

	return 0;
}

static int nativeFreeScript(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _freeScript\n");
		return -1;
	}

	//get the runner object
	Literal runnerLiteral = popLiteralArray(arguments);

	if (IS_IDENTIFIER(runnerLiteral)) {
		Literal idn = runnerLiteral;
		parseIdentifierToValue(interpreter, &runnerLiteral);
		freeLiteral(idn);
	}

	if (OPAQUE_TAG(runnerLiteral) != OPAQUE_TAG_RUNNER) {
		interpreter->errorOutput("Unrecognized opaque literal in _freeScript\n");
		return -1;
	}

	Runner* runner = AS_OPAQUE(runnerLiteral);

	//clear out the runner object
	runner->interpreter.hooks = NULL;
	freeInterpreter(&runner->interpreter);
	FREE_ARRAY(unsigned char, runner->bytecode, runner->size);

	FREE(Runner, runner);

	freeLiteral(runnerLiteral);

	return 0;
}

static int nativeCheckScriptDirty(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _runScript\n");
		return -1;
	}

	//get the runner object
	Literal runnerLiteral = popLiteralArray(arguments);

	if (IS_IDENTIFIER(runnerLiteral)) {
		Literal idn = runnerLiteral;
		parseIdentifierToValue(interpreter, &runnerLiteral);
		freeLiteral(idn);
	}

	if (OPAQUE_TAG(runnerLiteral) != OPAQUE_TAG_RUNNER) {
		interpreter->errorOutput("Unrecognized opaque literal in _runScript\n");
		return -1;
	}

	Runner* runner = AS_OPAQUE(runnerLiteral);

	//run
	Literal result = TO_BOOLEAN_LITERAL(runner->dirty);

	pushLiteralArray(&interpreter->stack, result);

	//cleanup
	freeLiteral(result);
	freeLiteral(runnerLiteral);

	return 0;
}

//call the hook
typedef struct Natives {
	char* name;
	NativeFn fn;
} Natives;

int hookRunner(Interpreter* interpreter, Literal identifier, Literal alias) {
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
	if (!IS_NULL(alias)) {
		//make sure the name isn't taken
		if (isDelcaredScopeVariable(interpreter->scope, alias)) {
			interpreter->errorOutput("Can't override an existing variable\n");
			freeLiteral(alias);
			return false;
		}

		//create the dictionary to load up with functions
		LiteralDictionary* dictionary = ALLOCATE(LiteralDictionary, 1);
		initLiteralDictionary(dictionary);

		//load the dict with functions
		for (int i = 0; natives[i].name; i++) {
			Literal name = TO_STRING_LITERAL(createRefString(natives[i].name));
			Literal func = TO_FUNCTION_LITERAL((void*)natives[i].fn, 0);
			func.type = LITERAL_FUNCTION_NATIVE;

			setLiteralDictionary(dictionary, name, func);

			freeLiteral(name);
			freeLiteral(func);
		}

		//build the type
		Literal type = TO_TYPE_LITERAL(LITERAL_DICTIONARY, true);
		Literal strType = TO_TYPE_LITERAL(LITERAL_STRING, true);
		Literal fnType = TO_TYPE_LITERAL(LITERAL_FUNCTION_NATIVE, true);
		TYPE_PUSH_SUBTYPE(&type, strType);
		TYPE_PUSH_SUBTYPE(&type, fnType);

		//set scope
		Literal dict = TO_DICTIONARY_LITERAL(dictionary);
		declareScopeVariable(interpreter->scope, alias, type);
		setScopeVariable(interpreter->scope, alias, dict, false);

		//cleanup
		freeLiteral(dict);
		freeLiteral(type);
		return 0;
	}

	//default
	for (int i = 0; natives[i].name; i++) {
		injectNativeFn(interpreter, natives[i].name, natives[i].fn);
	}

	return 0;
}

//file system API
static LiteralDictionary driveDictionary;

void initDriveDictionary() {
	initLiteralDictionary(&driveDictionary);
}

void freeDriveDictionary() {
	freeLiteralDictionary(&driveDictionary);
}

LiteralDictionary* getDriveDictionary() {
	return &driveDictionary;
}