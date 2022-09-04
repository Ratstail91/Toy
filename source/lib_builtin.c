#include "lib_builtin.h"

#include "memory.h"

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