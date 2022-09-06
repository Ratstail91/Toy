#include "lib_builtin.h"

#include "memory.h"
#include "literal.h"

#include <stdio.h>

//static math utils
static Literal addition(Literal lhs, Literal rhs) {
	//TODO
}

static Literal subtraction(Literal lhs, Literal rhs) {
	//TODO
}

static Literal multiplication(Literal lhs, Literal rhs) {
	//TODO
}

static Literal division(Literal lhs, Literal rhs) {
	//TODO
}

static Literal modulo(Literal lhs, Literal rhs) {
	//TODO
}

int _index(Interpreter* interpreter, LiteralArray* arguments) {
	//_index(compound, first, second, third, assignValue, op)
	Literal op = popLiteralArray(arguments);
	Literal assign = popLiteralArray(arguments);
	Literal third = popLiteralArray(arguments);
	Literal second = popLiteralArray(arguments);
	Literal first = popLiteralArray(arguments);
	Literal compound = popLiteralArray(arguments);

	Literal value = TO_NULL_LITERAL;

	//dictionary - no slicing
	if (IS_DICTIONARY(compound)) {
		value = getLiteralDictionary(AS_DICTIONARY(compound), first);

		//dictionary
		//dictionary
		if (IS_NULL(op)) {
			pushLiteralArray(&interpreter->stack, value);

			freeLiteral(compound);
			freeLiteral(first);
			freeLiteral(second);
			freeLiteral(third);
			freeLiteral(op);
			freeLiteral(assign);
			freeLiteral(value);

			return 1;
		}

		else if (!strcmp( AS_STRING(op), "=")) {
			setLiteralDictionary(AS_DICTIONARY(compound), first, assign);
		}

		else if (!strcmp( AS_STRING(op), "+=")) {
			setLiteralDictionary(AS_DICTIONARY(compound), first, addition(value, assign));
		}

		else if (!strcmp( AS_STRING(op), "-=")) {
			setLiteralDictionary(AS_DICTIONARY(compound), first, subtraction(value, assign));
		}

		else if (!strcmp( AS_STRING(op), "*=")) {
			setLiteralDictionary(AS_DICTIONARY(compound), first, multiplication(value, assign));
		}

		else if (!strcmp( AS_STRING(op), "/=")) {
			setLiteralDictionary(AS_DICTIONARY(compound), first, division(value, assign));
		}

		else if (!strcmp( AS_STRING(op), "%=")) {
			setLiteralDictionary(AS_DICTIONARY(compound), first, modulo(value, assign));
		}
	}

	//array - slicing
	if (IS_ARRAY(compound)) {
		value = getLiteralArray(AS_ARRAY(compound), first);

		//array slice
		if (IS_NULL(op)) {
			//parse out the booleans & their defaults
			if (!IS_NULL(first)) {
				if (IS_BOOLEAN(first)) {
					freeLiteral(first);
					first = TO_INTEGER_LITERAL(0);
				}
			}

			if (!IS_NULL(second)) {
				if (IS_BOOLEAN(second)) {
					freeLiteral(second);
					second = TO_INTEGER_LITERAL(AS_ARRAY(compound)->count);
				}
			}

			if (IS_NULL(third) || IS_BOOLEAN(third)) {
				freeLiteral(third);
				third = TO_INTEGER_LITERAL(1);
			}

			//handle each null case
			if (IS_NULL(first) || !IS_INTEGER(first)) {
				//something is weird - skip out
				freeLiteral(compound);
				freeLiteral(first);
				freeLiteral(second);
				freeLiteral(third);
				freeLiteral(op);
				freeLiteral(assign);
				freeLiteral(value);

				return -1;
			}

			if (IS_NULL(second)) { //assign only a single character
				//get the "first" within the array, then skip out

				value = getLiteralArray(AS_ARRAY(compound), first);
				pushLiteralArray(&interpreter->stack, value);

				freeLiteral(compound);
				freeLiteral(first);
				freeLiteral(second);
				freeLiteral(third);
				freeLiteral(op);
				freeLiteral(assign);
				freeLiteral(value);

				return 1;
			}

			if (!IS_INTEGER(second) || (!IS_NULL(third) && !IS_INTEGER(third)) || AS_INTEGER(second) < 0 || AS_INTEGER(second) > AS_ARRAY(compound)->count || AS_INTEGER(third) == 0) {
				//something is weird - skip out
				freeLiteral(compound);
				freeLiteral(first);
				freeLiteral(second);
				freeLiteral(third);
				freeLiteral(op);
				freeLiteral(assign);
				freeLiteral(value);

				return -1;
			}

			//start building a new array from the old one
			LiteralArray* result = ALLOCATE(LiteralArray, 1);
			initLiteralArray(result);

			int min = AS_INTEGER(third) > 0 ? 0 : AS_INTEGER(second) - 1;

			//copy compound into result
			for (int i = min; i >= 0 && i <= AS_ARRAY(compound)->count && i >= AS_INTEGER(first) && i < AS_INTEGER(second); i += AS_INTEGER(third)) {
				Literal idx = TO_INTEGER_LITERAL(i);
				Literal tmp = getLiteralArray(AS_ARRAY(compound), idx);
				pushLiteralArray(result, tmp);

				freeLiteral(idx);
				freeLiteral(tmp);
			}

			//finally, swap out the compound for the result
			freeLiteral(compound);
			compound = TO_ARRAY_LITERAL(result);
		}

		//array slice assignment
		else if (!strcmp( AS_STRING(op), "=")) {
			//parse out the booleans & their defaults
			if (!IS_NULL(first)) {
				if (IS_BOOLEAN(first)) {
					freeLiteral(first);
					first = TO_INTEGER_LITERAL(0);
				}
			}

			if (!IS_NULL(second)) {
				if (IS_BOOLEAN(second)) {
					freeLiteral(second);
					second = TO_INTEGER_LITERAL(AS_ARRAY(compound)->count);
				}
			}

			if (IS_NULL(third) || IS_BOOLEAN(third)) {
				freeLiteral(third);
				third = TO_INTEGER_LITERAL(1);
			}

			//handle each null case
			if (IS_NULL(first) || !IS_INTEGER(first)) {
				//something is weird - skip out
				freeLiteral(compound);
				freeLiteral(first);
				freeLiteral(second);
				freeLiteral(third);
				freeLiteral(op);
				freeLiteral(assign);
				freeLiteral(value);

				return -1;
			}

			if (IS_NULL(second)) {
				//set the "first" within the array, then skip out
				setLiteralArray(AS_ARRAY(compound), first, assign);

				pushLiteralArray(&interpreter->stack, compound);

				freeLiteral(compound);
				freeLiteral(first);
				freeLiteral(second);
				freeLiteral(third);
				freeLiteral(op);
				freeLiteral(assign);
				freeLiteral(value);

				return 1;
			}

			if (!IS_INTEGER(second) || (!IS_NULL(third) && !IS_INTEGER(third)) || AS_INTEGER(second) < 0 || AS_INTEGER(second) > AS_ARRAY(compound)->count || AS_INTEGER(third) == 0) {
				//something is weird - skip out
				freeLiteral(compound);
				freeLiteral(first);
				freeLiteral(second);
				freeLiteral(third);
				freeLiteral(op);
				freeLiteral(assign);
				freeLiteral(value);

				return -1;
			}

			//start building a new array from the old one
			LiteralArray* result = ALLOCATE(LiteralArray, 1);
			initLiteralArray(result);

			//if third is abs(1), simply insert into the correct positions
			if (AS_INTEGER(third) == 1 || AS_INTEGER(third) == -1) {
				for (int i = 0; i < AS_INTEGER(first); i++) {
					Literal idx = TO_INTEGER_LITERAL(i);
					Literal tmp = getLiteralArray(AS_ARRAY(compound), idx);
					pushLiteralArray(result, tmp);

					freeLiteral(idx);
					freeLiteral(tmp);
				}

				int min = AS_INTEGER(third) > 0 ? 0 : AS_ARRAY(assign)->count - 1;

				for (int i = min; i >= 0 && i < AS_ARRAY(assign)->count; i += AS_INTEGER(third)) {
					Literal idx = TO_INTEGER_LITERAL(i);
					Literal tmp = getLiteralArray(AS_ARRAY(assign), idx); //backwards

					//set result
					pushLiteralArray(result, tmp);

					freeLiteral(idx);
					freeLiteral(tmp);
				}

				for (int i = AS_INTEGER(second) + 1; i < AS_ARRAY(compound)->count; i++) {
					Literal idx = TO_INTEGER_LITERAL(i);
					Literal tmp = getLiteralArray(AS_ARRAY(compound), idx);
					pushLiteralArray(result, tmp);

					freeLiteral(idx);
					freeLiteral(tmp);
				}
			}

			//else override elements of the array instead
			else {
				//copy compound to result
				for (int i = 0; i < AS_ARRAY(compound)->count; i++) {
					Literal idx = TO_INTEGER_LITERAL(i);
					Literal tmp = getLiteralArray(AS_ARRAY(compound), idx);

					pushLiteralArray(result, tmp);

					freeLiteral(idx);
					freeLiteral(tmp);
				}

				int min = AS_INTEGER(third) > 0 ? 0 : AS_ARRAY(compound)->count - 1;

				int assignIndex = 0;
				for (int i = min; i >= 0 && i < AS_ARRAY(compound)->count && assignIndex < AS_ARRAY(assign)->count; i += AS_INTEGER(third)) {
					Literal idx = TO_INTEGER_LITERAL(i);
					Literal ai = TO_INTEGER_LITERAL(assignIndex++);
					Literal tmp = getLiteralArray(AS_ARRAY(assign), ai);

					setLiteralArray(result, idx, tmp);

					freeLiteral(idx);
					freeLiteral(ai);
					freeLiteral(tmp);
				}
			}

			//finally, swap out the compound for the result
			freeLiteral(compound);
			compound = TO_ARRAY_LITERAL(result);
		}

		else if (IS_STRING(op) && !strcmp( AS_STRING(op), "+=")) {
			setLiteralArray(AS_ARRAY(compound), first, addition(value, assign));
		}

		else if (IS_STRING(op) && !strcmp( AS_STRING(op), "-=")) {
			setLiteralArray(AS_ARRAY(compound), first, subtraction(value, assign));
		}

		else if (IS_STRING(op) && !strcmp( AS_STRING(op), "*=")) {
			setLiteralArray(AS_ARRAY(compound), first, multiplication(value, assign));
		}

		else if (IS_STRING(op) && !strcmp( AS_STRING(op), "/=")) {
			setLiteralArray(AS_ARRAY(compound), first, division(value, assign));
		}

		else if (IS_STRING(op) && !strcmp( AS_STRING(op), "%=")) {
			setLiteralArray(AS_ARRAY(compound), first, modulo(value, assign));
		}
	}

	//string - slicing
	if (IS_STRING(compound)) {
		//string slice
		if (IS_NULL(op)) {
			//parse out the booleans & their defaults
			if (!IS_NULL(first)) {
				if (IS_BOOLEAN(first)) {
					freeLiteral(first);
					first = TO_INTEGER_LITERAL(0);
				}
			}

			if (!IS_NULL(second)) {
				if (IS_BOOLEAN(second)) {
					freeLiteral(second);
					second = TO_INTEGER_LITERAL(strlen(AS_STRING(compound)));
				}
			}

			if (IS_NULL(third) || IS_BOOLEAN(third)) {
				freeLiteral(third);
				third = TO_INTEGER_LITERAL(1);
			}

			//handle each null case
			if (IS_NULL(first) || !IS_INTEGER(first)) {
				//something is weird - skip out
				freeLiteral(compound);
				freeLiteral(first);
				freeLiteral(second);
				freeLiteral(third);
				freeLiteral(op);
				freeLiteral(assign);
				freeLiteral(value);

				return -1;
			}

			if (IS_NULL(second)) { //assign only a single character
				char c = AS_STRING(compound)[AS_INTEGER(first)];

				char buffer[16];
				snprintf(buffer, 16, "%c", c);

				freeLiteral(value);
				value = TO_STRING_LITERAL(copyString(buffer, strlen(buffer)), strlen(buffer));

				pushLiteralArray(&interpreter->stack, value);

				freeLiteral(compound);
				freeLiteral(first);
				freeLiteral(second);
				freeLiteral(third);
				freeLiteral(op);
				freeLiteral(assign);
				freeLiteral(value);

				return 1;
			}

			if (!IS_INTEGER(second) || (!IS_NULL(third) && !IS_INTEGER(third)) || AS_INTEGER(second) < 0 || AS_INTEGER(second) > strlen(AS_STRING(compound)) || AS_INTEGER(third) == 0) {
				//something is weird - skip out
				freeLiteral(compound);
				freeLiteral(first);
				freeLiteral(second);
				freeLiteral(third);
				freeLiteral(op);
				freeLiteral(assign);
				freeLiteral(value);

				return -1;
			}

			//start building a new string from the old one
			char* result = ALLOCATE(char, MAX_STRING_LENGTH);

			int min = AS_INTEGER(third) > 0 ? AS_INTEGER(first) : AS_INTEGER(second) - 1;

			//copy compound into result
			int resultIndex = 0;
			for (int i = min; i >= AS_INTEGER(first) && i < AS_INTEGER(second); i += AS_INTEGER(third)) {
				result[ resultIndex++ ] = AS_STRING(compound)[ i ];
			}

			result[ resultIndex++ ] = '\0';

			//finally, swap out the compound for the result
			freeLiteral(compound);
			compound = TO_STRING_LITERAL(copyString(result, strlen(result)), strlen(result));

			FREE_ARRAY(char, result, MAX_STRING_LENGTH);
		}

		//string slice assignment
		else if (IS_STRING(op) && !strcmp( AS_STRING(op), "=")) {
			//parse out the booleans & their defaults
			if (!IS_NULL(first)) {
				if (IS_BOOLEAN(first)) {
					freeLiteral(first);
					first = TO_INTEGER_LITERAL(0);
				}
			}

			if (!IS_NULL(second)) {
				if (IS_BOOLEAN(second)) {
					freeLiteral(second);
					second = TO_INTEGER_LITERAL(strlen(AS_STRING(compound)));
				}
			}

			if (IS_NULL(third) || IS_BOOLEAN(third)) {
				freeLiteral(third);
				third = TO_INTEGER_LITERAL(1);
			}

			//handle each null case
			if (IS_NULL(first) || !IS_INTEGER(first)) {
				//something is weird - skip out
				freeLiteral(compound);
				freeLiteral(first);
				freeLiteral(second);
				freeLiteral(third);
				freeLiteral(op);
				freeLiteral(assign);
				freeLiteral(value);

				return -1;
			}

			if (IS_NULL(second)) { //assign only a single character
				//set the "first" within the array, then skip out
				if (strlen( AS_STRING(assign) ) != 1) {
					//something is weird - skip out

					freeLiteral(compound);
					freeLiteral(first);
					freeLiteral(second);
					freeLiteral(third);
					freeLiteral(op);
					freeLiteral(assign);
					freeLiteral(value);

					return -1;
				}

				AS_STRING(compound)[AS_INTEGER(first)] = AS_STRING(assign)[0];

				pushLiteralArray(&interpreter->stack, compound);

				freeLiteral(compound);
				freeLiteral(first);
				freeLiteral(second);
				freeLiteral(third);
				freeLiteral(op);
				freeLiteral(assign);
				freeLiteral(value);

				return 1;
			}

			if (!IS_INTEGER(second) || (!IS_NULL(third) && !IS_INTEGER(third)) || AS_INTEGER(second) < 0 || AS_INTEGER(second) > strlen(AS_STRING(compound)) || AS_INTEGER(third) == 0) {
				//something is weird - skip out
				freeLiteral(compound);
				freeLiteral(first);
				freeLiteral(second);
				freeLiteral(third);
				freeLiteral(op);
				freeLiteral(assign);
				freeLiteral(value);

				return -1;
			}

			//start building a new string from the old one
			char* result = ALLOCATE(char, MAX_STRING_LENGTH);

			//if third is abs(1), simply insert into the correct positions
			int resultIndex = 0;
			if (AS_INTEGER(third) == 1 || AS_INTEGER(third) == -1) {
				for (int i = 0; i < AS_INTEGER(first); i++) {
					result[ resultIndex++ ] = AS_STRING(compound)[ i ];
				}

				int min = AS_INTEGER(third) > 0 ? 0 : strlen(AS_STRING(assign)) - 1;

				//TODO: optimize strlen(assign)
				for (int i = min; i >= 0 && i < strlen(AS_STRING(assign)); i += AS_INTEGER(third)) {
					result[ resultIndex++ ] = AS_STRING(assign)[ i ];
				}

				for (int i = AS_INTEGER(second) + 1; i < strlen(AS_STRING(compound)); i++) {
					result[ resultIndex++ ] = AS_STRING(compound)[ i ];
				}
			}

			//else override elements of the array instead
			else {
				//copy compound to result
				snprintf(result, MAX_STRING_LENGTH, AS_STRING(compound));

				int min = AS_INTEGER(third) > 0 ? AS_INTEGER(first) : AS_INTEGER(second);

				int assignIndex = 0;
				for (int i = min; i >= AS_INTEGER(first) && i <= AS_INTEGER(second) && assignIndex < strlen(AS_STRING(assign)); i += AS_INTEGER(third)) {
					result[ i ] = AS_STRING(assign)[ assignIndex++ ];
					resultIndex++;
				}
			}

			//finally, swap out the compound for the result
			freeLiteral(compound);
			compound = TO_STRING_LITERAL(copyString(result, strlen(result)), strlen(result));

			FREE_ARRAY(char, result, MAX_STRING_LENGTH);
		}

		else if (IS_STRING(op) && !strcmp( AS_STRING(op), "+=")) {
			Literal tmp = addition(compound, value);
			freeLiteral(compound);
			compound = tmp; //don't clear tmp
		}
	}

	//leave the compound on the stack
	pushLiteralArray(&interpreter->stack, compound);

	freeLiteral(compound);
	freeLiteral(first);
	freeLiteral(second);
	freeLiteral(third);
	freeLiteral(op);
	freeLiteral(assign);
	freeLiteral(value);

	return 1;
}

