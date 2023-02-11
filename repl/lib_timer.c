#include "lib_timer.h"

#include "toy_memory.h"

#include <stdio.h>
#include <time.h>

//natives
static int nativeStartTimer(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 0) {
		interpreter->errorOutput("Incorrect number of arguments to startTimer\n");
		return -1;
	}

	//get the time from C (this is dumb)
	clock_t* ptr = TOY_ALLOCATE(clock_t, 1);
	*ptr = clock();

	//wrap in an opaque literal for Toy
	Toy_Literal timerLiteral = TOY_TO_OPAQUE_LITERAL(ptr, -1); //TODO: sort out the tags
	Toy_pushLiteralArray(&interpreter->stack, timerLiteral);

	Toy_freeLiteral(timerLiteral);

	return 1;
}

static int nativeStopTimer(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _stopTimer\n");
		return -1;
	}

	clock_t stop = clock();

	//unwrap the opaque literal
	Toy_Literal timerLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal timerLiteralIdn = timerLiteral;
	if (TOY_IS_IDENTIFIER(timerLiteral) && Toy_parseIdentifierToValue(interpreter, &timerLiteral)) {
		Toy_freeLiteral(timerLiteralIdn);
	}

	if (!TOY_IS_OPAQUE(timerLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to _stopTimer\n");
		Toy_freeLiteral(timerLiteral);
		return -1;
	}

	clock_t* ptr = TOY_AS_OPAQUE(timerLiteral);

	//determine the difference, and wrap it
	clock_t* diff = TOY_ALLOCATE(clock_t, 1);
	*diff = *ptr - stop;
	Toy_Literal diffLiteral = TOY_TO_OPAQUE_LITERAL(diff, -1);

	Toy_pushLiteralArray(&interpreter->stack, diffLiteral);

	//cleanup
	Toy_freeLiteral(timerLiteral);
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

	//determine the clocks per whatever
	clock_t* timer = TOY_ALLOCATE(clock_t, 1);
	*timer = TOY_AS_INTEGER(secondLiteral) * CLOCKS_PER_SEC + TOY_AS_INTEGER(microsecondLiteral);

	//wrap in an opaque literal for Toy
	Toy_Literal timeLiteral = TOY_TO_OPAQUE_LITERAL(timer, -1);
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

	clock_t* timer = TOY_AS_OPAQUE(timeLiteral);

	//create the result literal
	Toy_Literal result = TOY_TO_INTEGER_LITERAL(*timer / CLOCKS_PER_SEC);
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

	clock_t* timer = TOY_AS_OPAQUE(timeLiteral);

	//create the result literal
	Toy_Literal result = TOY_TO_INTEGER_LITERAL(*timer % CLOCKS_PER_SEC);
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

	clock_t* lhsTimer = TOY_AS_OPAQUE(lhsLiteral);
	clock_t* rhsTimer = TOY_AS_OPAQUE(rhsLiteral);

	//determine the difference, and wrap it
	clock_t* diff = TOY_ALLOCATE(clock_t, 1);
	*diff = *lhsTimer - *rhsTimer;

	Toy_Literal diffLiteral = TOY_TO_OPAQUE_LITERAL(diff, -1);
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

	clock_t* timer = TOY_AS_OPAQUE(timeLiteral);

	//create the string literal
	char buffer[128];
	snprintf(buffer, 128, "%ld.%06ld", *timer / CLOCKS_PER_SEC, *timer % CLOCKS_PER_SEC);
	Toy_Literal resultLiteral = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(buffer, strlen(buffer)));

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

	clock_t* timer = TOY_AS_OPAQUE(timeLiteral);

	TOY_FREE(clock_t, timer);

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
			return -1;
		}

		//create the dictionary to load up with functions
		Toy_LiteralDictionary* dictionary = TOY_ALLOCATE(Toy_LiteralDictionary, 1);
		Toy_initLiteralDictionary(dictionary);

		//load the dict with functions
		for (int i = 0; natives[i].name; i++) {
			Toy_Literal name = TOY_TO_STRING_LITERAL(Toy_createRefString(natives[i].name));
			Toy_Literal func = TOY_TO_FUNCTION_NATIVE_LITERAL(natives[i].fn);

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
