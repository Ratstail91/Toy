#include "lib_timer.h"

#include "toy_common.h"
#include "memory.h"

#include <stdio.h>

//god damn it
static struct timeval* diff(struct timeval* lhs, struct timeval* rhs) {
	struct timeval* d = ALLOCATE(struct timeval, 1);

	d->tv_sec = rhs->tv_sec - lhs->tv_sec;
	d->tv_usec = rhs->tv_usec - lhs->tv_usec;

	if (d->tv_usec < 0) {
		d->tv_sec--;
		d->tv_usec += 1000 * 1000;
	}

	return d;
}

static int nativeStartTimer(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 0) {
		interpreter->errorOutput("Incorrect number of arguments to startTimer\n");
		return -1;
	}

	//get the timeinfo from C
	struct timeval* timeinfo = ALLOCATE(struct timeval, 1);
	gettimeofday(timeinfo, NULL);

	//wrap in an opaque literal for Toy
	Literal timeLiteral = TO_OPAQUE_LITERAL(timeinfo, -1);
	pushLiteralArray(&interpreter->stack, timeLiteral);

	freeLiteral(timeLiteral);

	return 1;
}

static int nativeStopTimer(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _stopTimer\n");
		return -1;
	}

	//get the timeinfo from C
	struct timeval timerStop;
	gettimeofday(&timerStop, NULL);

	//unwrap the opaque literal
	Literal timeLiteral = popLiteralArray(arguments);

	Literal timeLiteralIdn = timeLiteral;
	if (IS_IDENTIFIER(timeLiteral) && parseIdentifierToValue(interpreter, &timeLiteral)) {
		freeLiteral(timeLiteralIdn);
	}

	if (!IS_OPAQUE(timeLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _stopTimer\n");
		freeLiteral(timeLiteral);
		return -1;
	}

	struct timeval* timerStart = AS_OPAQUE(timeLiteral);

	//determine the difference, and wrap it
	struct timeval* d = diff(timerStart, &timerStop);
	Literal diffLiteral = TO_OPAQUE_LITERAL(d, -1);
	pushLiteralArray(&interpreter->stack, diffLiteral);

	//cleanup
	freeLiteral(timeLiteral);
	freeLiteral(diffLiteral);

	return 1;
}

static int nativeCreateTimer(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to createTimer\n");
		return -1;
	}

	//get the args
	Literal microsecondLiteral = popLiteralArray(arguments);
	Literal secondLiteral = popLiteralArray(arguments);

	Literal secondLiteralIdn = secondLiteral;
	if (IS_IDENTIFIER(secondLiteral) && parseIdentifierToValue(interpreter, &secondLiteral)) {
		freeLiteral(secondLiteralIdn);
	}

	Literal microsecondLiteralIdn = microsecondLiteral;
	if (IS_IDENTIFIER(microsecondLiteral) && parseIdentifierToValue(interpreter, &microsecondLiteral)) {
		freeLiteral(microsecondLiteralIdn);
	}

	if (!IS_INTEGER(secondLiteral) || !IS_INTEGER(microsecondLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to createTimer\n");
		freeLiteral(secondLiteral);
		freeLiteral(microsecondLiteral);
		return -1;
	}

	//get the timeinfo from toy
	struct timeval* timeinfo = ALLOCATE(struct timeval, 1);
	timeinfo->tv_sec = AS_INTEGER(secondLiteral);
	timeinfo->tv_usec = AS_INTEGER(microsecondLiteral);

	//wrap in an opaque literal for Toy
	Literal timeLiteral = TO_OPAQUE_LITERAL(timeinfo, -1);
	pushLiteralArray(&interpreter->stack, timeLiteral);

	freeLiteral(timeLiteral);
	freeLiteral(secondLiteral);
	freeLiteral(microsecondLiteral);

	return 1;
}

static int nativeGetTimerSeconds(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _getTimerSeconds\n");
		return -1;
	}

	//unwrap the opaque literal
	Literal timeLiteral = popLiteralArray(arguments);

	Literal timeLiteralIdn = timeLiteral;
	if (IS_IDENTIFIER(timeLiteral) && parseIdentifierToValue(interpreter, &timeLiteral)) {
		freeLiteral(timeLiteralIdn);
	}

	if (!IS_OPAQUE(timeLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _getTimerSeconds\n");
		freeLiteral(timeLiteral);
		return -1;
	}

	struct timeval* timer = AS_OPAQUE(timeLiteral);

	//create the result literal
	Literal result = TO_INTEGER_LITERAL(timer->tv_sec);
	pushLiteralArray(&interpreter->stack, result);

	//cleanup
	freeLiteral(timeLiteral);
	freeLiteral(result);

	return 1;
}