int _dot(Interpreter* interpreter, LiteralArray* arguments) {
	//_dot(compound, first, assignValue, opcode)
	Literal op = popLiteralArray(arguments);
	Literal assign = popLiteralArray(arguments);
	Literal first = popLiteralArray(arguments);
	Literal compound = popLiteralArray(arguments);

	Literal value = getLiteralDictionary(AS_DICTIONARY(compound), first);

	//dictionary
	if (IS_NULL(op)) {
		pushLiteralArray(&interpreter->stack, value);
	}

	else if (!strcmp( AS_STRING(op), "=")) {
		setLiteralDictionary(AS_DICTIONARY(compound), first, assign);
		pushLiteralArray(&interpreter->stack, compound);
	}

	else if (!strcmp( AS_STRING(op), "+=")) {
		setLiteralDictionary(AS_DICTIONARY(compound), first, addition(value, assign));
		pushLiteralArray(&interpreter->stack, compound);
	}

	else if (!strcmp( AS_STRING(op), "-=")) {
		setLiteralDictionary(AS_DICTIONARY(compound), first, subtraction(value, assign));
		pushLiteralArray(&interpreter->stack, compound);
	}

	else if (!strcmp( AS_STRING(op), "*=")) {
		setLiteralDictionary(AS_DICTIONARY(compound), first, multiplication(value, assign));
		pushLiteralArray(&interpreter->stack, compound);
	}

	else if (!strcmp( AS_STRING(op), "/=")) {
		setLiteralDictionary(AS_DICTIONARY(compound), first, division(value, assign));
		pushLiteralArray(&interpreter->stack, compound);
	}

	else if (!strcmp( AS_STRING(op), "%=")) {
		setLiteralDictionary(AS_DICTIONARY(compound), first, modulo(value, assign));
		pushLiteralArray(&interpreter->stack, compound);
	}

	//cleanup
	freeLiteral(compound);
	freeLiteral(first);
	freeLiteral(op);
	freeLiteral(assign);
	freeLiteral(value);

	return 1;
}

