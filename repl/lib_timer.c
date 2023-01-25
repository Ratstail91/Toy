#include "lib_timer.h"

#include "toy_memory.h"

#include <stdio.h>
#include <time.h>
#include <sys/time.h>

//GOD DAMN IT: https://stackoverflow.com/questions/15846762/timeval-subtract-explanation
static int timeval_subtract(struct timeval *result, struct timeval *x, struct timeval *y) {
	//normallize
	if (x->tv_usec > 999999) {
		x->tv_sec += x->tv_usec / 1000000;
		x->tv_usec %= 1000000;
	}

	if (y->tv_usec > 999999) {
		y->tv_sec += y->tv_usec / 1000000;
		y->tv_usec %= 1000000;
	}

	//calc
	result->tv_sec = x->tv_sec - y->tv_sec;

	if ((result->tv_usec = x->tv_usec - y->tv_usec) < 0) {
		if (result->tv_sec != 0) { //only works far from 0
			result->tv_usec += 1000000;
			result->tv_sec--; // borrow
		}
	}

	return result->tv_sec < 0 || (result->tv_sec == 0 && result->tv_usec < 0);
}

//god damn it
static struct timeval* diff(struct timeval* lhs, struct timeval* rhs) {
	struct timeval* d = TOY_ALLOCATE(struct timeval, 1);

	//I gave up, copied from SO
	timeval_subtract(d, rhs, lhs);

	return d;
}

//callbacks
static int nativeStartTimer(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 0) {
		interpreter->errorOutput("Incorrect number of arguments to startTimer\n");
		return -1;
	}

	//get the timeinfo from C
	struct timeval* timeinfo = TOY_ALLOCATE(struct timeval, 1);
	gettimeofday(timeinfo, NULL);

	//wrap in an opaque literal for Toy
	Toy_Literal timeLiteral = TOY_TO_OPAQUE_LITERAL(timeinfo, -1);
	Toy_pushLiteralArray(&interpreter->stack, timeLiteral);

	Toy_freeLiteral(timeLiteral);

	return 1;
}

static int nativeStopTimer(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _stopTimer\n");
		return -1;
	}

	//get the timeinfo from C
	struct timeval timerStop;
	gettimeofday(&timerStop, NULL);

	//unwrap the opaque literal
	Toy_Literal timeLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal timeLiteralIdn = timeLiteral;
	if (TOY_IS_IDENTIFIER(timeLiteral) && Toy_parseIdentifierToValue(interpreter, &timeLiteral)) {
		Toy_freeLiteral(timeLiteralIdn);
	}

	if (!TOY_IS_OPAQUE(timeLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _stopTimer\n");
		Toy_freeLiteral(timeLiteral);
		return -1;
	}

	struct timeval* timerStart = TOY_AS_OPAQUE(timeLiteral);

	//determine the difference, and wrap it
	struct timeval* d = diff(timerStart, &timerStop);
	Toy_Literal diffLiteral = TOY_TO_OPAQUE_LITERAL(d, -1);
	Toy_pushLiteralArray(&interpreter->stack, diffLiteral);

	//cleanup
	Toy_freeLiteral(timeLiteral);
	Toy_freeLiteral(diffLiteral);

	return 1;
}

static int nativeCreateTimer(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to createTimer\n");
		return -1;
	}

	//get the args
	Toy_Literal microsecondLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal secondLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal secondLiteralIdn = secondLiteral;
	if (TOY_IS_IDENTIFIER(secondLiteral) && Toy_parseIdentifierToValue(interpreter, &secondLiteral)) {
		Toy_freeLiteral(secondLiteralIdn);
	}

	Toy_Literal microsecondLiteralIdn = microsecondLiteral;
	if (TOY_IS_IDENTIFIER(microsecondLiteral) && Toy_parseIdentifierToValue(interpreter, &microsecondLiteral)) {
		Toy_freeLiteral(microsecondLiteralIdn);
	}

	if (!TOY_IS_INTEGER(secondLiteral) || !TOY_IS_INTEGER(microsecondLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to createTimer\n");
		Toy_freeLiteral(secondLiteral);
		Toy_freeLiteral(microsecondLiteral);
		return -1;
	}

	if (TOY_AS_INTEGER(microsecondLiteral) <= -1000 * 1000 || TOY_AS_INTEGER(microsecondLiteral) >= 1000 * 1000 || (TOY_AS_INTEGER(secondLiteral) != 0 && TOY_AS_INTEGER(microsecondLiteral) < 0) ) {
		interpreter->errorOutput("Microseconds out of range in createTimer\n");
		Toy_freeLiteral(secondLiteral);
		Toy_freeLiteral(microsecondLiteral);
		return -1;
	}

	//get the timeinfo from toy
	struct timeval* timeinfo = TOY_ALLOCATE(struct timeval, 1);
	timeinfo->tv_sec = TOY_AS_INTEGER(secondLiteral);
	timeinfo->tv_usec = TOY_AS_INTEGER(microsecondLiteral);

	//wrap in an opaque literal for Toy
	Toy_Literal timeLiteral = TOY_TO_OPAQUE_LITERAL(timeinfo, -1);
	Toy_pushLiteralArray(&interpreter->stack, timeLiteral);

	Toy_freeLiteral(timeLiteral);
	Toy_freeLiteral(secondLiteral);
	Toy_freeLiteral(microsecondLiteral);

	return 1;
}

