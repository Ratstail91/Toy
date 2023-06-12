#include "lib_standard.h"

#include "toy_memory.h"

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

static int nativeClock(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 0) {
		interpreter->errorOutput("Incorrect number of arguments to clock\n");
		return -1;
	}

	//get the time from C (what a pain)
	time_t rawtime = time(NULL);
	struct tm* timeinfo = localtime( &rawtime );
	char* timestr = asctime(timeinfo);

	//push to the stack
	size_t len = strlen(timestr) - 1; //-1 for the newline
	Toy_Literal timeLiteral = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(timestr, len));

	//push to the stack
	Toy_pushLiteralArray(&interpreter->stack, timeLiteral);

	//cleanup
	Toy_freeLiteral(timeLiteral);

	return 1;
}

static int nativeHash(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to hash\n");
		return -1;
	}

	//get the self
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral)) {
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	Toy_Literal result = TOY_TO_INTEGER_LITERAL(Toy_hashLiteral(selfLiteral));

	Toy_pushLiteralArray(&interpreter->stack, result);

	Toy_freeLiteral(result);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeAbs(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to abs\n");
		return -1;
	}

	//get the self
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral)) {
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	if (!(TOY_IS_INTEGER(selfLiteral) || TOY_IS_FLOAT(selfLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to abs\n");
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	Toy_Literal result;

	if (TOY_IS_INTEGER(selfLiteral)) {
		result = TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(selfLiteral) > 0 ? TOY_AS_INTEGER(selfLiteral) : -TOY_AS_INTEGER(selfLiteral) );
	}
	if (TOY_IS_FLOAT(selfLiteral)) {
		result = TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(selfLiteral) > 0 ? TOY_AS_FLOAT(selfLiteral) : -TOY_AS_FLOAT(selfLiteral) );
	}

	Toy_pushLiteralArray(&interpreter->stack, result);

	Toy_freeLiteral(result);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeCeil(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to ceil\n");
		return -1;
	}

	//get the self
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral)) {
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	if (!(TOY_IS_INTEGER(selfLiteral) || TOY_IS_FLOAT(selfLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to ceil\n");
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	Toy_Literal result;

	if (TOY_IS_INTEGER(selfLiteral)) {
		//NO-OP
		result = Toy_copyLiteral(selfLiteral);
	}
	if (TOY_IS_FLOAT(selfLiteral)) {
		result = TOY_TO_INTEGER_LITERAL( (int)TOY_AS_FLOAT(selfLiteral) - TOY_AS_FLOAT(selfLiteral) == 0 ? (int)TOY_AS_FLOAT(selfLiteral) : (int)TOY_AS_FLOAT(selfLiteral) + 1 );
	}

	Toy_pushLiteralArray(&interpreter->stack, result);

	Toy_freeLiteral(result);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeFloor(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to floor\n");
		return -1;
	}

	//get the self
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral)) {
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	if (!(TOY_IS_INTEGER(selfLiteral) || TOY_IS_FLOAT(selfLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to floor\n");
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	Toy_Literal result;

	if (TOY_IS_INTEGER(selfLiteral)) {
		//NO-OP
		result = Toy_copyLiteral(selfLiteral);
	}
	if (TOY_IS_FLOAT(selfLiteral)) {
		result = TOY_TO_INTEGER_LITERAL( (int)TOY_AS_FLOAT(selfLiteral) );
	}

	Toy_pushLiteralArray(&interpreter->stack, result);

	Toy_freeLiteral(result);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeMax(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//return value
	Toy_Literal resultLiteral = TOY_TO_NULL_LITERAL;

	//iterate over all arguments
	do {
		//get the self
		Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

		//parse to value if needed
		Toy_Literal selfLiteralIdn = selfLiteral;
		if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
			Toy_freeLiteral(selfLiteralIdn);
		}

		if (TOY_IS_IDENTIFIER(selfLiteral)) {
			Toy_freeLiteral(selfLiteral);
			return -1;
		}

		if (!(TOY_IS_INTEGER(selfLiteral) || TOY_IS_FLOAT(selfLiteral))) {
			interpreter->errorOutput("Incorrect argument type passed to max\n");
			Toy_freeLiteral(selfLiteral);
			return -1;
		}

		//if not comparing yet...
		if (TOY_IS_NULL(resultLiteral)) {
			resultLiteral = selfLiteral;
			continue;
		}

		//cooerce if needed
		if (TOY_IS_INTEGER(resultLiteral) && TOY_IS_FLOAT(selfLiteral)) {
			resultLiteral = TOY_TO_FLOAT_LITERAL( TOY_AS_INTEGER(resultLiteral) );
		}

		if (TOY_IS_FLOAT(resultLiteral) && TOY_IS_INTEGER(selfLiteral)) {
			selfLiteral = TOY_TO_FLOAT_LITERAL( TOY_AS_INTEGER(selfLiteral) );
		}

		//compare
		if (TOY_IS_INTEGER(resultLiteral) && TOY_AS_INTEGER(resultLiteral) < TOY_AS_INTEGER(selfLiteral)) {
			//NOTE: just ints, don't free
			resultLiteral = selfLiteral;
		}

		else if (TOY_IS_FLOAT(resultLiteral) && TOY_AS_FLOAT(resultLiteral) < TOY_AS_FLOAT(selfLiteral)) {
			//NOTE: just floats, don't free
			resultLiteral = selfLiteral;
		}
	}
	while (arguments->count > 0);

	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	Toy_freeLiteral(resultLiteral);

	return 1;
}

static int nativeMin(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//return value
	Toy_Literal resultLiteral = TOY_TO_NULL_LITERAL;

	//iterate over all arguments
	do {
		//get the self
		Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

		//parse to value if needed
		Toy_Literal selfLiteralIdn = selfLiteral;
		if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
			Toy_freeLiteral(selfLiteralIdn);
		}

		if (TOY_IS_IDENTIFIER(selfLiteral)) {
			Toy_freeLiteral(selfLiteral);
			return -1;
		}

		if (!(TOY_IS_INTEGER(selfLiteral) || TOY_IS_FLOAT(selfLiteral))) {
			interpreter->errorOutput("Incorrect argument type passed to min\n");
			Toy_freeLiteral(selfLiteral);
			return -1;
		}

		//if not comparing yet...
		if (TOY_IS_NULL(resultLiteral)) {
			resultLiteral = selfLiteral;
			continue;
		}

		//cooerce if needed
		if (TOY_IS_INTEGER(resultLiteral) && TOY_IS_FLOAT(selfLiteral)) {
			resultLiteral = TOY_TO_FLOAT_LITERAL( TOY_AS_INTEGER(resultLiteral) );
		}

		if (TOY_IS_FLOAT(resultLiteral) && TOY_IS_INTEGER(selfLiteral)) {
			selfLiteral = TOY_TO_FLOAT_LITERAL( TOY_AS_INTEGER(selfLiteral) );
		}

		//compare
		if (TOY_IS_INTEGER(resultLiteral) && TOY_AS_INTEGER(resultLiteral) > TOY_AS_INTEGER(selfLiteral)) {
			//NOTE: just ints, don't free
			resultLiteral = selfLiteral;
		}

		else if (TOY_IS_FLOAT(resultLiteral) && TOY_AS_FLOAT(resultLiteral) > TOY_AS_FLOAT(selfLiteral)) {
			//NOTE: just floats, don't free
			resultLiteral = selfLiteral;
		}
	}
	while (arguments->count > 0);

	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	Toy_freeLiteral(resultLiteral);

	return 1;
}

static int nativeRound(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to round\n");
		return -1;
	}

	//get the self
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral)) {
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	if (!(TOY_IS_INTEGER(selfLiteral) || TOY_IS_FLOAT(selfLiteral))) {
		interpreter->errorOutput("Incorrect argument type passed to round\n");
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	Toy_Literal result;

	if (TOY_IS_INTEGER(selfLiteral)) {
		//NO-OP
		result = Toy_copyLiteral(selfLiteral);
	}
	if (TOY_IS_FLOAT(selfLiteral)) {
		//catch the already-rounded case
		if (TOY_AS_FLOAT(selfLiteral) == (int)TOY_AS_FLOAT(selfLiteral)) {
			result = TOY_TO_INTEGER_LITERAL((int)TOY_AS_FLOAT(selfLiteral));
		}
		else {
			result = TOY_TO_INTEGER_LITERAL( TOY_AS_FLOAT(selfLiteral) - (int)TOY_AS_FLOAT(selfLiteral) < 0.5 ? (int)TOY_AS_FLOAT(selfLiteral) : (int)TOY_AS_FLOAT(selfLiteral) + 1 );
		}
	}

	Toy_pushLiteralArray(&interpreter->stack, result);

	Toy_freeLiteral(result);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeConcat(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to concat\n");
		return -1;
	}

	//get the args
	Toy_Literal otherLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_Literal otherLiteralIdn = otherLiteral;
	if (TOY_IS_IDENTIFIER(otherLiteral) && Toy_parseIdentifierToValue(interpreter, &otherLiteral)) {
		Toy_freeLiteral(otherLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(otherLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(otherLiteral);
		return -1;
	}

	//for each self type
	if (TOY_IS_ARRAY(selfLiteral)) {
		if (!TOY_IS_ARRAY(otherLiteral)) {
			interpreter->errorOutput("Incorrect argument type passed to concat (unknown type for other)\n");
			Toy_freeLiteral(selfLiteral);
			Toy_freeLiteral(otherLiteral);
			return -1;
		}

		//append each element of other to self, one-by-one
		for (int i = 0; i < TOY_AS_ARRAY(otherLiteral)->count; i++) {
			Toy_pushLiteralArray(TOY_AS_ARRAY(selfLiteral), TOY_AS_ARRAY(otherLiteral)->literals[i]);
		}

		//return and clean up
		Toy_pushLiteralArray(&interpreter->stack, selfLiteral);

		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(otherLiteral);

		return 1;
	}

	if (TOY_IS_DICTIONARY(selfLiteral)) {
		if (!TOY_IS_DICTIONARY(otherLiteral)) {
			interpreter->errorOutput("Incorrect argument type passed to concat (unknown type for other)\n");
			Toy_freeLiteral(selfLiteral);
			Toy_freeLiteral(otherLiteral);
			return -1;
		}

		//append each element of self to other, which will overwrite existing entries
		for (int i = 0; i < TOY_AS_DICTIONARY(selfLiteral)->capacity; i++) {
			if (!TOY_IS_NULL(TOY_AS_DICTIONARY(selfLiteral)->entries[i].key)) {
				Toy_setLiteralDictionary(TOY_AS_DICTIONARY(otherLiteral), TOY_AS_DICTIONARY(selfLiteral)->entries[i].key, TOY_AS_DICTIONARY(selfLiteral)->entries[i].value);
			}
		}

		//return and clean up
		Toy_pushLiteralArray(&interpreter->stack, otherLiteral);

		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(otherLiteral);

		return 1;
	}

	if (TOY_IS_STRING(selfLiteral)) { //a little redundant
		if (!TOY_IS_STRING(otherLiteral)) {
			interpreter->errorOutput("Incorrect argument type passed to concat (unknown type for other)\n");
			Toy_freeLiteral(selfLiteral);
			Toy_freeLiteral(otherLiteral);
			return -1;
		}

		//get the combined length for the new string
		size_t length = TOY_AS_STRING(selfLiteral)->length + TOY_AS_STRING(otherLiteral)->length + 1;

		if (length > TOY_MAX_STRING_LENGTH) {
			interpreter->errorOutput("Can't concatenate these strings, result is too long (error found in concat)\n");
			Toy_freeLiteral(selfLiteral);
			Toy_freeLiteral(otherLiteral);
			return -1;
		}

		//allocate the space and generate
		char* buffer = TOY_ALLOCATE(char, length);
		snprintf(buffer, length, "%s%s", Toy_toCString(TOY_AS_STRING(selfLiteral)), Toy_toCString(TOY_AS_STRING(otherLiteral)));

		Toy_Literal result = TOY_TO_STRING_LITERAL(Toy_createRefString(buffer));

		//push and clean up
		Toy_pushLiteralArray(&interpreter->stack, result);

		TOY_FREE_ARRAY(char, buffer, length);
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(otherLiteral);
		Toy_freeLiteral(result);

		return 1;
	} 

	interpreter->errorOutput("Incorrect argument type passed to concat (unknown type for self)\n");
	Toy_freeLiteral(selfLiteral);
	Toy_freeLiteral(otherLiteral);
	return -1;
}

static int nativeContainsKey(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to containsKey\n");
		return -1;
	}

	//get the args
	Toy_Literal keyLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_Literal keyLiteralIdn = keyLiteral;
	if (TOY_IS_IDENTIFIER(keyLiteral) && Toy_parseIdentifierToValue(interpreter, &keyLiteral)) {
		Toy_freeLiteral(keyLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(keyLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(keyLiteral);
		return -1;
	}

	//check type
	if (!(/* TOY_IS_ARRAY(selfLiteral) || */ TOY_IS_DICTIONARY(selfLiteral) )) {
		interpreter->errorOutput("Incorrect argument type passed to containsKey\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(keyLiteral);
		return -1;
	}

	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(false);
	if (TOY_IS_DICTIONARY(selfLiteral) && Toy_existsLiteralDictionary( TOY_AS_DICTIONARY(selfLiteral), keyLiteral )) {
		//return true of it contains the key
		Toy_freeLiteral(resultLiteral);
		resultLiteral = TOY_TO_BOOLEAN_LITERAL(true);
	}
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);
	Toy_freeLiteral(resultLiteral);

	Toy_freeLiteral(selfLiteral);
	Toy_freeLiteral(keyLiteral);

	return 1;
}

static int nativeContainsValue(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to containsValue\n");
		return -1;
	}

	//get the args
	Toy_Literal valueLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_Literal valueLiteralIdn = valueLiteral;
	if (TOY_IS_IDENTIFIER(valueLiteral) && Toy_parseIdentifierToValue(interpreter, &valueLiteral)) {
		Toy_freeLiteral(valueLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(valueLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(valueLiteral);
		return -1;
	}

	//check type
	if (!( TOY_IS_ARRAY(selfLiteral) || TOY_IS_DICTIONARY(selfLiteral) )) {
		interpreter->errorOutput("Incorrect argument type passed to containsValue\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(valueLiteral);
		return -1;
	}

	Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(false);
	if (TOY_IS_DICTIONARY(selfLiteral)) {
		for (int i = 0; i < TOY_AS_DICTIONARY(selfLiteral)->capacity; i++) {
			if (!TOY_IS_NULL(TOY_AS_DICTIONARY(selfLiteral)->entries[i].key) && Toy_literalsAreEqual( TOY_AS_DICTIONARY(selfLiteral)->entries[i].value, valueLiteral )) {
				//return true of it contains the value
				Toy_freeLiteral(resultLiteral);
				resultLiteral = TOY_TO_BOOLEAN_LITERAL(true);
				break;
			}
		}
	}
	else if (TOY_IS_ARRAY(selfLiteral)) {
		for (int i = 0; i < TOY_AS_ARRAY(selfLiteral)->count; i++) {
			Toy_Literal indexLiteral = TOY_TO_INTEGER_LITERAL(i);
			Toy_Literal elementLiteral = Toy_getLiteralArray(TOY_AS_ARRAY(selfLiteral), indexLiteral);


			if (Toy_literalsAreEqual(elementLiteral, valueLiteral)) {
				Toy_freeLiteral(indexLiteral);
				Toy_freeLiteral(elementLiteral);

				//return true of it contains the value
				Toy_freeLiteral(resultLiteral);
				resultLiteral = TOY_TO_BOOLEAN_LITERAL(true);
				break;
			}

			Toy_freeLiteral(indexLiteral);
			Toy_freeLiteral(elementLiteral);
		}
	}
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);
	Toy_freeLiteral(resultLiteral);

	Toy_freeLiteral(selfLiteral);
	Toy_freeLiteral(valueLiteral);

	return 1;
}

static int nativeEvery(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to every\n");
		return -1;
	}

	//get the args
	Toy_Literal fnLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_Literal fnLiteralIdn = fnLiteral;
	if (TOY_IS_IDENTIFIER(fnLiteral) && Toy_parseIdentifierToValue(interpreter, &fnLiteral)) {
		Toy_freeLiteral(fnLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(fnLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//check type
	if (!( TOY_IS_ARRAY(selfLiteral) || TOY_IS_DICTIONARY(selfLiteral) ) || !( TOY_IS_FUNCTION(fnLiteral) || TOY_IS_FUNCTION_NATIVE(fnLiteral) )) {
		interpreter->errorOutput("Incorrect argument type passed to every\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//call the given function on each element, based on the compound type
	if (TOY_IS_ARRAY(selfLiteral)) {
		bool result = true;

		//
		for (int i = 0; i < TOY_AS_ARRAY(selfLiteral)->count; i++) {
			Toy_Literal indexLiteral = TOY_TO_INTEGER_LITERAL(i);

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_pushLiteralArray(&arguments, indexLiteral);
			Toy_pushLiteralArray(&arguments, TOY_AS_ARRAY(selfLiteral)->literals[i]);

			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

			//grab the results
			Toy_Literal lit = Toy_popLiteralArray(&returns);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);
			Toy_freeLiteral(indexLiteral);

			//if not truthy
			if (!TOY_IS_TRUTHY(lit)) {
				Toy_freeLiteral(lit);
				result = false;
				break;
			}

			Toy_freeLiteral(lit);
		}

		Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result);
		Toy_pushLiteralArray(&interpreter->stack, resultLiteral);
		Toy_freeLiteral(resultLiteral);
	}

	if (TOY_IS_DICTIONARY(selfLiteral)) {
		bool result = true;

		for (int i = 0; i < TOY_AS_DICTIONARY(selfLiteral)->capacity; i++) {
			//skip nulls
			if (TOY_IS_NULL(TOY_AS_DICTIONARY(selfLiteral)->entries[i].key)) {
				continue;
			}

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_pushLiteralArray(&arguments, TOY_AS_DICTIONARY(selfLiteral)->entries[i].key);
			Toy_pushLiteralArray(&arguments, TOY_AS_DICTIONARY(selfLiteral)->entries[i].value);

			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

			//grab the results
			Toy_Literal lit = Toy_popLiteralArray(&returns);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);

			//if not truthy
			if (!TOY_IS_TRUTHY(lit)) {
				Toy_freeLiteral(lit);
				result = false;
				break;
			}

			Toy_freeLiteral(lit);
		}

		Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result);
		Toy_pushLiteralArray(&interpreter->stack, resultLiteral);
		Toy_freeLiteral(resultLiteral);
	}

	Toy_freeLiteral(fnLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeFilter(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to filter\n");
		return -1;
	}

	//get the args
	Toy_Literal fnLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_Literal fnLiteralIdn = fnLiteral;
	if (TOY_IS_IDENTIFIER(fnLiteral) && Toy_parseIdentifierToValue(interpreter, &fnLiteral)) {
		Toy_freeLiteral(fnLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(fnLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//check type
	if (!( TOY_IS_ARRAY(selfLiteral) || TOY_IS_DICTIONARY(selfLiteral) ) || !( TOY_IS_FUNCTION(fnLiteral) || TOY_IS_FUNCTION_NATIVE(fnLiteral) )) {
		interpreter->errorOutput("Incorrect argument type passed to filter\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//call the given function on each element, based on the compound type
	if (TOY_IS_ARRAY(selfLiteral)) {
		Toy_LiteralArray* result = TOY_ALLOCATE(Toy_LiteralArray, 1);
		Toy_initLiteralArray(result);

		//
		for (int i = 0; i < TOY_AS_ARRAY(selfLiteral)->count; i++) {
			Toy_Literal indexLiteral = TOY_TO_INTEGER_LITERAL(i);

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_pushLiteralArray(&arguments, indexLiteral);
			Toy_pushLiteralArray(&arguments, TOY_AS_ARRAY(selfLiteral)->literals[i]);

			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

			//grab the results
			Toy_Literal lit = Toy_popLiteralArray(&returns);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);
			Toy_freeLiteral(indexLiteral);

			//if truthy
			if (TOY_IS_TRUTHY(lit)) {
				Toy_pushLiteralArray(result, TOY_AS_ARRAY(selfLiteral)->literals[i]);
			}

			Toy_freeLiteral(lit);
		}

		Toy_Literal resultLiteral = TOY_TO_ARRAY_LITERAL(result);
		Toy_pushLiteralArray(&interpreter->stack, resultLiteral);
		Toy_freeLiteral(resultLiteral);
	}

	if (TOY_IS_DICTIONARY(selfLiteral)) {
		Toy_LiteralDictionary* result = TOY_ALLOCATE(Toy_LiteralDictionary, 1);
		Toy_initLiteralDictionary(result);

		for (int i = 0; i < TOY_AS_DICTIONARY(selfLiteral)->capacity; i++) {
			//skip nulls
			if (TOY_IS_NULL(TOY_AS_DICTIONARY(selfLiteral)->entries[i].key)) {
				continue;
			}

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_pushLiteralArray(&arguments, TOY_AS_DICTIONARY(selfLiteral)->entries[i].key);
			Toy_pushLiteralArray(&arguments, TOY_AS_DICTIONARY(selfLiteral)->entries[i].value);

			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

			//grab the results
			Toy_Literal lit = Toy_popLiteralArray(&returns);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);

			//if truthy
			if (TOY_IS_TRUTHY(lit)) {
				Toy_setLiteralDictionary(result, TOY_AS_DICTIONARY(selfLiteral)->entries[i].key, TOY_AS_DICTIONARY(selfLiteral)->entries[i].value);
			}

			Toy_freeLiteral(lit);
		}

		Toy_Literal resultLiteral = TOY_TO_DICTIONARY_LITERAL(result);
		Toy_pushLiteralArray(&interpreter->stack, resultLiteral);
		Toy_freeLiteral(resultLiteral);
	}

	Toy_freeLiteral(fnLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeForEach(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to forEach\n");
		return -1;
	}

	//get the args
	Toy_Literal fnLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_Literal fnLiteralIdn = fnLiteral;
	if (TOY_IS_IDENTIFIER(fnLiteral) && Toy_parseIdentifierToValue(interpreter, &fnLiteral)) {
		Toy_freeLiteral(fnLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(fnLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//check type
	if (!( TOY_IS_ARRAY(selfLiteral) || TOY_IS_DICTIONARY(selfLiteral) ) || !( TOY_IS_FUNCTION(fnLiteral) || TOY_IS_FUNCTION_NATIVE(fnLiteral) )) {
		interpreter->errorOutput("Incorrect argument type passed to forEach\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//call the given function on each element, based on the compound type
	if (TOY_IS_ARRAY(selfLiteral)) {
		for (int i = 0; i < TOY_AS_ARRAY(selfLiteral)->count; i++) {
			Toy_Literal indexLiteral = TOY_TO_INTEGER_LITERAL(i);

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_pushLiteralArray(&arguments, indexLiteral);
			Toy_pushLiteralArray(&arguments, TOY_AS_ARRAY(selfLiteral)->literals[i]);

			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);
			Toy_freeLiteral(indexLiteral);
		}
	}

	if (TOY_IS_DICTIONARY(selfLiteral)) {
		for (int i = 0; i < TOY_AS_DICTIONARY(selfLiteral)->capacity; i++) {
			//skip nulls
			if (TOY_IS_NULL(TOY_AS_DICTIONARY(selfLiteral)->entries[i].key)) {
				continue;
			}

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_pushLiteralArray(&arguments, TOY_AS_DICTIONARY(selfLiteral)->entries[i].key);
			Toy_pushLiteralArray(&arguments, TOY_AS_DICTIONARY(selfLiteral)->entries[i].value);

			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);
		}
	}

	Toy_freeLiteral(fnLiteral);
	Toy_freeLiteral(selfLiteral);

	return 0;
}

static int nativeGetKeys(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to getKeys\n");
		return -1;
	}

	//get the self
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral)) {
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	//check type
	if (!TOY_IS_DICTIONARY(selfLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to getKeys\n");
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	//generate the result literal
	Toy_LiteralArray* resultPtr = TOY_ALLOCATE(Toy_LiteralArray, 1);
	Toy_initLiteralArray(resultPtr);

	//get each key from the dictionary, pass it to the array
	for (int i = 0; i < TOY_AS_DICTIONARY(selfLiteral)->capacity; i++) {
		if (!TOY_IS_NULL( TOY_AS_DICTIONARY(selfLiteral)->entries[i].key )) {
			Toy_pushLiteralArray(resultPtr, TOY_AS_DICTIONARY(selfLiteral)->entries[i].key);
		}
	}

	//return the result
	Toy_Literal result = TOY_TO_ARRAY_LITERAL(resultPtr); //no copy
	Toy_pushLiteralArray(&interpreter->stack, result); //internal copy

	//clean up
	Toy_freeLiteralArray(resultPtr);
	TOY_FREE(Toy_LiteralArray, resultPtr);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeGetValues(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to getValues\n");
		return -1;
	}

	//get the self
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral)) {
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	//check type
	if (!TOY_IS_DICTIONARY(selfLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to getValues\n");
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	//generate the result literal
	Toy_LiteralArray* resultPtr = TOY_ALLOCATE(Toy_LiteralArray, 1);
	Toy_initLiteralArray(resultPtr);

	//get each key from the dictionary, pass it to the array
	for (int i = 0; i < TOY_AS_DICTIONARY(selfLiteral)->capacity; i++) {
		if (!TOY_IS_NULL( TOY_AS_DICTIONARY(selfLiteral)->entries[i].key )) {
			Toy_pushLiteralArray(resultPtr, TOY_AS_DICTIONARY(selfLiteral)->entries[i].value);
		}
	}

	//return the result
	Toy_Literal result = TOY_TO_ARRAY_LITERAL(resultPtr); //no copy
	Toy_pushLiteralArray(&interpreter->stack, result); //internal copy

	//clean up
	Toy_freeLiteralArray(resultPtr);
	TOY_FREE(Toy_LiteralArray, resultPtr);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeIndexOf(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to indexOf\n");
		return -1;
	}

	//get the args
	Toy_Literal valueLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_Literal valueLiteralIdn = valueLiteral;
	if (TOY_IS_IDENTIFIER(valueLiteral) && Toy_parseIdentifierToValue(interpreter, &valueLiteral)) {
		Toy_freeLiteral(valueLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(valueLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(valueLiteral);
		return -1;
	}

	//check type
	if (!TOY_IS_ARRAY(selfLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to indexOf\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(valueLiteral);
		return -1;
	}

	//search the array for the matching literal
	Toy_Literal resultLiteral = TOY_TO_NULL_LITERAL;
	for (int i = 0; i < TOY_AS_ARRAY(selfLiteral)->count; i++) {
		if (Toy_literalsAreEqual(valueLiteral, TOY_AS_ARRAY(selfLiteral)->literals[i])) {
			resultLiteral = TOY_TO_INTEGER_LITERAL(i);
			break;
		}
	}

	//return the result and clean up
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeMap(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to map\n");
		return -1;
	}

	//get the args
	Toy_Literal fnLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_Literal fnLiteralIdn = fnLiteral;
	if (TOY_IS_IDENTIFIER(fnLiteral) && Toy_parseIdentifierToValue(interpreter, &fnLiteral)) {
		Toy_freeLiteral(fnLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(fnLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//check type
	if (!( TOY_IS_ARRAY(selfLiteral) || TOY_IS_DICTIONARY(selfLiteral) ) || !( TOY_IS_FUNCTION(fnLiteral) || TOY_IS_FUNCTION_NATIVE(fnLiteral) )) {
		interpreter->errorOutput("Incorrect argument type passed to map\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//call the given function on each element, based on the compound type
	if (TOY_IS_ARRAY(selfLiteral)) {
		Toy_LiteralArray* returnsPtr = TOY_ALLOCATE(Toy_LiteralArray, 1);
		Toy_initLiteralArray(returnsPtr);

		for (int i = 0; i < TOY_AS_ARRAY(selfLiteral)->count; i++) {
			Toy_Literal indexLiteral = TOY_TO_INTEGER_LITERAL(i);

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_pushLiteralArray(&arguments, indexLiteral);
			Toy_pushLiteralArray(&arguments, TOY_AS_ARRAY(selfLiteral)->literals[i]);

			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

			//grab the results
			Toy_Literal lit = Toy_popLiteralArray(&returns);
			Toy_pushLiteralArray(returnsPtr, lit);
			Toy_freeLiteral(lit);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);
			Toy_freeLiteral(indexLiteral);
		}

		Toy_Literal returnsLiteral = TOY_TO_ARRAY_LITERAL(returnsPtr);
		Toy_pushLiteralArray(&interpreter->stack, returnsLiteral);
		Toy_freeLiteral(returnsLiteral);
	}

	if (TOY_IS_DICTIONARY(selfLiteral)) {
		Toy_LiteralArray* returnsPtr = TOY_ALLOCATE(Toy_LiteralArray, 1);
		Toy_initLiteralArray(returnsPtr);

		for (int i = 0; i < TOY_AS_DICTIONARY(selfLiteral)->capacity; i++) {
			//skip nulls
			if (TOY_IS_NULL(TOY_AS_DICTIONARY(selfLiteral)->entries[i].key)) {
				continue;
			}

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_pushLiteralArray(&arguments, TOY_AS_DICTIONARY(selfLiteral)->entries[i].key);
			Toy_pushLiteralArray(&arguments, TOY_AS_DICTIONARY(selfLiteral)->entries[i].value);

			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

			//grab the results
			Toy_Literal lit = Toy_popLiteralArray(&returns);
			Toy_pushLiteralArray(returnsPtr, lit);
			Toy_freeLiteral(lit);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);
		}

		Toy_Literal returnsLiteral = TOY_TO_ARRAY_LITERAL(returnsPtr);
		Toy_pushLiteralArray(&interpreter->stack, returnsLiteral);
		Toy_freeLiteral(returnsLiteral);
	}

	Toy_freeLiteral(fnLiteral);
	Toy_freeLiteral(selfLiteral);

	return 0;
}

static int nativeReduce(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 3) {
		interpreter->errorOutput("Incorrect number of arguments to reduce\n");
		return -1;
	}

	//get the args
	Toy_Literal fnLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal defaultLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_Literal defaultLiteralIdn = defaultLiteral;
	if (TOY_IS_IDENTIFIER(defaultLiteral) && Toy_parseIdentifierToValue(interpreter, &defaultLiteral)) {
		Toy_freeLiteral(defaultLiteralIdn);
	}

	Toy_Literal fnLiteralIdn = fnLiteral;
	if (TOY_IS_IDENTIFIER(fnLiteral) && Toy_parseIdentifierToValue(interpreter, &fnLiteral)) {
		Toy_freeLiteral(fnLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(defaultLiteral) || TOY_IS_IDENTIFIER(fnLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(defaultLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//check type
	if (!( TOY_IS_ARRAY(selfLiteral) || TOY_IS_DICTIONARY(selfLiteral) ) || !( TOY_IS_FUNCTION(fnLiteral) || TOY_IS_FUNCTION_NATIVE(fnLiteral) )) {
		interpreter->errorOutput("Incorrect argument type passed to reduce\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(defaultLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//call the given function on each element, based on the compound type
	if (TOY_IS_ARRAY(selfLiteral)) {
		for (int i = 0; i < TOY_AS_ARRAY(selfLiteral)->count; i++) {
			Toy_Literal indexLiteral = TOY_TO_INTEGER_LITERAL(i);

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_pushLiteralArray(&arguments, defaultLiteral);
			Toy_pushLiteralArray(&arguments, indexLiteral);
			Toy_pushLiteralArray(&arguments, TOY_AS_ARRAY(selfLiteral)->literals[i]);

			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

			//grab the results
			Toy_freeLiteral(defaultLiteral);
			defaultLiteral = Toy_popLiteralArray(&returns);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);
			Toy_freeLiteral(indexLiteral);
		}

		Toy_pushLiteralArray(&interpreter->stack, defaultLiteral);
	}

	if (TOY_IS_DICTIONARY(selfLiteral)) {
		for (int i = 0; i < TOY_AS_DICTIONARY(selfLiteral)->capacity; i++) {
			//skip nulls
			if (TOY_IS_NULL(TOY_AS_DICTIONARY(selfLiteral)->entries[i].key)) {
				continue;
			}

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_pushLiteralArray(&arguments, defaultLiteral);
			Toy_pushLiteralArray(&arguments, TOY_AS_DICTIONARY(selfLiteral)->entries[i].key);
			Toy_pushLiteralArray(&arguments, TOY_AS_DICTIONARY(selfLiteral)->entries[i].value);

			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

			//grab the results
			Toy_freeLiteral(defaultLiteral);
			defaultLiteral = Toy_popLiteralArray(&returns);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);
		}

		Toy_pushLiteralArray(&interpreter->stack, defaultLiteral);
	}

	Toy_freeLiteral(fnLiteral);
	Toy_freeLiteral(defaultLiteral);
	Toy_freeLiteral(selfLiteral);

	return 0;
}

static int nativeSome(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to some\n");
		return -1;
	}

	//get the args
	Toy_Literal fnLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_Literal fnLiteralIdn = fnLiteral;
	if (TOY_IS_IDENTIFIER(fnLiteral) && Toy_parseIdentifierToValue(interpreter, &fnLiteral)) {
		Toy_freeLiteral(fnLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(fnLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//check type
	if (!( TOY_IS_ARRAY(selfLiteral) || TOY_IS_DICTIONARY(selfLiteral) ) || !( TOY_IS_FUNCTION(fnLiteral) || TOY_IS_FUNCTION_NATIVE(fnLiteral) )) {
		interpreter->errorOutput("Incorrect argument type passed to some\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//call the given function on each element, based on the compound type
	if (TOY_IS_ARRAY(selfLiteral)) {
		bool result = false;

		//
		for (int i = 0; i < TOY_AS_ARRAY(selfLiteral)->count; i++) {
			Toy_Literal indexLiteral = TOY_TO_INTEGER_LITERAL(i);

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_pushLiteralArray(&arguments, indexLiteral);
			Toy_pushLiteralArray(&arguments, TOY_AS_ARRAY(selfLiteral)->literals[i]);

			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

			//grab the results
			Toy_Literal lit = Toy_popLiteralArray(&returns);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);
			Toy_freeLiteral(indexLiteral);

			//if not truthy
			if (TOY_IS_TRUTHY(lit)) {
				Toy_freeLiteral(lit);
				result = true;
				break;
			}

			Toy_freeLiteral(lit);
		}

		Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result);
		Toy_pushLiteralArray(&interpreter->stack, resultLiteral);
		Toy_freeLiteral(resultLiteral);
	}

	if (TOY_IS_DICTIONARY(selfLiteral)) {
		bool result = false;

		for (int i = 0; i < TOY_AS_DICTIONARY(selfLiteral)->capacity; i++) {
			//skip nulls
			if (TOY_IS_NULL(TOY_AS_DICTIONARY(selfLiteral)->entries[i].key)) {
				continue;
			}

			Toy_LiteralArray arguments;
			Toy_initLiteralArray(&arguments);
			Toy_pushLiteralArray(&arguments, TOY_AS_DICTIONARY(selfLiteral)->entries[i].key);
			Toy_pushLiteralArray(&arguments, TOY_AS_DICTIONARY(selfLiteral)->entries[i].value);

			Toy_LiteralArray returns;
			Toy_initLiteralArray(&returns);

			Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

			//grab the results
			Toy_Literal lit = Toy_popLiteralArray(&returns);

			Toy_freeLiteralArray(&arguments);
			Toy_freeLiteralArray(&returns);

			//if not truthy
			if (TOY_IS_TRUTHY(lit)) {
				Toy_freeLiteral(lit);
				result = true;
				break;
			}

			Toy_freeLiteral(lit);
		}

		Toy_Literal resultLiteral = TOY_TO_BOOLEAN_LITERAL(result);
		Toy_pushLiteralArray(&interpreter->stack, resultLiteral);
		Toy_freeLiteral(resultLiteral);
	}

	Toy_freeLiteral(fnLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static void swapLiteralsUtil(Toy_Literal* lhs, Toy_Literal* rhs) {
	Toy_Literal tmp = *lhs;
	*lhs = *rhs;
	*rhs = tmp;
}

//https://www.youtube.com/watch?v=MZaf_9IZCrc
static void recursiveLiteralQuicksortUtil(Toy_Interpreter* interpreter, Toy_Literal* ptr, int literalCount, Toy_Literal fnCompareLiteral) {
	//base case
	if (literalCount <= 1) {
		return;
	}

	int runner = 0;

	//iterate through the array
	for (int checker = 0; checker < literalCount - 1; checker++) {
		Toy_LiteralArray arguments;
		Toy_LiteralArray returns;

		Toy_initLiteralArray(&arguments);
		Toy_initLiteralArray(&returns);

		Toy_pushLiteralArray(&arguments, ptr[checker]);
		Toy_pushLiteralArray(&arguments, ptr[literalCount - 1]);

		Toy_callLiteralFn(interpreter, fnCompareLiteral, &arguments, &returns);

		Toy_Literal lessThan = Toy_popLiteralArray(&returns);

		Toy_freeLiteralArray(&arguments);
		Toy_freeLiteralArray(&returns);

		if (TOY_IS_TRUTHY(lessThan)) {
			swapLiteralsUtil(&ptr[runner++], &ptr[checker]);
		}

		Toy_freeLiteral(lessThan);
	}

	//"shift everything up" so the pivot is in the middle
	swapLiteralsUtil(&ptr[runner], &ptr[literalCount - 1]);

	//recurse on each end
	if (runner > 0) {
		recursiveLiteralQuicksortUtil(interpreter, &ptr[0], runner, fnCompareLiteral);
	}

	if (runner < literalCount) {
		recursiveLiteralQuicksortUtil(interpreter, &ptr[runner + 1], literalCount - runner - 1, fnCompareLiteral);
	}
}

static int nativeSort(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to sort\n");
		return -1;
	}

	//get the args
	Toy_Literal fnLiteral = Toy_popLiteralArray(arguments);
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to value if needed
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_Literal fnLiteralIdn = fnLiteral;
	if (TOY_IS_IDENTIFIER(fnLiteral) && Toy_parseIdentifierToValue(interpreter, &fnLiteral)) {
		Toy_freeLiteral(fnLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(fnLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//check type
	if (!TOY_IS_ARRAY(selfLiteral) || !( TOY_IS_FUNCTION(fnLiteral) || TOY_IS_FUNCTION_NATIVE(fnLiteral) )) {
		interpreter->errorOutput("Incorrect argument type passed to sort\n");
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(fnLiteral);
		return -1;
	}

	//BUGFIX: check if the array is already sorted
	bool sorted = true;
	for (int checker = 0; checker < TOY_AS_ARRAY(selfLiteral)->count - 1 && sorted; checker++) {
		Toy_LiteralArray arguments;
		Toy_LiteralArray returns;

		Toy_initLiteralArray(&arguments);
		Toy_initLiteralArray(&returns);

		Toy_pushLiteralArray(&arguments, TOY_AS_ARRAY(selfLiteral)->literals[checker]);
		Toy_pushLiteralArray(&arguments, TOY_AS_ARRAY(selfLiteral)->literals[checker + 1]);

		Toy_callLiteralFn(interpreter, fnLiteral, &arguments, &returns);

		Toy_Literal lessThan = Toy_popLiteralArray(&returns);

		Toy_freeLiteralArray(&arguments);
		Toy_freeLiteralArray(&returns);

		if (!TOY_IS_TRUTHY(lessThan)) {
			sorted = false;
		}

		Toy_freeLiteral(lessThan);
	}

	//call the quicksort util
	if (!sorted) {
		recursiveLiteralQuicksortUtil(interpreter, TOY_AS_ARRAY(selfLiteral)->literals, TOY_AS_ARRAY(selfLiteral)->count, fnLiteral);
	}

	Toy_pushLiteralArray(&interpreter->stack, selfLiteral);

	Toy_freeLiteral(fnLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeToLower(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to toLower\n");
		return -1;
	}

	//get the argument to a C-string
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral)) {
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	if (!TOY_IS_STRING(selfLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to toLower\n");
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	Toy_RefString* selfRefString = TOY_AS_STRING(selfLiteral);
	const char* self = Toy_toCString(selfRefString);

	//allocate buffer space for the result
	char* result = TOY_ALLOCATE(char, Toy_lengthRefString(selfRefString) + 1);

	//set each new character
	for (int i = 0; i < (int)Toy_lengthRefString(selfRefString); i++) {
		result[i] = tolower(self[i]);
	}
	result[Toy_lengthRefString(selfRefString)] = '\0'; //end the string

	//wrap up and push the new result onto the stack
	Toy_RefString* resultRefString = Toy_createRefStringLength(result, Toy_lengthRefString(selfRefString)); //internal copy
	Toy_Literal resultLiteral = TOY_TO_STRING_LITERAL(resultRefString); //NO copy

	Toy_pushLiteralArray(&interpreter->stack, resultLiteral); //internal copy

	//cleanup
	TOY_FREE_ARRAY(char, result, Toy_lengthRefString(resultRefString) + 1);
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static char* toStringUtilObject = NULL;
static void toStringUtil(const char* input) {
	size_t len = strlen(input) + 1;

	if (len > TOY_MAX_STRING_LENGTH) {
		len = TOY_MAX_STRING_LENGTH; //TODO: don't truncate
	}

	toStringUtilObject = TOY_ALLOCATE(char, len);

	snprintf(toStringUtilObject, len, "%s", input);
}

static int nativeToString(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to toString\n");
		return -1;
	}

	//get the argument
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	//parse to a value
	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral)) {
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	//BUGFIX: probably an undefined variable
	if (TOY_IS_IDENTIFIER(selfLiteral)) {
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	//print it to a custom function
	Toy_printLiteralCustom(selfLiteral, toStringUtil);

	//create the resulting string and push it
	Toy_Literal result = TOY_TO_STRING_LITERAL(Toy_createRefString(toStringUtilObject)); //internal copy

	Toy_pushLiteralArray(&interpreter->stack, result);

	//cleanup
	TOY_FREE_ARRAY(char, toStringUtilObject, Toy_lengthRefString( TOY_AS_STRING(result) ) + 1);
	toStringUtilObject = NULL;

	Toy_freeLiteral(result);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeToUpper(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to toUpper\n");
		return -1;
	}

	//get the argument to a C-string
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral)) {
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	if (!TOY_IS_STRING(selfLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to toUpper\n");
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	Toy_RefString* selfRefString = TOY_AS_STRING(selfLiteral);
	const char* self = Toy_toCString(selfRefString);

	//allocate buffer space for the result
	char* result = TOY_ALLOCATE(char, Toy_lengthRefString(selfRefString) + 1);

	//set each new character
	for (int i = 0; i < (int)Toy_lengthRefString(selfRefString); i++) {
		result[i] = toupper(self[i]);
	}
	result[Toy_lengthRefString(selfRefString)] = '\0'; //end the string

	//wrap up and push the new result onto the stack
	Toy_RefString* resultRefString = Toy_createRefStringLength(result, Toy_lengthRefString(selfRefString)); //internal copy
	Toy_Literal resultLiteral = TOY_TO_STRING_LITERAL(resultRefString); //NO copy

	Toy_pushLiteralArray(&interpreter->stack, resultLiteral); //internal copy

	//cleanup
	TOY_FREE_ARRAY(char, result, Toy_lengthRefString(resultRefString) + 1);
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeTrim(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count < 1 || arguments->count > 2) {
		interpreter->errorOutput("Incorrect number of arguments to trim\n");
		return -1;
	}

	//get the arguments
	Toy_Literal trimCharsLiteral;
	Toy_Literal selfLiteral;

	if (arguments->count == 2) {
		trimCharsLiteral = Toy_popLiteralArray(arguments);

		Toy_Literal trimCharsLiteralIdn = trimCharsLiteral;
		if (TOY_IS_IDENTIFIER(trimCharsLiteral) && Toy_parseIdentifierToValue(interpreter, &trimCharsLiteral)) {
			Toy_freeLiteral(trimCharsLiteralIdn);
		}
	}
	else {
		trimCharsLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(" \t\n\r"));
	}
	selfLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(trimCharsLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(trimCharsLiteral);
		return -1;
	}

	if (!TOY_IS_STRING(selfLiteral) || !TOY_IS_STRING(trimCharsLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to trim\n");
		Toy_freeLiteral(trimCharsLiteral);
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	//unwrap the arguments
	Toy_RefString* trimCharsRefString = TOY_AS_STRING(trimCharsLiteral);
	Toy_RefString* selfRefString = TOY_AS_STRING(selfLiteral);

	//allocate space for the new string
	size_t bufferBegin = 0;
	size_t bufferEnd = Toy_lengthRefString(selfRefString);

	//for each character in self, check it against each character in trimChars - on a fail, go to end
	for (int i = 0; i < (int)Toy_lengthRefString(selfRefString); i++) {
		int trimIndex = 0;

		while (trimIndex < (int)Toy_lengthRefString(trimCharsRefString)) {
			//there is a match - DON'T increment anymore
			if (Toy_toCString(selfRefString)[i] == Toy_toCString(trimCharsRefString)[trimIndex]) {
				break;
			}

			trimIndex++;
		}

		//if the match is found, increment buffer begin
		if (trimIndex < (int)Toy_lengthRefString(trimCharsRefString)) {
			bufferBegin++;
			continue;
		}
		else {
			break;
		}
	}

	//again, from the back
	for (int i = (int)Toy_lengthRefString(selfRefString); i >= 0; i--) {
		int trimIndex = 0;

		while (trimIndex < (int)Toy_lengthRefString(trimCharsRefString)) {
			//there is a match - DON'T increment anymore
			if (Toy_toCString(selfRefString)[i-1] == Toy_toCString(trimCharsRefString)[trimIndex]) {
				break;
			}

			trimIndex++;
		}

		//if the match is found, increment buffer begin
		if (trimIndex < (int)Toy_lengthRefString(trimCharsRefString)) {
			bufferEnd--;
			continue;
		}
		else {
			break;
		}
	}

	//generate the result
	Toy_Literal resultLiteral;
	if (bufferBegin >= bufferEnd) { //catch errors
		resultLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(""));
	}
	else {
		char buffer[TOY_MAX_STRING_LENGTH];
		snprintf(buffer, bufferEnd - bufferBegin + 1, "%s", &Toy_toCString(selfRefString)[ bufferBegin ]);
		resultLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(buffer)); //internal copy
	}

	//wrap up the buffer and return it
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral); //internal copy

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(trimCharsLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeTrimBegin(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count < 1 || arguments->count > 2) {
		interpreter->errorOutput("Incorrect number of arguments to trimBegin\n");
		return -1;
	}

	//get the arguments
	Toy_Literal trimCharsLiteral;
	Toy_Literal selfLiteral;

	if (arguments->count == 2) {
		trimCharsLiteral = Toy_popLiteralArray(arguments);

		Toy_Literal trimCharsLiteralIdn = trimCharsLiteral;
		if (TOY_IS_IDENTIFIER(trimCharsLiteral) && Toy_parseIdentifierToValue(interpreter, &trimCharsLiteral)) {
			Toy_freeLiteral(trimCharsLiteralIdn);
		}
	}
	else {
		trimCharsLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(" \t\n\r"));
	}
	selfLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(trimCharsLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(trimCharsLiteral);
		return -1;
	}

	if (!TOY_IS_STRING(selfLiteral) || !TOY_IS_STRING(trimCharsLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to trimBegin\n");
		Toy_freeLiteral(trimCharsLiteral);
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	//unwrap the arguments
	Toy_RefString* trimCharsRefString = TOY_AS_STRING(trimCharsLiteral);
	Toy_RefString* selfRefString = TOY_AS_STRING(selfLiteral);

	//allocate space for the new string
	size_t bufferBegin = 0;
	size_t bufferEnd = Toy_lengthRefString(selfRefString);

	//for each character in self, check it against each character in trimChars - on a fail, go to end
	for (int i = 0; i < (int)Toy_lengthRefString(selfRefString); i++) {
		int trimIndex = 0;

		while (trimIndex < (int)Toy_lengthRefString(trimCharsRefString)) {
			//there is a match - DON'T increment anymore
			if (Toy_toCString(selfRefString)[i] == Toy_toCString(trimCharsRefString)[trimIndex]) {
				break;
			}

			trimIndex++;
		}

		//if the match is found, increment buffer begin
		if (trimIndex < (int)Toy_lengthRefString(trimCharsRefString)) {
			bufferBegin++;
			continue;
		}
		else {
			break;
		}
	}

	//generate the result
	Toy_Literal resultLiteral;
	if (bufferBegin >= bufferEnd) { //catch errors
		resultLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(""));
	}
	else {
		char buffer[TOY_MAX_STRING_LENGTH];
		snprintf(buffer, bufferEnd - bufferBegin + 1, "%s", &Toy_toCString(selfRefString)[ bufferBegin ]);
		resultLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(buffer)); //internal copy
	}

	//wrap up the buffer and return it
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral); //internal copy

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(trimCharsLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

static int nativeTrimEnd(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	if (arguments->count < 1 || arguments->count > 2) {
		interpreter->errorOutput("Incorrect number of arguments to trimEnd\n");
		return -1;
	}

	//get the arguments
	Toy_Literal trimCharsLiteral;
	Toy_Literal selfLiteral;

	if (arguments->count == 2) {
		trimCharsLiteral = Toy_popLiteralArray(arguments);

		Toy_Literal trimCharsLiteralIdn = trimCharsLiteral;
		if (TOY_IS_IDENTIFIER(trimCharsLiteral) && Toy_parseIdentifierToValue(interpreter, &trimCharsLiteral)) {
			Toy_freeLiteral(trimCharsLiteralIdn);
		}
	}
	else {
		trimCharsLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(" \t\n\r"));
	}
	selfLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(selfLiteral) || TOY_IS_IDENTIFIER(trimCharsLiteral)) {
		Toy_freeLiteral(selfLiteral);
		Toy_freeLiteral(trimCharsLiteral);
		return -1;
	}

	if (!TOY_IS_STRING(selfLiteral) || !TOY_IS_STRING(trimCharsLiteral)) {
		interpreter->errorOutput("Incorrect argument type passed to trimEnd\n");
		Toy_freeLiteral(trimCharsLiteral);
		Toy_freeLiteral(selfLiteral);
		return -1;
	}

	//unwrap the arguments
	Toy_RefString* trimCharsRefString = TOY_AS_STRING(trimCharsLiteral);
	Toy_RefString* selfRefString = TOY_AS_STRING(selfLiteral);

	//allocate space for the new string
	size_t bufferBegin = 0;
	size_t bufferEnd = Toy_lengthRefString(selfRefString);

	//again, from the back
	for (int i = (int)Toy_lengthRefString(selfRefString); i >= 0; i--) {
		int trimIndex = 0;

		while (trimIndex < (int)Toy_lengthRefString(trimCharsRefString)) {
			//there is a match - DON'T increment anymore
			if (Toy_toCString(selfRefString)[i-1] == Toy_toCString(trimCharsRefString)[trimIndex]) {
				break;
			}

			trimIndex++;
		}

		//if the match is found, increment buffer begin
		if (trimIndex < (int)Toy_lengthRefString(trimCharsRefString)) {
			bufferEnd--;
			continue;
		}
		else {
			break;
		}
	}

	//generate the result
	Toy_Literal resultLiteral;
	if (bufferBegin >= bufferEnd) { //catch errors
		resultLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(""));
	}
	else {
		char buffer[TOY_MAX_STRING_LENGTH];
		snprintf(buffer, bufferEnd - bufferBegin + 1, "%s", &Toy_toCString(selfRefString)[ bufferBegin ]);
		resultLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(buffer)); //internal copy
	}

	//wrap up the buffer and return it
	Toy_pushLiteralArray(&interpreter->stack, resultLiteral); //internal copy

	//cleanup
	Toy_freeLiteral(resultLiteral);
	Toy_freeLiteral(trimCharsLiteral);
	Toy_freeLiteral(selfLiteral);

	return 1;
}

//call the hook
typedef struct Natives {
	char* name;
	Toy_NativeFn fn;
} Natives;

int Toy_hookStandard(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias) {
	//build the natives list
	Natives natives[] = {
		//misc. utils
		{"clock", nativeClock},
		{"hash", nativeHash},
		
		//math utils
		{"abs", nativeAbs},
		{"ceil", nativeCeil},
		{"floor", nativeFloor},
		{"max", nativeMax},
		{"min", nativeMin},
		{"round", nativeRound},

		//compound utils
		{"concat", nativeConcat}, //array, dictionary, string
		{"containsKey", nativeContainsKey}, //dictionary
		{"containsValue", nativeContainsValue}, //array, dictionary
		{"every", nativeEvery}, //array, dictionary
		{"filter", nativeFilter}, //array, dictionary
		{"forEach", nativeForEach}, //array, dictionary
		{"getKeys", nativeGetKeys}, //dictionary
		{"getValues", nativeGetValues}, //dictionary
		{"indexOf", nativeIndexOf}, //array
		{"map", nativeMap}, //array, dictionary
		{"reduce", nativeReduce}, //array, dictionary
		{"some", nativeSome}, //array, dictionary
		{"sort", nativeSort}, //array
		{"toLower", nativeToLower}, //string
		{"toString", nativeToString}, //array, dictionary
		{"toUpper", nativeToUpper}, //string
		{"trim", nativeTrim}, //string
		{"trimBegin", nativeTrimBegin}, //string
		{"trimEnd", nativeTrimEnd}, //string
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