int _set(Interpreter* interpreter, LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 3) {
		interpreter->errorOutput("Incorrect number of arguments to _set\n");
		return -1;
	}

	Literal idn = arguments->literals[0];
	Literal obj = arguments->literals[0];
	Literal key = arguments->literals[1];
	Literal val = arguments->literals[2];

	if (!IS_IDENTIFIER(idn)) {
		interpreter->errorOutput("Expected identifier in _set\n");
		return -1;
	}

	parseIdentifierToValue(interpreter, &obj);

	bool freeKey = false;
	if (IS_IDENTIFIER(key)) {
		parseIdentifierToValue(interpreter, &key);
		freeKey = true;
	}

	bool freeVal = false;
	if (IS_IDENTIFIER(val)) {
		parseIdentifierToValue(interpreter, &val);
		freeVal = true;
	}

	switch(obj.type) {
		case LITERAL_ARRAY: {
			Literal typeLiteral = getScopeType(interpreter->scope, key);

			if (AS_TYPE(typeLiteral).typeOf == LITERAL_ARRAY) {
				Literal subtypeLiteral = ((Literal*)(AS_TYPE(typeLiteral).subtypes))[0];

				if (AS_TYPE(subtypeLiteral).typeOf != LITERAL_ANY && AS_TYPE(subtypeLiteral).typeOf != val.type) {
					interpreter->errorOutput("Bad argument type in _set\n");
					return -1;
				}
			}

			if (!IS_INTEGER(key)) {
				interpreter->errorOutput("Expected integer index in _set\n");
				return -1;
			}

			if (AS_ARRAY(obj)->count <= AS_INTEGER(key) || AS_INTEGER(key) < 0) {
				interpreter->errorOutput("Index out of bounds in _set\n");
				return -1;
			}

			//don't use pushLiteralArray, since we're setting
			freeLiteral(AS_ARRAY(obj)->literals[AS_INTEGER(key)]); //BUGFIX: clear any existing data first
			AS_ARRAY(obj)->literals[AS_INTEGER(key)] = copyLiteral(val);

			if (!setScopeVariable(interpreter->scope, idn, obj, true)) {
				interpreter->errorOutput("Incorrect type assigned to array in _set: \"");
				printLiteralCustom(val, interpreter->errorOutput);
				interpreter->errorOutput("\"\n");
				return -1;
			}

			break;
		}

		case LITERAL_DICTIONARY: {
			Literal typeLiteral = getScopeType(interpreter->scope, key);

			if (AS_TYPE(typeLiteral).typeOf == LITERAL_DICTIONARY) {
				Literal keySubtypeLiteral = ((Literal*)(AS_TYPE(typeLiteral).subtypes))[0];
				Literal valSubtypeLiteral = ((Literal*)(AS_TYPE(typeLiteral).subtypes))[1];

				if (AS_TYPE(keySubtypeLiteral).typeOf != LITERAL_ANY && AS_TYPE(keySubtypeLiteral).typeOf != key.type) {
					interpreter->printOutput("bad argument type in _set\n");
					return -1;
				}

				if (AS_TYPE(valSubtypeLiteral).typeOf != LITERAL_ANY && AS_TYPE(valSubtypeLiteral).typeOf != val.type) {
					interpreter->printOutput("bad argument type in _set\n");
					return -1;
				}
			}

			setLiteralDictionary(AS_DICTIONARY(obj), key, val);

			if (!setScopeVariable(interpreter->scope, idn, obj, true)) {
				interpreter->errorOutput("Incorrect type assigned to dictionary in _set: \"");
				printLiteralCustom(val, interpreter->errorOutput);
				interpreter->errorOutput("\"\n");
				return -1;
			}

			break;
		}

		default:
			interpreter->errorOutput("Incorrect compound type in _set: ");
			printLiteralCustom(obj, interpreter->errorOutput);
			interpreter->errorOutput("\"\n");
			return -1;
	}

	freeLiteral(obj);

	if (freeKey) {
		freeLiteral(key);
	}

	if (freeVal) {
		freeLiteral(val);
	}

	return 0;
}

