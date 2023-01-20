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

	//
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
	initInterpreter(&runner->interpreter);
	runner->bytecode = bytecode;
	runner->size = fileSize;

	//build the opaque object, and push it to the stack
	Literal runnerLiteral = TO_OPAQUE_LITERAL(runner, OPAQUE_TAG_RUNNER);
	pushLiteralArray(&interpreter->stack, runnerLiteral);

	FREE_ARRAY(char, filePath, realLength);

	return 1;
}

static int nativeRunScript(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 0) {
		interpreter->errorOutput("Incorrect number of arguments to _runScript\n");
		return -1;
	}

	//

	return -1;
}

static int nativeGetScriptVar(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 0) {
		interpreter->errorOutput("Incorrect number of arguments to _getScriptVar\n");
		return -1;
	}

	//

	return -1;
}

static int nativeCallScriptFn(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 0) {
		interpreter->errorOutput("Incorrect number of arguments to _callScriptFn\n");
		return -1;
	}

	//

	return -1;
}

static int nativeResetScript(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 0) {
		interpreter->errorOutput("Incorrect number of arguments to _resetScript\n");
		return -1;
	}

	//

	return -1;
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
	freeInterpreter(&runner->interpreter);
	FREE_ARRAY(unsigned char, runner->bytecode, runner->size);

	FREE(Runner, runner);

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
		{"x_runScript", nativeRunScript},
		{"x_getScriptVar", nativeGetScriptVar},
		{"x_callScriptFn", nativeCallScriptFn},
		{"x_resetScript", nativeResetScript},
		{"_freeScript", nativeFreeScript},
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