static int nativeGetTimerSeconds(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _getTimerSeconds\n");
		return -1;
	}

	//unwrap the opaque literal
	Toy_Literal timeLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal timeLiteralIdn = timeLiteral;
	if (TOY_IS_IDENTIFIER(timeLiteral) && Toy_parseIdentifierToValue(interpreter, &timeLiteral)) {
		Toy_freeLiteral(timeLiteralIdn);
	}

	if (!TOY_IS_OPAQUE(timeLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _getTimerSeconds\n");
		Toy_freeLiteral(timeLiteral);
		return -1;
	}

	struct timeval* timer = TOY_AS_OPAQUE(timeLiteral);

	//create the result literal
	Toy_Literal result = TOY_TO_INTEGER_LITERAL(timer->tv_sec);
	Toy_pushLiteralArray(&interpreter->stack, result);

	//cleanup
	Toy_freeLiteral(timeLiteral);
	Toy_freeLiteral(result);

	return 1;
}

static int nativeGetTimerMicroseconds(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _getTimerMicroseconds\n");
		return -1;
	}

	//unwrap the opaque literal
	Toy_Literal timeLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal timeLiteralIdn = timeLiteral;
	if (TOY_IS_IDENTIFIER(timeLiteral) && Toy_parseIdentifierToValue(interpreter, &timeLiteral)) {
		Toy_freeLiteral(timeLiteralIdn);
	}

	if (!TOY_IS_OPAQUE(timeLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _getTimerMicroseconds\n");
		Toy_freeLiteral(timeLiteral);
		return -1;
	}

	struct timeval* timer = TOY_AS_OPAQUE(timeLiteral);

	//create the result literal
	Toy_Literal result = TOY_TO_INTEGER_LITERAL(timer->tv_usec);
	Toy_pushLiteralArray(&interpreter->stack, result);

	//cleanup
	Toy_freeLiteral(timeLiteral);
	Toy_freeLiteral(result);

	return 1;
}

static int nativeCompareTimer(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to _compareTimer\n");
		return -1;
	}

	//unwrap the opaque literals
	Toy_Literal rhsLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal lhsLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal lhsLiteralIdn = lhsLiteral;
	if (TOY_IS_IDENTIFIER(lhsLiteral) && Toy_parseIdentifierToValue(interpreter, &lhsLiteral)) {
		Toy_freeLiteral(lhsLiteralIdn);
	}

	Toy_Literal rhsLiteralIdn = rhsLiteral;
	if (TOY_IS_IDENTIFIER(rhsLiteral) && Toy_parseIdentifierToValue(interpreter, &rhsLiteral)) {
		Toy_freeLiteral(rhsLiteralIdn);
	}

	if (!TOY_IS_OPAQUE(lhsLiteral) || !TOY_IS_OPAQUE(rhsLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _compareTimer\n");
		Toy_freeLiteral(lhsLiteral);
		Toy_freeLiteral(rhsLiteral);
		return -1;
	}

	struct timeval* lhsTimer = TOY_AS_OPAQUE(lhsLiteral);
	struct timeval* rhsTimer = TOY_AS_OPAQUE(rhsLiteral);

	//determine the difference, and wrap it
	struct timeval* d = diff(lhsTimer, rhsTimer);
	Toy_Literal diffLiteral = TOY_TO_OPAQUE_LITERAL(d, -1);
	Toy_pushLiteralArray(&interpreter->stack, diffLiteral);

	//cleanup
	Toy_freeLiteral(lhsLiteral);
	Toy_freeLiteral(rhsLiteral);
	Toy_freeLiteral(diffLiteral);

	return 1;
}

static int nativeTimerToString(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _timerToString\n");
		return -1;
	}

	//unwrap in an opaque literal
	Toy_Literal timeLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal timeLiteralIdn = timeLiteral;
	if (TOY_IS_IDENTIFIER(timeLiteral) && Toy_parseIdentifierToValue(interpreter, &timeLiteral)) {
		Toy_freeLiteral(timeLiteralIdn);
	}

	if (!TOY_IS_OPAQUE(timeLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _timerToString\n");
		Toy_freeLiteral(timeLiteral);
		return -1;
	}

	struct timeval* timer = TOY_AS_OPAQUE(timeLiteral);

	//create the string literal
	Toy_Literal resultLiteral = TOY_TO_NULL_LITERAL;
	if (timer->tv_sec == 0 && timer->tv_usec < 0) { //special case, for when the negative sign is encoded in the usec
		char buffer[128];
		snprintf(buffer, 128, "-%ld.%06ld", timer->tv_sec, -timer->tv_usec);
		resultLiteral = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(buffer, strlen(buffer)));
	}
	else { //normal case
		char buffer[128];
		snprintf(buffer, 128, "%ld.%06ld", timer->tv_sec, timer->tv_usec);
		resultLiteral = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(buffer, strlen(buffer)));
	}

	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(timeLiteral);
	Toy_freeLiteral(resultLiteral);

	return 1;
}

static int nativeDestroyTimer(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _destroyTimer\n");
		return -1;
	}

		//unwrap in an opaque literal
	Toy_Literal timeLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal timeLiteralIdn = timeLiteral;
	if (TOY_IS_IDENTIFIER(timeLiteral) && Toy_parseIdentifierToValue(interpreter, &timeLiteral)) {
		Toy_freeLiteral(timeLiteralIdn);
	}

	if (!TOY_IS_OPAQUE(timeLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _destroyTimer\n");
		Toy_freeLiteral(timeLiteral);
		return -1;
	}

	struct timeval* timer = TOY_AS_OPAQUE(timeLiteral);

	TOY_FREE(struct timeval, timer);

	Toy_freeLiteral(timeLiteral);

	return 0;
}

//call the hook
typedef struct Natives {
	char* name;
	Toy_NativeFn fn;
} Natives;

int Toy_hookTimer(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias) {
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