int _get(Interpreter* interpreter, LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to _get");
		return -1;
	}

	Literal obj = arguments->literals[0];
	Literal key = arguments->literals[1];

	bool freeObj = false;
	if (IS_IDENTIFIER(obj)) {
		parseIdentifierToValue(interpreter, &obj);
		freeObj = true;
	}

	bool freeKey = false;
	if (IS_IDENTIFIER(key)) {
		parseIdentifierToValue(interpreter, &key);
		freeKey = true;
	}

	switch(obj.type) {
		case LITERAL_ARRAY: {
			if (!IS_INTEGER(key)) {
				interpreter->errorOutput("Expected integer index in _get\n");
				return -1;
			}

			if (AS_ARRAY(obj)->count <= AS_INTEGER(key) || AS_INTEGER(key) < 0) {
				interpreter->errorOutput("Index out of bounds in _get\n");
				return -1;
			}

			pushLiteralArray(&interpreter->stack, AS_ARRAY(obj)->literals[AS_INTEGER(key)]);

			if (freeObj) {
				freeLiteral(obj);
			}

			if (freeKey) {
				freeLiteral(key);
			}

			return 1;
		}

		case LITERAL_DICTIONARY: {
			Literal dict = getLiteralDictionary(AS_DICTIONARY(obj), key);
			pushLiteralArray(&interpreter->stack, dict);
			freeLiteral(dict);

			if (freeObj) {
				freeLiteral(obj);
			}

			if (freeKey) {
				freeLiteral(key);
			}

			return 1;
		}

		default:
			interpreter->errorOutput("Incorrect compound type in _get \"");
			printLiteralCustom(obj, interpreter->errorOutput);
			interpreter->errorOutput("\"\n");
			return -1;
	}
}

