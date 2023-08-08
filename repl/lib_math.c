#include "lib_math.h"

#include "toy_memory.h"

#include <math.h>

#define LIB_MATH_PI      3.14159265358979323846f
#define LIB_MATH_E       2.71828182845904523536f
#define LIB_MATH_EPSILON 0.000001f

static int nativePow(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to pow\n");
		return -1;
	}

	//get the arguments
	Toy_Literal yLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal xLiteral = Toy_popLiteralArray(arguments);

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
		interpreter->errorOutput("Incorrect argument type passed to pow\n");
		Toy_freeLiteral(xLiteral);
		Toy_freeLiteral(yLiteralIdn);
		return -1;
	}

	if (!(TOY_IS_INTEGER(yLiteral) || TOY_IS_FLOAT(yLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to pow\n");
		Toy_freeLiteral(yLiteral);
		Toy_freeLiteral(xLiteral);
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
		interpreter->errorOutput("Incorrect number of arguments to sqrt\n");
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
		interpreter->errorOutput("Incorrect argument type passed to sqrt\n");
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
		interpreter->errorOutput("Incorrect number of arguments to cbrt\n");
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
		interpreter->errorOutput("Incorrect argument type passed to cbrt\n");
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
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to hypot\n");
		return -1;
	}

	//get the arguments
	Toy_Literal yLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal xLiteral = Toy_popLiteralArray(arguments);

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
		interpreter->errorOutput("Incorrect argument type passed to hypot\n");
		Toy_freeLiteral(xLiteral);
		Toy_freeLiteral(yLiteral);
		return -1;
	}

	if (!(TOY_IS_INTEGER(yLiteral) || TOY_IS_FLOAT(yLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to hypot\n");
		Toy_freeLiteral(yLiteral);
		Toy_freeLiteral(xLiteral);
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

static int nativeToRadians(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
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

	float result = degrees * (LIB_MATH_PI / 180.0f);

	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(degreesLiteral);

	return 1;
}

static int nativeToDegrees(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
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

	float result = radians * (180.0f / LIB_MATH_PI);

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

static int nativeAsin(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to asin\n");
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
		interpreter->errorOutput("Incorrect argument type passed to asin\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = asinf(radians);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeAcos(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to acos\n");
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
		interpreter->errorOutput("Incorrect argument type passed to acos\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = acosf(radians);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeAtan(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to atan\n");
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
		interpreter->errorOutput("Incorrect argument type passed to atan\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = atanf(radians);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeAtan2(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to atan2\n");
		return -1;
	}

	//get the argument
	Toy_Literal yLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal xLiteral = Toy_popLiteralArray(arguments);

	//parse the argument (if it's an identifier)
	Toy_Literal xLiteralIdn = xLiteral;
	if (TOY_IS_IDENTIFIER(xLiteral) && Toy_parseIdentifierToValue(interpreter, &xLiteral)) {
		Toy_freeLiteral(xLiteralIdn);
	}

	Toy_Literal yLiteralIdn = yLiteral;
	if (TOY_IS_IDENTIFIER(yLiteral) && Toy_parseIdentifierToValue(interpreter, &yLiteral)) {
		Toy_freeLiteral(yLiteralIdn);
	}

	//check the argument type
	if (!(TOY_IS_INTEGER(xLiteral) || TOY_IS_FLOAT(xLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to atan2\n");
		Toy_freeLiteral(xLiteral);
		Toy_freeLiteral(yLiteral);
		return -1;
	}

	if (!(TOY_IS_INTEGER(yLiteral) || TOY_IS_FLOAT(yLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to atan2\n");
		Toy_freeLiteral(yLiteral);
		Toy_freeLiteral(xLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float x = TOY_IS_INTEGER(xLiteral)? TOY_AS_INTEGER(xLiteral) : TOY_AS_FLOAT(xLiteral);
	float y = TOY_IS_INTEGER(yLiteral)? TOY_AS_INTEGER(yLiteral) : TOY_AS_FLOAT(yLiteral);

	// calculate the result
	float result = atan2f(x, y);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(xLiteral);

	return 1;
}

static int nativeSinh(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to sinh\n");
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
		interpreter->errorOutput("Incorrect argument type passed to sinh\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = sinhf(radians);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeCosh(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to cosh\n");
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
		interpreter->errorOutput("Incorrect argument type passed to cosh\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = coshf(radians);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeTanh(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to tanh\n");
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
		interpreter->errorOutput("Incorrect argument type passed to tanh\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = tanhf(radians);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeAsinh(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to asinh\n");
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
		interpreter->errorOutput("Incorrect argument type passed to asinh\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = asinhf(radians);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeAcosh(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to acosh\n");
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
		interpreter->errorOutput("Incorrect argument type passed to acosh\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = acoshf(radians);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeAtanh(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to atanh\n");
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
		interpreter->errorOutput("Incorrect argument type passed to atanh\n");
		Toy_freeLiteral(radiansLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float radians = TOY_IS_INTEGER(radiansLiteral)? TOY_AS_INTEGER(radiansLiteral) : TOY_AS_FLOAT(radiansLiteral);

	// calculate the result
	float result = atanhf(radians);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_FLOAT_LITERAL(result);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(radiansLiteral);

	return 1;
}

static int nativeCheckIsNaN(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to checkIsNaN\n");
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
		interpreter->errorOutput("Incorrect argument type passed to checkIsNaN\n");
		Toy_freeLiteral(xLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float x = TOY_IS_INTEGER(xLiteral)? TOY_AS_INTEGER(xLiteral) : TOY_AS_FLOAT(xLiteral);

	// calculate the result
	int result = isnan(x);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result != 0);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(xLiteral);

	return 1;
}

static int nativeCheckIsFinite(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to checkIsFinite\n");
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
		interpreter->errorOutput("Incorrect argument type passed to checkIsFinite\n");
		Toy_freeLiteral(xLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float x = TOY_IS_INTEGER(xLiteral)? TOY_AS_INTEGER(xLiteral) : TOY_AS_FLOAT(xLiteral);

	// calculate the result
	int result = isfinite(x);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result != 0);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(xLiteral);

	return 1;
}

static int nativeCheckIsInfinite(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to checkIsInfinite\n");
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
		interpreter->errorOutput("Incorrect argument type passed to checkIsInfinite\n");
		Toy_freeLiteral(xLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float x = TOY_IS_INTEGER(xLiteral)? TOY_AS_INTEGER(xLiteral) : TOY_AS_FLOAT(xLiteral);

	// calculate the result
	int result = isinf(x);
	
	//return the result
	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result != 0);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(xLiteral);

	return 1;
}

static int nativeEpsilionCompare(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to mod\n");
		return -1;
	}

	//get the arguments
	Toy_Literal yLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal xLiteral = Toy_popLiteralArray(arguments);

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
		interpreter->errorOutput("Incorrect argument type passed to mod\n");
		Toy_freeLiteral(xLiteral);
		return -1;
	}

	if (!(TOY_IS_INTEGER(yLiteral) || TOY_IS_FLOAT(yLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to mod\n");
		Toy_freeLiteral(yLiteral);
		return -1;
	}

	// cast ints to floats to handle all types of numbers
	float x = TOY_IS_INTEGER(xLiteral)? TOY_AS_INTEGER(xLiteral) : TOY_AS_FLOAT(xLiteral);
	float y = TOY_IS_INTEGER(yLiteral)? TOY_AS_INTEGER(yLiteral) : TOY_AS_FLOAT(yLiteral);

	// calculate the result
	int result = (fabsf(x - y)) <= (LIB_MATH_EPSILON * fmaxf(1, fmaxf(fabsf(x), fabsf(y))));

	// return the result
	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result != 0);
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	// cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(xLiteral);
	Toy_freeLiteral(yLiteral);

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
		// Exponential

		// Power
		{"pow", nativePow},
		{"sqrt", nativeSqrt},
		{"cbrt", nativeCbrt},
		{"hypot", nativeHypot},

		// Trigonometric
		{"toRadians", nativeToRadians},
		{"toDegrees", nativeToDegrees},
		{"sin", nativeSin},
		{"cos", nativeCos},
		{"tan", nativeTan},
		{"asin", nativeAsin},
		{"acos", nativeAcos},
		{"atan", nativeAtan},
		{"atans", nativeAtan2},

		// Hyperbolic
		{"sinh", nativeSinh},
		{"cosh", nativeCosh},
		{"tanh", nativeTanh},
		{"asinh", nativeAsinh},
		{"acosh", nativeAcosh},
		{"atanh", nativeAtanh},

		// Comparison
		{"checkIsNaN",  nativeCheckIsNaN},
		{"checkIsFinite", nativeCheckIsFinite},
		{"checkIsInfinite", nativeCheckIsInfinite},
		{"epsilionCompare", nativeEpsilionCompare},

		{NULL, NULL}
	};

	// math constants
	Toy_Literal piKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("PI"));
	Toy_Literal piIdentifierLiteral = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString("PI"));
	Toy_Literal piLiteral = TOY_TO_FLOAT_LITERAL(LIB_MATH_PI);

	Toy_Literal eKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("E"));
	Toy_Literal eIdentifierLiteral = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString("E"));
	Toy_Literal eLiteral = TOY_TO_FLOAT_LITERAL(LIB_MATH_E);

	Toy_Literal epsilonKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("EPSILON"));
	Toy_Literal epsilonIdentifierLiteral = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString("EPSILON"));
	Toy_Literal epsilonLiteral = TOY_TO_FLOAT_LITERAL(LIB_MATH_EPSILON);

	Toy_Literal nanKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("NAN"));
	Toy_Literal nanIdentifierLiteral = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString("NAN"));
	Toy_Literal nanLiteral = TOY_TO_FLOAT_LITERAL(NAN);

	Toy_Literal infinityKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("INFINITY"));
	Toy_Literal infinityIdentifierLiteral = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString("INFINITY"));
	Toy_Literal infinityLiteral = TOY_TO_FLOAT_LITERAL(INFINITY);

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
		Toy_setLiteralDictionary(dictionary, nanKeyLiteral, nanLiteral);
		Toy_setLiteralDictionary(dictionary, infinityKeyLiteral, infinityLiteral);
		Toy_setLiteralDictionary(dictionary, epsilonKeyLiteral, epsilonLiteral);

		//build the type
		Toy_Literal type = TOY_TO_TYPE_LITERAL(TOY_LITERAL_DICTIONARY, true);
		Toy_Literal anyType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_ANY, true);
		Toy_Literal fnType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_FUNCTION_NATIVE, true);
		TOY_TYPE_PUSH_SUBTYPE(&type, anyType);
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
		Toy_isDeclaredScopeVariable(interpreter->scope, piKeyLiteral)		||
		Toy_isDeclaredScopeVariable(interpreter->scope, eKeyLiteral)		||
		Toy_isDeclaredScopeVariable(interpreter->scope, epsilonKeyLiteral)  ||
		Toy_isDeclaredScopeVariable(interpreter->scope, nanKeyLiteral)		||
		Toy_isDeclaredScopeVariable(interpreter->scope, infinityLiteral)
	) {
		interpreter->errorOutput("Can't override an existing variable\n");
		
		// cleanup
		Toy_freeLiteral(alias);
		Toy_freeLiteral(piIdentifierLiteral);
		Toy_freeLiteral(piKeyLiteral);
		Toy_freeLiteral(eIdentifierLiteral);
		Toy_freeLiteral(eKeyLiteral);
		Toy_freeLiteral(epsilonIdentifierLiteral);
		Toy_freeLiteral(epsilonKeyLiteral);
		Toy_freeLiteral(nanIdentifierLiteral);
		Toy_freeLiteral(nanKeyLiteral);
		Toy_freeLiteral(infinityIdentifierLiteral);
		Toy_freeLiteral(infinityLiteral);

		return -1;
	}

	Toy_Literal floatType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_FLOAT, false);

	// pi
	Toy_declareScopeVariable(interpreter->scope, piIdentifierLiteral, floatType);
	Toy_setScopeVariable(interpreter->scope, piIdentifierLiteral, piLiteral, false);

	// e
	Toy_declareScopeVariable(interpreter->scope, eIdentifierLiteral, floatType);
	Toy_setScopeVariable(interpreter->scope, eIdentifierLiteral, eLiteral, false);

	// epsilon
	Toy_declareScopeVariable(interpreter->scope, epsilonIdentifierLiteral, floatType);
	Toy_setScopeVariable(interpreter->scope, epsilonIdentifierLiteral, epsilonLiteral, false);

	// nan
	Toy_declareScopeVariable(interpreter->scope, nanIdentifierLiteral, floatType);
	Toy_setScopeVariable(interpreter->scope, nanIdentifierLiteral, nanLiteral, false);

	// infinity
	Toy_declareScopeVariable(interpreter->scope, infinityIdentifierLiteral, floatType);
	Toy_setScopeVariable(interpreter->scope, infinityIdentifierLiteral, infinityLiteral, false);

	// cleanup
	Toy_freeLiteral(floatType);
	Toy_freeLiteral(piKeyLiteral);
	Toy_freeLiteral(piIdentifierLiteral);
	Toy_freeLiteral(piLiteral);
	Toy_freeLiteral(eKeyLiteral);
	Toy_freeLiteral(eIdentifierLiteral);
	Toy_freeLiteral(eLiteral);
	Toy_freeLiteral(epsilonKeyLiteral);
	Toy_freeLiteral(epsilonIdentifierLiteral);
	Toy_freeLiteral(epsilonLiteral);
	Toy_freeLiteral(nanIdentifierLiteral);
	Toy_freeLiteral(nanKeyLiteral);
	Toy_freeLiteral(nanLiteral);
	Toy_freeLiteral(infinityIdentifierLiteral);
	Toy_freeLiteral(infinityKeyLiteral);
	Toy_freeLiteral(infinityLiteral);

	return 0;
}
