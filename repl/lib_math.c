#include "lib_math.h"

#include "toy_memory.h"

#include <math.h>

#define LIB_MATH_PI	3.14159265358979323846f
#define LIB_MATH_E	2.71828182845904523536f

static int nativeMod(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to sin\n");
		return -1;
	}

	//get the arguments
	Toy_Literal xLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal yLiteral = Toy_popLiteralArray(arguments);

	//parse the argument (if it's an identifier)
	Toy_Literal xLiteralIdn = xLiteral;
	if (TOY_IS_IDENTIFIER(xLiteral) && Toy_parseIdentifierToValue(interpreter, &xLiteral)) {
		Toy_freeLiteral(xLiteralIdn);
	}

	Toy_Literal yLiteralIdn = yLiteral;
	if (TOY_IS_IDENTIFIER(yLiteral) && Toy_parseIdentifierToValue(interpreter, &yLiteral)) {
		Toy_freeLiteral(yLiteralIdn);
	}

	//check the argument types
	if (!(TOY_IS_INTEGER(xLiteral) || TOY_IS_FLOAT(xLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to sin\n");
		Toy_freeLiteral(xLiteral);
		return -1;
	}

	if (!(TOY_IS_INTEGER(yLiteral) || TOY_IS_FLOAT(yLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to sin\n");
		Toy_freeLiteral(yLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float x = TOY_IS_INTEGER(xLiteral)? TOY_AS_INTEGER(xLiteral) : TOY_AS_FLOAT(xLiteral);
	float y = TOY_IS_INTEGER(yLiteral)? TOY_AS_INTEGER(yLiteral) : TOY_AS_FLOAT(yLiteral);

	// calculate the result
	float result = fmodf(x, y);

	// return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(xLiteral);
	Toy_freeLiteral(yLiteral);

	return 1;
}

static int nativePow(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to sin\n");
		return -1;
	}

	//get the arguments
	Toy_Literal xLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal yLiteral = Toy_popLiteralArray(arguments);

	//parse the argument (if it's an identifier)
	Toy_Literal xLiteralIdn = xLiteral;
	if (TOY_IS_IDENTIFIER(xLiteral) && Toy_parseIdentifierToValue(interpreter, &xLiteral)) {
		Toy_freeLiteral(xLiteralIdn);
	}

	Toy_Literal yLiteralIdn = yLiteral;
	if (TOY_IS_IDENTIFIER(yLiteral) && Toy_parseIdentifierToValue(interpreter, &yLiteral)) {
		Toy_freeLiteral(yLiteralIdn);
	}

	//check the argument types
	if (!(TOY_IS_INTEGER(xLiteral) || TOY_IS_FLOAT(xLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to sin\n");
		Toy_freeLiteral(xLiteral);
		return -1;
	}

	if (!(TOY_IS_INTEGER(yLiteral) || TOY_IS_FLOAT(yLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to sin\n");
		Toy_freeLiteral(yLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float x = TOY_IS_INTEGER(xLiteral)? TOY_AS_INTEGER(xLiteral) : TOY_AS_FLOAT(xLiteral);
	float y = TOY_IS_INTEGER(yLiteral)? TOY_AS_INTEGER(yLiteral) : TOY_AS_FLOAT(yLiteral);

	// calculate the result
	float result = powf(x, y);

	// return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(xLiteral);
	Toy_freeLiteral(yLiteral);

	return 1;
}

static int nativeSqrt(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to sin\n");
		return -1;
	}

	//get the argument
	Toy_Literal xLiteral = Toy_popLiteralArray(arguments);

	//parse the argument (if it's an identifier)
	Toy_Literal xLiteralIdn = xLiteral;
	if (TOY_IS_IDENTIFIER(xLiteral) && Toy_parseIdentifierToValue(interpreter, &xLiteral)) {
		Toy_freeLiteral(xLiteralIdn);
	}

	//check the argument type
	if (!(TOY_IS_INTEGER(xLiteral) || TOY_IS_FLOAT(xLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to sin\n");
		Toy_freeLiteral(xLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float x = TOY_IS_INTEGER(xLiteral)? TOY_AS_INTEGER(xLiteral) : TOY_AS_FLOAT(xLiteral);

	// calculate the result
	float result = sqrtf(x);

	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(xLiteral);

	return 1;
}

static int nativeCbrt(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to sin\n");
		return -1;
	}

	//get the argument
	Toy_Literal xLiteral = Toy_popLiteralArray(arguments);

	//parse the argument (if it's an identifier)
	Toy_Literal xLiteralIdn = xLiteral;
	if (TOY_IS_IDENTIFIER(xLiteral) && Toy_parseIdentifierToValue(interpreter, &xLiteral)) {
		Toy_freeLiteral(xLiteralIdn);
	}

	//check the argument type
	if (!(TOY_IS_INTEGER(xLiteral) || TOY_IS_FLOAT(xLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to sin\n");
		Toy_freeLiteral(xLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float x = TOY_IS_INTEGER(xLiteral)? TOY_AS_INTEGER(xLiteral) : TOY_AS_FLOAT(xLiteral);

	// calculate the result
	float result = cbrtf(x);

	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(xLiteral);

	return 1;
}

static int nativeHypot(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to sin\n");
		return -1;
	}

	//get the arguments
	Toy_Literal xLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal yLiteral = Toy_popLiteralArray(arguments);

	//parse the argument (if it's an identifier)
	Toy_Literal xLiteralIdn = xLiteral;
	if (TOY_IS_IDENTIFIER(xLiteral) && Toy_parseIdentifierToValue(interpreter, &xLiteral)) {
		Toy_freeLiteral(xLiteralIdn);
	}

	Toy_Literal yLiteralIdn = yLiteral;
	if (TOY_IS_IDENTIFIER(yLiteral) && Toy_parseIdentifierToValue(interpreter, &yLiteral)) {
		Toy_freeLiteral(yLiteralIdn);
	}

	//check the argument types
	if (!(TOY_IS_INTEGER(xLiteral) || TOY_IS_FLOAT(xLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to sin\n");
		Toy_freeLiteral(xLiteral);
		return -1;
	}

	if (!(TOY_IS_INTEGER(yLiteral) || TOY_IS_FLOAT(yLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to sin\n");
		Toy_freeLiteral(yLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float x = TOY_IS_INTEGER(xLiteral)? TOY_AS_INTEGER(xLiteral) : TOY_AS_FLOAT(xLiteral);
	float y = TOY_IS_INTEGER(yLiteral)? TOY_AS_INTEGER(yLiteral) : TOY_AS_FLOAT(yLiteral);

	// calculate the result
	float result = hypotf(x, y);

	// return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(xLiteral);
	Toy_freeLiteral(yLiteral);

	return 1;
}

static int nativeToRad(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to toRad\n");
		return -1;
	}

	//get the argument
	Toy_Literal degreesLiteral = Toy_popLiteralArray(arguments);

	//parse the argument (if it's an identifier)
	Toy_Literal degreesLiteralIdn = degreesLiteral;
	if (TOY_IS_IDENTIFIER(degreesLiteral) && Toy_parseIdentifierToValue(interpreter, &degreesLiteral)) {	
		Toy_freeLiteral(degreesLiteralIdn);
	}

	//check the argument type
	if (!(TOY_IS_INTEGER(degreesLiteral) || TOY_IS_FLOAT(degreesLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to toRad\n");
		Toy_freeLiteral(degreesLiteral);
		return -1;
	}

	// cast int to float to handle all types of numbers
	float degrees = TOY_IS_INTEGER(degreesLiteral)? TOY_AS_INTEGER(degreesLiteral) : TOY_AS_FLOAT(degreesLiteral);

	float result = degrees * (LIB_MATH_PI / 180.0);

	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(degreesLiteral);

	return 1;
}

static int nativeToDeg(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to toDeg\n");
		return -1;
	}

	//get the argument
	Toy_Literal radiansLiteral = Toy_popLiteralArray(arguments);

	//parse the argument (if it's an identifier)
	Toy_Literal radiansLiteralIdn = radiansLiteral;
	if (TOY_IS_IDENTIFIER(radiansLiteral) && Toy_parseIdentifierToValue(interpreter, &radiansLiteral)) {	
		Toy_freeLiteral(radiansLiteralIdn);
	}

	//check the argument type
	if (!(TOY_IS_INTEGER(radiansLiteral) || TOY_IS_FLOAT(radiansLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to toDeg\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast int to float to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	float result = radians * (180.0 / LIB_MATH_PI);

	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeSin(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to sin\n");
		return -1;
	}

	//get the argument
	Toy_Literal radiansLiteral = Toy_popLiteralArray(arguments);

	//parse the argument (if it's an identifier)
	Toy_Literal radiansLiteralIdn = radiansLiteral;
	if (TOY_IS_IDENTIFIER(radiansLiteral) && Toy_parseIdentifierToValue(interpreter, &radiansLiteral)) {
		Toy_freeLiteral(radiansLiteralIdn);
	}

	//check the argument type
	if (!(TOY_IS_INTEGER(radiansLiteral) || TOY_IS_FLOAT(radiansLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to sin\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = sinf(radians);

	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeCos(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to cos\n");
		return -1;
	}

	//get the argument
	Toy_Literal radiansLiteral = Toy_popLiteralArray(arguments);

	//parse the argument (if it's an identifier)
	Toy_Literal radiansLiteralIdn = radiansLiteral;
	if (TOY_IS_IDENTIFIER(radiansLiteral) && Toy_parseIdentifierToValue(interpreter, &radiansLiteral)) {
		Toy_freeLiteral(radiansLiteralIdn);
	}

	//check the argument type
	if (!(TOY_IS_INTEGER(radiansLiteral) || TOY_IS_FLOAT(radiansLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to cos\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = cosf(radians);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeTan(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to tan\n");
		return -1;
	}

	//get the argument
	Toy_Literal radiansLiteral = Toy_popLiteralArray(arguments);

	//parse the argument (if it's an identifier)
	Toy_Literal radiansLiteralIdn = radiansLiteral;
	if (TOY_IS_IDENTIFIER(radiansLiteral) && Toy_parseIdentifierToValue(interpreter, &radiansLiteral)) {
		Toy_freeLiteral(radiansLiteralIdn);
	}

	//check the argument type
	if (!(TOY_IS_INTEGER(radiansLiteral) || TOY_IS_FLOAT(radiansLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to tan\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = tanf(radians);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeTan(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to tan\n");
		return -1;
	}

	//get the argument
	Toy_Literal radiansLiteral = Toy_popLiteralArray(arguments);

	//parse the argument (if it's an identifier)
	Toy_Literal radiansLiteralIdn = radiansLiteral;
	if (TOY_IS_IDENTIFIER(radiansLiteral) && Toy_parseIdentifierToValue(interpreter, &radiansLiteral)) {
		Toy_freeLiteral(radiansLiteralIdn);
	}

	//check the argument type
	if (!(TOY_IS_INTEGER(radiansLiteral) || TOY_IS_FLOAT(radiansLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to tan\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = tanf(radians);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

//call the hook
typedef struct Natives {
	char* name;
	Toy_NativeFn fn;
} Natives;

int Toy_hookMath(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias) {
	//build the natives list
	Natives natives[] = {
		// Common
		{"mod", nativeMod},

		// Power
		{"pow", nativePow},
		{"sqrt", nativeSqrt},
		{"cbrt", nativeCbrt},
		{"hypot", nativeHypot},

		// Trigonometric
		{"toRad", nativeToRad},
		{"toDeg", nativeToDeg},
		{"sin", nativeSin},
		{"cos", nativeCos},
		{"tan", nativeTan},

		{NULL, NULL}
	};

	// math constants
	Toy_Literal piKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("PI"));
	Toy_Literal piIdentifierLiteral = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString("PI"));
	Toy_Literal piLiteral = TOY_TO_FLOAT_LITERAL(LIB_MATH_PI);

	Toy_Literal eKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("E"));
	Toy_Literal eIdentifierLiteral = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString("E"));
	Toy_Literal eLiteral = TOY_TO_FLOAT_LITERAL(LIB_MATH_E);
	

	//store the library in an aliased dictionary
	if (!TOY_IS_NULL(alias)) {
		//make sure the name isn't taken
		if (Toy_isDeclaredScopeVariable(interpreter->scope, alias)) {
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

		Toy_setLiteralDictionary(dictionary, piKeyLiteral, piLiteral);
		Toy_setLiteralDictionary(dictionary, eKeyLiteral, eLiteral);


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

	if (
		Toy_isDeclaredScopeVariable(interpreter->scope, piKeyLiteral) ||
		Toy_isDeclaredScopeVariable(interpreter->scope, eKeyLiteral)
	) {
		interpreter->errorOutput("Can't override an existing variable\n");
		
		// cleanup
		Toy_freeLiteral(alias);
		Toy_freeLiteral(piIdentifierLiteral);
		Toy_freeLiteral(piKeyLiteral);
		Toy_freeLiteral(eIdentifierLiteral);
		Toy_freeLiteral(eKeyLiteral);

		return -1;
	}

	Toy_Literal floatType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_FLOAT, false);

	// pi
	Toy_declareScopeVariable(interpreter->scope, piIdentifierLiteral, floatType);
	Toy_setScopeVariable(interpreter->scope, piIdentifierLiteral, piLiteral, false);

	// e
	Toy_declareScopeVariable(interpreter->scope, eIdentifierLiteral, floatType);
	Toy_setScopeVariable(interpreter->scope, eIdentifierLiteral, eLiteral, false);

	// cleanup
	Toy_freeLiteral(floatType);
	Toy_freeLiteral(piKeyLiteral);
	Toy_freeLiteral(piIdentifierLiteral);
	Toy_freeLiteral(piLiteral);
	Toy_freeLiteral(eKeyLiteral);
	Toy_freeLiteral(eIdentifierLiteral);
	Toy_freeLiteral(eLiteral);

	return 0;
}