int _push(Interpreter* interpreter, LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to _push\n");
		return -1;
	}

	Literal idn = arguments->literals[0];
	Literal obj = arguments->literals[0];
	Literal val = arguments->literals[1];

	if (!IS_IDENTIFIER(idn)) {
		interpreter->errorOutput("Expected identifier in _push\n");
		return -1;
	}

	parseIdentifierToValue(interpreter, &obj);

	bool freeVal = false;
	if (IS_IDENTIFIER(val)) {
		parseIdentifierToValue(interpreter, &val);
		freeVal = true;
	}

	switch(obj.type) {
		case LITERAL_ARRAY: {
			Literal typeLiteral = getScopeType(interpreter->scope, val);

			if (AS_TYPE(typeLiteral).typeOf == LITERAL_ARRAY) {
				Literal subtypeLiteral = ((Literal*)(AS_TYPE(typeLiteral).subtypes))[0];

				if (AS_TYPE(subtypeLiteral).typeOf != LITERAL_ANY && AS_TYPE(subtypeLiteral).typeOf != val.type) {
					interpreter->errorOutput("Bad argument type in _push");
					return -1;
				}
			}

			pushLiteralArray(AS_ARRAY(obj), val);

			if (!setScopeVariable(interpreter->scope, idn, obj, true)) { //TODO: could definitely be more efficient than overwriting the whole original object
				interpreter->errorOutput("Incorrect type assigned to array in _push: \"");
				printLiteralCustom(val, interpreter->errorOutput);
				interpreter->errorOutput("\"\n");
				return -1;
			}

			freeLiteral(obj);

			if (freeVal) {
				freeLiteral(val);
			}

			return 0;
		}

		default:
			interpreter->errorOutput("Incorrect compound type in _push: ");
			printLiteralCustom(obj, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			return -1;
	}
}