static int nativeGetTimerMicroseconds(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _getTimerMicroseconds\n");
		return -1;
	}

	//unwrap the opaque literal
	Literal timeLiteral = popLiteralArray(arguments);

	Literal timeLiteralIdn = timeLiteral;
	if (IS_IDENTIFIER(timeLiteral) && parseIdentifierToValue(interpreter, &timeLiteral)) {
		freeLiteral(timeLiteralIdn);
	}

	if (!IS_OPAQUE(timeLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _getTimerMicroseconds\n");
		freeLiteral(timeLiteral);
		return -1;
	}

	struct timeval* timer = AS_OPAQUE(timeLiteral);

	//create the result literal
	Literal result = TO_INTEGER_LITERAL(timer->tv_usec);
	pushLiteralArray(&interpreter->stack, result);

	//cleanup
	freeLiteral(timeLiteral);
	freeLiteral(result);

	return 1;
}

static int nativeCompareTimer(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to _compareTimer\n");
		return -1;
	}

	//unwrap the opaque literals
	Literal rhsLiteral = popLiteralArray(arguments);
	Literal lhsLiteral = popLiteralArray(arguments);

	Literal lhsLiteralIdn = lhsLiteral;
	if (IS_IDENTIFIER(lhsLiteral) && parseIdentifierToValue(interpreter, &lhsLiteral)) {
		freeLiteral(lhsLiteralIdn);
	}

	Literal rhsLiteralIdn = rhsLiteral;
	if (IS_IDENTIFIER(rhsLiteral) && parseIdentifierToValue(interpreter, &rhsLiteral)) {
		freeLiteral(rhsLiteralIdn);
	}

	if (!IS_OPAQUE(lhsLiteral) || !IS_OPAQUE(rhsLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _compareTimer\n");
		freeLiteral(lhsLiteral);
		freeLiteral(rhsLiteral);
		return -1;
	}

	struct timeval* lhsTimer = AS_OPAQUE(lhsLiteral);
	struct timeval* rhsTimer = AS_OPAQUE(rhsLiteral);

	//determine the difference, and wrap it
	struct timeval* d = diff(lhsTimer, rhsTimer);
	Literal diffLiteral = TO_OPAQUE_LITERAL(d, -1);
	pushLiteralArray(&interpreter->stack, diffLiteral);

	//cleanup
	freeLiteral(lhsLiteral);
	freeLiteral(rhsLiteral);
	freeLiteral(diffLiteral);

	return 1;
}

static int nativeTimerToString(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _timerToString\n");
		return -1;
	}

	//unwrap in an opaque literal
	Literal timeLiteral = popLiteralArray(arguments);

	Literal timeLiteralIdn = timeLiteral;
	if (IS_IDENTIFIER(timeLiteral) && parseIdentifierToValue(interpreter, &timeLiteral)) {
		freeLiteral(timeLiteralIdn);
	}

	if (!IS_OPAQUE(timeLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _timerToString\n");
		freeLiteral(timeLiteral);
		return -1;
	}

	struct timeval* timer = AS_OPAQUE(timeLiteral);

	//create the string literal
	char buffer[128];
	snprintf(buffer, 128, "%ld.%06ld", timer->tv_sec, timer->tv_usec);
	Literal resultLiteral = TO_STRING_LITERAL( copyString(buffer, strlen(buffer)), strlen(buffer));

	pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	freeLiteral(timeLiteral);
	freeLiteral(resultLiteral);

	return 1;
}

static int nativeDestroyTimer(Interpreter* interpreter, LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _destroyTimer\n");
		return -1;
	}

		//unwrap in an opaque literal
	Literal timeLiteral = popLiteralArray(arguments);

	Literal timeLiteralIdn = timeLiteral;
	if (IS_IDENTIFIER(timeLiteral) && parseIdentifierToValue(interpreter, &timeLiteral)) {
		freeLiteral(timeLiteralIdn);
	}

	if (!IS_OPAQUE(timeLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _destroyTimer\n");
		freeLiteral(timeLiteral);
		return -1;
	}

	struct timeval* timer = AS_OPAQUE(timeLiteral);

	FREE(struct timeval, timer);

	freeLiteral(timeLiteral);

	return 0;
}

//call the hook
typedef struct Natives {
	char* name;
	NativeFn fn;
} Natives;

int hookTimer(Interpreter* interpreter, Literal identifier, Literal alias) {
	//build the natives list
	Natives natives[] = {
		{"startTimer", nativeStartTimer},
		{"_stopTimer", nativeStopTimer},
		{"createTimer", nativeCreateTimer},
		{"_getTimerSeconds", nativeGetTimerSeconds},
		{"_getTimerMicroseconds", nativeGetTimerMicroseconds},
		{"_compareTimer", nativeCompareTimer},
		{"_timerToString", nativeTimerToString},
		{"_destroyTimer", nativeDestroyTimer},
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
			Literal name = TO_STRING_LITERAL(copyString(natives[i].name, strlen(natives[i].name)), strlen(natives[i].name));
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