int _pop(Interpreter* interpreter, LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _pop\n");
		return -1;
	}

	Literal idn = arguments->literals[0];
	Literal obj = arguments->literals[0];

	if (!IS_IDENTIFIER(idn)) {
		interpreter->errorOutput("Expected identifier in _pop\n");
		return -1;
	}

	parseIdentifierToValue(interpreter, &obj);

	switch(obj.type) {
		case LITERAL_ARRAY: {
			Literal lit = popLiteralArray(AS_ARRAY(obj));
			pushLiteralArray(&interpreter->stack, lit);
			freeLiteral(lit);

			if (!setScopeVariable(interpreter->scope, idn, obj, true)) { //TODO: could definitely be more efficient than overwriting the whole original object
				interpreter->errorOutput("Incorrect type assigned to array in _pop: ");
				printLiteralCustom(obj, interpreter->errorOutput);
				interpreter->errorOutput("\n");
				return -1;
			}

			freeLiteral(obj);

			return 1;
		}

		default:
			interpreter->errorOutput("Incorrect compound type in _pop: ");
			printLiteralCustom(obj, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			return -1;
	}
}

int _length(Interpreter* interpreter, LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _get\n");
		return -1;
	}

	Literal obj = arguments->literals[0];

	bool freeObj = false;
	if (IS_IDENTIFIER(obj)) {
		parseIdentifierToValue(interpreter, &obj);
		freeObj = true;
	}

	switch(obj.type) {
		case LITERAL_ARRAY: {
			Literal lit = TO_INTEGER_LITERAL( AS_ARRAY(obj)->count );
			pushLiteralArray(&interpreter->stack, lit);
			freeLiteral(lit);
			break;
		}

		case LITERAL_DICTIONARY: {
			Literal lit = TO_INTEGER_LITERAL( AS_DICTIONARY(obj)->count );
			pushLiteralArray(&interpreter->stack, lit);
			freeLiteral(lit);
			break;
		}

		case LITERAL_STRING: {
			Literal lit = TO_INTEGER_LITERAL( strlen(AS_STRING(obj)) );
			pushLiteralArray(&interpreter->stack, lit);
			freeLiteral(lit);
			break;
		}

		default:
			interpreter->errorOutput("Incorrect compound type in _length: ");
			printLiteralCustom(obj, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			return -1;
	}

	if (freeObj) {
		freeLiteral(obj);
	}

	return 1;
}

int _clear(Interpreter* interpreter, LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _clear\n");
		return -1;
	}

	Literal idn = arguments->literals[0];
	Literal obj = arguments->literals[0];

	if (!IS_IDENTIFIER(idn)) {
		interpreter->errorOutput("expected identifier in _clear\n");
		return -1;
	}

	parseIdentifierToValue(interpreter, &obj);

	//NOTE: just pass in new compounds

	switch(obj.type) {
		case LITERAL_ARRAY: {
			LiteralArray* array = ALLOCATE(LiteralArray, 1);
			initLiteralArray(array);

			Literal obj = TO_ARRAY_LITERAL(array);

			if (!setScopeVariable(interpreter->scope, idn, obj, true)) {
				interpreter->errorOutput("Incorrect type assigned to array in _clear: ");
				printLiteralCustom(obj, interpreter->errorOutput);
				interpreter->errorOutput("\n");
				return -1;
			}

			freeLiteral(obj);

			break;
		}

		case LITERAL_DICTIONARY: {
			LiteralDictionary* dictionary = ALLOCATE(LiteralDictionary, 1);
			initLiteralDictionary(dictionary);

			Literal obj = TO_DICTIONARY_LITERAL(dictionary);

			if (!setScopeVariable(interpreter->scope, idn, obj, true)) {
				interpreter->errorOutput("Incorrect type assigned to dictionary in _clear: ");
				printLiteralCustom(obj, interpreter->errorOutput);
				interpreter->errorOutput("\n");
				return -1;
			}

			freeLiteral(obj);

			break;
		}

		default:
			interpreter->errorOutput("Incorrect compound type in _clear: ");
			printLiteralCustom(obj, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			return -1;
	}

	freeLiteral(obj);
	return 1;
}
