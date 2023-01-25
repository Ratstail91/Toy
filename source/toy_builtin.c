#include "toy_builtin.h"

#include "toy_memory.h"
#include "toy_literal.h"

#include <stdio.h>

//static math utils, copied from the interpreter
static Toy_Literal addition(Toy_Interpreter* interpreter, Toy_Literal lhs, Toy_Literal rhs) {
	//special case for string concatenation ONLY
	if (TOY_IS_STRING(lhs) && TOY_IS_STRING(rhs)) {
		//check for overflow
		int totalLength = TOY_AS_STRING(lhs)->length + TOY_AS_STRING(rhs)->length;
		if (totalLength > TOY_MAX_STRING_LENGTH) {
			interpreter->errorOutput("Can't concatenate these strings (result is too long)\n");
			return TOY_TO_NULL_LITERAL;
		}

		//concat the strings
		char buffer[TOY_MAX_STRING_LENGTH];
		snprintf(buffer, TOY_MAX_STRING_LENGTH, "%s%s", Toy_toCString(TOY_AS_STRING(lhs)), Toy_toCString(TOY_AS_STRING(rhs)));
		Toy_Literal literal = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(buffer, totalLength));

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);

		return literal;
	}

	//type coersion
	if (TOY_IS_FLOAT(lhs) && TOY_IS_INTEGER(rhs)) {
		rhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(rhs));
	}

	if (TOY_IS_INTEGER(lhs) && TOY_IS_FLOAT(rhs)) {
		lhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(lhs));
	}

	//results
	Toy_Literal result = TOY_TO_NULL_LITERAL;

	if (TOY_IS_INTEGER(lhs) && TOY_IS_INTEGER(rhs)) {
		result = TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) + TOY_AS_INTEGER(rhs) );

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);

		return result;
	}

	if (TOY_IS_FLOAT(lhs) && TOY_IS_FLOAT(rhs)) {
		result = TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(lhs) + TOY_AS_FLOAT(rhs) );

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);

		return result;
	}

	//wrong types
	interpreter->errorOutput("Bad arithmetic argument ");
	Toy_printLiteralCustom(lhs, interpreter->errorOutput);
	interpreter->errorOutput(" and ");
	Toy_printLiteralCustom(rhs, interpreter->errorOutput);
	interpreter->errorOutput("\n");

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return TOY_TO_NULL_LITERAL;
}

static Toy_Literal subtraction(Toy_Interpreter* interpreter, Toy_Literal lhs, Toy_Literal rhs) {
	//type coersion
	if (TOY_IS_FLOAT(lhs) && TOY_IS_INTEGER(rhs)) {
		rhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(rhs));
	}

	if (TOY_IS_INTEGER(lhs) && TOY_IS_FLOAT(rhs)) {
		lhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(lhs));
	}

	//results
	Toy_Literal result = TOY_TO_NULL_LITERAL;

	if (TOY_IS_INTEGER(lhs) && TOY_IS_INTEGER(rhs)) {
		result = TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) - TOY_AS_INTEGER(rhs) );

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);

		return result;
	}

	if (TOY_IS_FLOAT(lhs) && TOY_IS_FLOAT(rhs)) {
		result = TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(lhs) - TOY_AS_FLOAT(rhs) );

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);

		return result;
	}

	//wrong types
	interpreter->errorOutput("Bad arithmetic argument ");
	Toy_printLiteralCustom(lhs, interpreter->errorOutput);
	interpreter->errorOutput(" and ");
	Toy_printLiteralCustom(rhs, interpreter->errorOutput);
	interpreter->errorOutput("\n");

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return TOY_TO_NULL_LITERAL;
}

static Toy_Literal multiplication(Toy_Interpreter* interpreter, Toy_Literal lhs, Toy_Literal rhs) {
	//type coersion
	if (TOY_IS_FLOAT(lhs) && TOY_IS_INTEGER(rhs)) {
		rhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(rhs));
	}

	if (TOY_IS_INTEGER(lhs) && TOY_IS_FLOAT(rhs)) {
		lhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(lhs));
	}

	//results
	Toy_Literal result = TOY_TO_NULL_LITERAL;

	if (TOY_IS_INTEGER(lhs) && TOY_IS_INTEGER(rhs)) {
		result = TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) * TOY_AS_INTEGER(rhs) );

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);

		return result;
	}

	if (TOY_IS_FLOAT(lhs) && TOY_IS_FLOAT(rhs)) {
		result = TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(lhs) * TOY_AS_FLOAT(rhs) );

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);

		return result;
	}

	//wrong types
	interpreter->errorOutput("Bad arithmetic argument ");
	Toy_printLiteralCustom(lhs, interpreter->errorOutput);
	interpreter->errorOutput(" and ");
	Toy_printLiteralCustom(rhs, interpreter->errorOutput);
	interpreter->errorOutput("\n");

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return TOY_TO_NULL_LITERAL;
}

static Toy_Literal division(Toy_Interpreter* interpreter, Toy_Literal lhs, Toy_Literal rhs) {
	//division check
	if ((TOY_IS_INTEGER(rhs) && TOY_AS_INTEGER(rhs) == 0) || (TOY_IS_FLOAT(rhs) && TOY_AS_FLOAT(rhs) == 0)) {
		interpreter->errorOutput("Can't divide by zero");
	}

	//type coersion
	if (TOY_IS_FLOAT(lhs) && TOY_IS_INTEGER(rhs)) {
		rhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(rhs));
	}

	if (TOY_IS_INTEGER(lhs) && TOY_IS_FLOAT(rhs)) {
		lhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(lhs));
	}

	//results
	Toy_Literal result = TOY_TO_NULL_LITERAL;

	if (TOY_IS_INTEGER(lhs) && TOY_IS_INTEGER(rhs)) {
		result = TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) + TOY_AS_INTEGER(rhs) );

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);

		return result;
	}

	if (TOY_IS_FLOAT(lhs) && TOY_IS_FLOAT(rhs)) {
		result = TOY_TO_FLOAT_LITERAL( TOY_AS_FLOAT(lhs) + TOY_AS_FLOAT(rhs) );

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);

		return result;
	}

	//wrong types
	interpreter->errorOutput("Bad arithmetic argument ");
	Toy_printLiteralCustom(lhs, interpreter->errorOutput);
	interpreter->errorOutput(" and ");
	Toy_printLiteralCustom(rhs, interpreter->errorOutput);
	interpreter->errorOutput("\n");

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return TOY_TO_NULL_LITERAL;
}

static Toy_Literal modulo(Toy_Interpreter* interpreter, Toy_Literal lhs, Toy_Literal rhs) {
	//division check
	if ((TOY_IS_INTEGER(rhs) && TOY_AS_INTEGER(rhs) == 0) || (TOY_IS_FLOAT(rhs) && TOY_AS_FLOAT(rhs) == 0)) {
		interpreter->errorOutput("Can't divide by zero");
	}

	//type coersion
	if (TOY_IS_FLOAT(lhs) && TOY_IS_INTEGER(rhs)) {
		rhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(rhs));
	}

	if (TOY_IS_INTEGER(lhs) && TOY_IS_FLOAT(rhs)) {
		lhs = TOY_TO_FLOAT_LITERAL(TOY_AS_INTEGER(lhs));
	}

	//results
	Toy_Literal result = TOY_TO_NULL_LITERAL;

	if (TOY_IS_INTEGER(lhs) && TOY_IS_INTEGER(rhs)) {
		result = TOY_TO_INTEGER_LITERAL( TOY_AS_INTEGER(lhs) + TOY_AS_INTEGER(rhs) );

		Toy_freeLiteral(lhs);
		Toy_freeLiteral(rhs);

		return result;
	}

	//NOTE: no float modulo

	//wrong types
	interpreter->errorOutput("Bad arithmetic argument ");
	Toy_printLiteralCustom(lhs, interpreter->errorOutput);
	interpreter->errorOutput(" and ");
	Toy_printLiteralCustom(rhs, interpreter->errorOutput);
	interpreter->errorOutput("\n");

	Toy_freeLiteral(lhs);
	Toy_freeLiteral(rhs);

	return TOY_TO_NULL_LITERAL;
}

int _index(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//_index(compound, first, second, third, assignValue, op)
	Toy_Literal op = Toy_popLiteralArray(arguments);
	Toy_Literal assign = Toy_popLiteralArray(arguments);
	Toy_Literal third = Toy_popLiteralArray(arguments);
	Toy_Literal second = Toy_popLiteralArray(arguments);
	Toy_Literal first = Toy_popLiteralArray(arguments);
	Toy_Literal compound = Toy_popLiteralArray(arguments);

	Toy_Literal value = TOY_TO_NULL_LITERAL;

	//dictionary - no slicing
	if (TOY_IS_DICTIONARY(compound)) {
		if (TOY_IS_IDENTIFIER(first)) {
			Toy_Literal idn = first;
			Toy_parseIdentifierToValue(interpreter, &first);
			Toy_freeLiteral(idn);
		}

		if (TOY_IS_IDENTIFIER(second)) {
			Toy_Literal idn = second;
			Toy_parseIdentifierToValue(interpreter, &second);
			Toy_freeLiteral(idn);
		}

		if (TOY_IS_IDENTIFIER(third)) {
			Toy_Literal idn = third;
			Toy_parseIdentifierToValue(interpreter, &third);
			Toy_freeLiteral(idn);
		}

		value = Toy_getLiteralDictionary(TOY_AS_DICTIONARY(compound), first);

		//dictionary
		if (TOY_IS_NULL(op)) {
			Toy_pushLiteralArray(&interpreter->stack, value);

			Toy_freeLiteral(op);
			Toy_freeLiteral(assign);
			Toy_freeLiteral(third);
			Toy_freeLiteral(second);
			Toy_freeLiteral(first);
			Toy_freeLiteral(compound);
			Toy_freeLiteral(value);

			return 1;
		}

		else if (Toy_equalsRefStringCString(TOY_AS_STRING(op), "=")) {
			Toy_setLiteralDictionary(TOY_AS_DICTIONARY(compound), first, assign);
		}

		else if (Toy_equalsRefStringCString(TOY_AS_STRING(op), "+=")) {
			Toy_Literal lit = addition(interpreter, value, assign);
			Toy_setLiteralDictionary(TOY_AS_DICTIONARY(compound), first, lit);
			Toy_freeLiteral(lit);
		}

		else if (Toy_equalsRefStringCString(TOY_AS_STRING(op), "-=")) {
			Toy_Literal lit = subtraction(interpreter, value, assign);
			Toy_setLiteralDictionary(TOY_AS_DICTIONARY(compound), first, lit);
			Toy_freeLiteral(lit);
		}

		else if (Toy_equalsRefStringCString(TOY_AS_STRING(op), "*=")) {
			Toy_Literal lit = multiplication(interpreter, value, assign);
			Toy_setLiteralDictionary(TOY_AS_DICTIONARY(compound), first, lit);
			Toy_freeLiteral(lit);
		}

		else if (Toy_equalsRefStringCString(TOY_AS_STRING(op), "/=")) {
			Toy_Literal lit = division(interpreter, value, assign);
			Toy_setLiteralDictionary(TOY_AS_DICTIONARY(compound), first, lit);
			Toy_freeLiteral(lit);
		}

		else if (Toy_equalsRefStringCString(TOY_AS_STRING(op), "%=")) {
			Toy_Literal lit = modulo(interpreter, value, assign);
			Toy_setLiteralDictionary(TOY_AS_DICTIONARY(compound), first, lit);
			Toy_freeLiteral(lit);
		}
	}

	//array - slicing
	if (TOY_IS_ARRAY(compound)) {
		//array slice
		if (TOY_IS_NULL(op)) {
			//parse out the blanks & their defaults
			if (!TOY_IS_NULL(first)) {
				if (TOY_IS_INDEX_BLANK(first)) {
					Toy_freeLiteral(first);
					first = TOY_TO_INTEGER_LITERAL(0);
				}

				if (TOY_IS_IDENTIFIER(first)) {
					Toy_Literal idn = first;
					Toy_parseIdentifierToValue(interpreter, &first);
					Toy_freeLiteral(idn);
				}
			}

			if (!TOY_IS_NULL(second)) {
				if (TOY_IS_INDEX_BLANK(second)) {
					Toy_freeLiteral(second);
					second = TOY_TO_INTEGER_LITERAL(TOY_AS_ARRAY(compound)->count - 1);
				}

				if (TOY_IS_IDENTIFIER(second)) {
					Toy_Literal idn = second;
					Toy_parseIdentifierToValue(interpreter, &second);
					Toy_freeLiteral(idn);
				}
			}

			if (TOY_IS_NULL(third) || TOY_IS_INDEX_BLANK(third)) {
				Toy_freeLiteral(third);
				third = TOY_TO_INTEGER_LITERAL(1);
			}

			if (TOY_IS_IDENTIFIER(third)) {
				Toy_Literal idn = third;
				Toy_parseIdentifierToValue(interpreter, &third);
				Toy_freeLiteral(idn);
			}

			//handle each null case
			if (TOY_IS_NULL(first) || !TOY_IS_INTEGER(first)) {
				//something is weird - skip out
				Toy_freeLiteral(op);
				Toy_freeLiteral(assign);
				Toy_freeLiteral(third);
				Toy_freeLiteral(second);
				Toy_freeLiteral(first);
				Toy_freeLiteral(compound);
				Toy_freeLiteral(value);

				return -1;
			}

			if (TOY_IS_NULL(second)) { //assign only a single character
				//get the "first" within the array, then skip out

				Toy_freeLiteral(value);
				value = Toy_getLiteralArray(TOY_AS_ARRAY(compound), first);
				Toy_pushLiteralArray(&interpreter->stack, value);

				Toy_freeLiteral(op);
				Toy_freeLiteral(assign);
				Toy_freeLiteral(third);
				Toy_freeLiteral(second);
				Toy_freeLiteral(first);
				Toy_freeLiteral(compound);
				Toy_freeLiteral(value);

				return 1;
			}

			if (!TOY_IS_INTEGER(second) || (!TOY_IS_NULL(third) && !TOY_IS_INTEGER(third)) || TOY_AS_INTEGER(second) < 0 || TOY_AS_INTEGER(second) > TOY_AS_ARRAY(compound)->count || TOY_AS_INTEGER(third) == 0) {
				//something is weird - skip out
				Toy_freeLiteral(op);
				Toy_freeLiteral(assign);
				Toy_freeLiteral(third);
				Toy_freeLiteral(second);
				Toy_freeLiteral(first);
				Toy_freeLiteral(compound);
				Toy_freeLiteral(value);

				return -1;
			}

			//start building a new array from the old one
			Toy_LiteralArray* result = TOY_ALLOCATE(Toy_LiteralArray, 1);
			Toy_initLiteralArray(result);

			int min = TOY_AS_INTEGER(third) > 0 ? TOY_AS_INTEGER(first) : TOY_AS_INTEGER(second);

			//copy compound into result
			for (int i = min; i >= 0 && i <= TOY_AS_ARRAY(compound)->count && i >= TOY_AS_INTEGER(first) && i <= TOY_AS_INTEGER(second); i += TOY_AS_INTEGER(third)) {
				Toy_Literal idx = TOY_TO_INTEGER_LITERAL(i);
				Toy_Literal tmp = Toy_getLiteralArray(TOY_AS_ARRAY(compound), idx);
				Toy_pushLiteralArray(result, tmp);

				Toy_freeLiteral(idx);
				Toy_freeLiteral(tmp);
			}

			//finally, swap out the compound for the result
			Toy_freeLiteral(compound);
			compound = TOY_TO_ARRAY_LITERAL(result);
		}

		//array slice assignment
		if (TOY_IS_STRING(op) && Toy_equalsRefStringCString(TOY_AS_STRING(op), "=")) {
			//parse out the blanks & their defaults
			if (!TOY_IS_NULL(first)) {
				if (TOY_IS_INDEX_BLANK(first)) {
					Toy_freeLiteral(first);
					first = TOY_TO_INTEGER_LITERAL(0);
				}

				if (TOY_IS_IDENTIFIER(first)) {
					Toy_Literal idn = first;
					Toy_parseIdentifierToValue(interpreter, &first);
					Toy_freeLiteral(idn);
				}
			}

			if (!TOY_IS_NULL(second)) {
				if (TOY_IS_INDEX_BLANK(second)) {
					Toy_freeLiteral(second);
					second = TOY_TO_INTEGER_LITERAL(TOY_AS_INTEGER(first));
				}

				if (TOY_IS_IDENTIFIER(second)) {
					Toy_Literal idn = second;
					Toy_parseIdentifierToValue(interpreter, &second);
					Toy_freeLiteral(idn);
				}
			}

			if (TOY_IS_NULL(third) || TOY_IS_INDEX_BLANK(third)) {
				Toy_freeLiteral(third);
				third = TOY_TO_INTEGER_LITERAL(1);
			}

			if (TOY_IS_IDENTIFIER(third)) {
				Toy_Literal idn = third;
				Toy_parseIdentifierToValue(interpreter, &third);
				Toy_freeLiteral(idn);
			}

			//handle each null case
			if (TOY_IS_NULL(first) || !TOY_IS_INTEGER(first)) {
				//something is weird - skip out
				Toy_freeLiteral(op);
				Toy_freeLiteral(assign);
				Toy_freeLiteral(third);
				Toy_freeLiteral(second);
				Toy_freeLiteral(first);
				Toy_freeLiteral(compound);
				Toy_freeLiteral(value);

				return -1;
			}

			if (TOY_IS_NULL(second)) {
				//set the "first" within the array, then skip out
				if (!Toy_setLiteralArray(TOY_AS_ARRAY(compound), first, assign)) {
					interpreter->errorOutput("Index assignment out of bounds\n");

					Toy_freeLiteral(op);
					Toy_freeLiteral(assign);
					Toy_freeLiteral(third);
					Toy_freeLiteral(second);
					Toy_freeLiteral(first);
					Toy_freeLiteral(compound);
					Toy_freeLiteral(value);

					return -1;
				}

				Toy_pushLiteralArray(&interpreter->stack, compound);

				Toy_freeLiteral(op);
				Toy_freeLiteral(assign);
				Toy_freeLiteral(third);
				Toy_freeLiteral(second);
				Toy_freeLiteral(first);
				Toy_freeLiteral(compound);
				Toy_freeLiteral(value);

				return 1;
			}

			if (!TOY_IS_INTEGER(second) || (!TOY_IS_NULL(third) && !TOY_IS_INTEGER(third)) || TOY_AS_INTEGER(second) < 0 || TOY_AS_INTEGER(second) > TOY_AS_ARRAY(compound)->count || TOY_AS_INTEGER(third) == 0) {
				//something is weird - skip out
				Toy_freeLiteral(op);
				Toy_freeLiteral(assign);
				Toy_freeLiteral(third);
				Toy_freeLiteral(second);
				Toy_freeLiteral(first);
				Toy_freeLiteral(compound);
				Toy_freeLiteral(value);

				return -1;
			}

			//start building a new array from the old one
			Toy_LiteralArray* result = TOY_ALLOCATE(Toy_LiteralArray, 1);
			Toy_initLiteralArray(result);

			//if third is abs(1), simply insert into the correct positions
			if (TOY_AS_INTEGER(third) == 1 || TOY_AS_INTEGER(third) == -1) {
				for (int i = 0; i < TOY_AS_INTEGER(first); i++) {
					Toy_Literal idx = TOY_TO_INTEGER_LITERAL(i);
					Toy_Literal tmp = Toy_getLiteralArray(TOY_AS_ARRAY(compound), idx);
					Toy_pushLiteralArray(result, tmp);

					Toy_freeLiteral(idx);
					Toy_freeLiteral(tmp);
				}

				int min = TOY_AS_INTEGER(third) > 0 ? 0 : TOY_AS_ARRAY(assign)->count - 1;

				if (TOY_IS_ARRAY(assign)) { //push elements of an assigned array
					for (int i = min; i >= 0 && i < TOY_AS_ARRAY(assign)->count; i += TOY_AS_INTEGER(third)) {
						Toy_Literal idx = TOY_TO_INTEGER_LITERAL(i);
						Toy_Literal tmp = Toy_getLiteralArray(TOY_AS_ARRAY(assign), idx); //backwards

						//set result
						Toy_pushLiteralArray(result, tmp);

						Toy_freeLiteral(idx);
						Toy_freeLiteral(tmp);
					}
				}
				else { //push just one element into the array
					Toy_pushLiteralArray(result, assign);
				}

				for (int i = TOY_AS_INTEGER(second) + 1; i < TOY_AS_ARRAY(compound)->count; i++) {
					Toy_Literal idx = TOY_TO_INTEGER_LITERAL(i);
					Toy_Literal tmp = Toy_getLiteralArray(TOY_AS_ARRAY(compound), idx);
					Toy_pushLiteralArray(result, tmp);

					Toy_freeLiteral(idx);
					Toy_freeLiteral(tmp);
				}
			}

			//else override elements of the array instead
			else {
				//copy compound to result
				for (int i = 0; i < TOY_AS_ARRAY(compound)->count; i++) {
					Toy_Literal idx = TOY_TO_INTEGER_LITERAL(i);
					Toy_Literal tmp = Toy_getLiteralArray(TOY_AS_ARRAY(compound), idx);

					Toy_pushLiteralArray(result, tmp);

					Toy_freeLiteral(idx);
					Toy_freeLiteral(tmp);
				}

				int min = TOY_AS_INTEGER(third) > 0 ? 0 : TOY_AS_ARRAY(compound)->count - 1;

				int assignIndex = 0;
				for (int i = min; i >= 0 && i < TOY_AS_ARRAY(compound)->count && assignIndex < TOY_AS_ARRAY(assign)->count; i += TOY_AS_INTEGER(third)) {
					Toy_Literal idx = TOY_TO_INTEGER_LITERAL(i);
					Toy_Literal ai = TOY_TO_INTEGER_LITERAL(assignIndex++);
					Toy_Literal tmp = Toy_getLiteralArray(TOY_AS_ARRAY(assign), ai);

					Toy_setLiteralArray(result, idx, tmp);

					Toy_freeLiteral(idx);
					Toy_freeLiteral(ai);
					Toy_freeLiteral(tmp);
				}
			}

			//finally, swap out the compound for the result
			Toy_freeLiteral(compound);
			compound = TOY_TO_ARRAY_LITERAL(result);
		}

		if (TOY_IS_IDENTIFIER(first)) {
			Toy_Literal idn = first;
			Toy_parseIdentifierToValue(interpreter, &first);
			Toy_freeLiteral(idn);
		}

		value = Toy_getLiteralArray(TOY_AS_ARRAY(compound), first);

		if (TOY_IS_STRING(op) && Toy_equalsRefStringCString(TOY_AS_STRING(op), "+=")) {
			Toy_Literal lit = addition(interpreter, value, assign);
			Toy_setLiteralArray(TOY_AS_ARRAY(compound), first, lit);
			Toy_freeLiteral(lit);
		}

		if (TOY_IS_STRING(op) && Toy_equalsRefStringCString(TOY_AS_STRING(op), "-=")) {
			Toy_Literal lit = subtraction(interpreter, value, assign);
			Toy_setLiteralArray(TOY_AS_ARRAY(compound), first, lit);
			Toy_freeLiteral(lit);
		}

		if (TOY_IS_STRING(op) && Toy_equalsRefStringCString(TOY_AS_STRING(op), "*=")) {
			Toy_Literal lit = multiplication(interpreter, value, assign);
			Toy_setLiteralArray(TOY_AS_ARRAY(compound), first, lit);
			Toy_freeLiteral(lit);
		}

		if (TOY_IS_STRING(op) && Toy_equalsRefStringCString(TOY_AS_STRING(op), "/=")) {
			Toy_Literal lit = division(interpreter, value, assign);
			Toy_setLiteralArray(TOY_AS_ARRAY(compound), first, lit);
			Toy_freeLiteral(lit);
		}

		if (TOY_IS_STRING(op) && Toy_equalsRefStringCString(TOY_AS_STRING(op), "%=")) {
			Toy_Literal lit = modulo(interpreter, value, assign);
			Toy_setLiteralArray(TOY_AS_ARRAY(compound), first, lit);
			Toy_freeLiteral(lit);
		}
	}

	//string - slicing
	if (TOY_IS_STRING(compound)) {
		//string slice
		if (TOY_IS_NULL(op)) {
			//parse out the blanks & their defaults
			if (!TOY_IS_NULL(first)) {
				if (TOY_IS_INDEX_BLANK(first)) {
					Toy_freeLiteral(first);
					first = TOY_TO_INTEGER_LITERAL(0);
				}

				if (TOY_IS_IDENTIFIER(first)) {
					Toy_Literal idn = first;
					Toy_parseIdentifierToValue(interpreter, &first);
					Toy_freeLiteral(idn);
				}
			}

			int compoundLength = TOY_AS_STRING(compound)->length;
			if (!TOY_IS_NULL(second)) {
				if (TOY_IS_INDEX_BLANK(second)) {
					Toy_freeLiteral(second);
					second = TOY_TO_INTEGER_LITERAL(compoundLength);
				}

				if (TOY_IS_IDENTIFIER(second)) {
					Toy_Literal idn = second;
					Toy_parseIdentifierToValue(interpreter, &second);
					Toy_freeLiteral(idn);
				}
			}

			if (TOY_IS_NULL(third) || TOY_IS_INDEX_BLANK(third)) {
				Toy_freeLiteral(third);
				third = TOY_TO_INTEGER_LITERAL(1);
			}

			if (TOY_IS_IDENTIFIER(third)) {
				Toy_Literal idn = third;
				Toy_parseIdentifierToValue(interpreter, &third);
				Toy_freeLiteral(idn);
			}

			//handle each null case
			if (TOY_IS_NULL(first) || !TOY_IS_INTEGER(first)) {
				//something is weird - skip out
				Toy_freeLiteral(op);
				Toy_freeLiteral(assign);
				Toy_freeLiteral(third);
				Toy_freeLiteral(second);
				Toy_freeLiteral(first);
				Toy_freeLiteral(compound);
				Toy_freeLiteral(value);

				return -1;
			}

			if (TOY_IS_NULL(second)) { //assign only a single character
				char c = Toy_toCString(TOY_AS_STRING(compound))[TOY_AS_INTEGER(first)];

				char buffer[16];
				snprintf(buffer, 16, "%c", c);

				Toy_freeLiteral(value);
				int totalLength = strlen(buffer);
				value = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(buffer, totalLength));

				Toy_pushLiteralArray(&interpreter->stack, value);

				Toy_freeLiteral(op);
				Toy_freeLiteral(assign);
				Toy_freeLiteral(third);
				Toy_freeLiteral(second);
				Toy_freeLiteral(first);
				Toy_freeLiteral(compound);
				Toy_freeLiteral(value);

				return 1;
			}

			if (!TOY_IS_INTEGER(second) || (!TOY_IS_NULL(third) && !TOY_IS_INTEGER(third)) || TOY_AS_INTEGER(second) < 0 || TOY_AS_INTEGER(second) > compoundLength || TOY_AS_INTEGER(third) == 0) {
				//something is weird - skip out
				Toy_freeLiteral(op);
				Toy_freeLiteral(assign);
				Toy_freeLiteral(third);
				Toy_freeLiteral(second);
				Toy_freeLiteral(first);
				Toy_freeLiteral(compound);
				Toy_freeLiteral(value);

				return -1;
			}

			//start building a new string from the old one
			char* result = TOY_ALLOCATE(char, TOY_MAX_STRING_LENGTH);

			int lower = TOY_AS_INTEGER(third) > 0 ? TOY_AS_INTEGER(first) : TOY_AS_INTEGER(first) -1;
			int min = TOY_AS_INTEGER(third) > 0 ? TOY_AS_INTEGER(first) : TOY_AS_INTEGER(second) -1;
			int max = TOY_AS_INTEGER(third) > 0 ? TOY_AS_INTEGER(second) + (TOY_AS_INTEGER(second) == compoundLength ? -1 : 0) : TOY_AS_INTEGER(second);

			//copy compound into result
			int resultIndex = 0;
			for (int i = min; i >= 0 && i >= lower && i <= max; i += TOY_AS_INTEGER(third)) {
				result[ resultIndex++ ] = Toy_toCString(TOY_AS_STRING(compound))[ i ];
			}

			result[ resultIndex ] = '\0';

			//finally, swap out the compound for the result
			Toy_freeLiteral(compound);
			compound = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(result, resultIndex));

			TOY_FREE_ARRAY(char, result, TOY_MAX_STRING_LENGTH);
		}

		//string slice assignment
		else if (TOY_IS_STRING(op) && Toy_equalsRefStringCString(TOY_AS_STRING(op), "=")) {
			//parse out the blanks & their defaults
			if (!TOY_IS_NULL(first)) {
				if (TOY_IS_INDEX_BLANK(first)) {
					Toy_freeLiteral(first);
					first = TOY_TO_INTEGER_LITERAL(0);
				}

				if (TOY_IS_IDENTIFIER(first)) {
					Toy_Literal idn = first;
					Toy_parseIdentifierToValue(interpreter, &first);
					Toy_freeLiteral(idn);
				}
			}

			int compoundLength = TOY_AS_STRING(compound)->length;
			if (!TOY_IS_NULL(second)) {
				if (TOY_IS_INDEX_BLANK(second)) {
					Toy_freeLiteral(second);
					second = TOY_TO_INTEGER_LITERAL(compoundLength);
				}

				if (TOY_IS_IDENTIFIER(second)) {
					Toy_Literal idn = second;
					Toy_parseIdentifierToValue(interpreter, &second);
					Toy_freeLiteral(idn);
				}
			}

			if (TOY_IS_NULL(third) || TOY_IS_INDEX_BLANK(third)) {
				Toy_freeLiteral(third);
				third = TOY_TO_INTEGER_LITERAL(1);
			}

			if (TOY_IS_IDENTIFIER(first)) {
				Toy_Literal idn = first;
				Toy_parseIdentifierToValue(interpreter, &first);
				Toy_freeLiteral(idn);
			}

			//handle each null case
			if (TOY_IS_NULL(first) || !TOY_IS_INTEGER(first)) {
				//something is weird - skip out
				Toy_freeLiteral(op);
				Toy_freeLiteral(assign);
				Toy_freeLiteral(third);
				Toy_freeLiteral(second);
				Toy_freeLiteral(first);
				Toy_freeLiteral(compound);
				Toy_freeLiteral(value);

				return -1;
			}

			if (TOY_IS_NULL(second)) { //assign only a single character
				//set the "first" within the array, then skip out
				if (TOY_AS_STRING(assign)->length != 1) {
					//something is weird - skip out
					Toy_freeLiteral(op);
					Toy_freeLiteral(assign);
					Toy_freeLiteral(third);
					Toy_freeLiteral(second);
					Toy_freeLiteral(first);
					Toy_freeLiteral(compound);
					Toy_freeLiteral(value);

					return -1;
				}

				Toy_Literal copiedCompound = TOY_TO_STRING_LITERAL(Toy_deepCopyRefString(TOY_AS_STRING(compound)));

				TOY_AS_STRING(copiedCompound)->data[TOY_AS_INTEGER(first)] = Toy_toCString(TOY_AS_STRING(assign))[0];

				Toy_pushLiteralArray(&interpreter->stack, copiedCompound);

				Toy_freeLiteral(op);
				Toy_freeLiteral(assign);
				Toy_freeLiteral(third);
				Toy_freeLiteral(second);
				Toy_freeLiteral(first);
				Toy_freeLiteral(compound);
				Toy_freeLiteral(value);

				return 1;
			}

			if (!TOY_IS_INTEGER(second) || (!TOY_IS_NULL(third) && !TOY_IS_INTEGER(third)) || TOY_AS_INTEGER(second) < 0 || TOY_AS_INTEGER(second) > compoundLength || TOY_AS_INTEGER(third) == 0) {
				//something is weird - skip out
				Toy_freeLiteral(op);
				Toy_freeLiteral(assign);
				Toy_freeLiteral(third);
				Toy_freeLiteral(second);
				Toy_freeLiteral(first);
				Toy_freeLiteral(compound);
				Toy_freeLiteral(value);

				return -1;
			}

			//start building a new string from the old one
			char* result = TOY_ALLOCATE(char, TOY_MAX_STRING_LENGTH);

			//if third is abs(1), simply insert into the correct positions
			int resultIndex = 0;
			if (TOY_AS_INTEGER(third) == 1 || TOY_AS_INTEGER(third) == -1) {
				for (int i = 0; i < TOY_AS_INTEGER(first); i++) {
					result[ resultIndex++ ] = Toy_toCString(TOY_AS_STRING(compound))[ i ];
				}

				int assignLength = TOY_AS_STRING(assign)->length;
				int min = TOY_AS_INTEGER(third) > 0 ? 0 : assignLength - 1;

				for (int i = min; i >= 0 && i < assignLength; i += TOY_AS_INTEGER(third)) {
					result[ resultIndex++ ] = Toy_toCString(TOY_AS_STRING(assign))[ i ];
				}

				for (int i = TOY_AS_INTEGER(second) + 1; i < compoundLength; i++) {
					result[ resultIndex++ ] = Toy_toCString(TOY_AS_STRING(compound))[ i ];
				}

				result[ resultIndex ] = '\0';
			}

			//else override elements of the array instead
			else {
				//copy compound to result
				snprintf(result, TOY_MAX_STRING_LENGTH, "%s", Toy_toCString(TOY_AS_STRING(compound)));

				int assignLength = TOY_AS_STRING(assign)->length;
				int min = TOY_AS_INTEGER(third) > 0 ? TOY_AS_INTEGER(first) : TOY_AS_INTEGER(second) - 1;

				int assignIndex = 0;
				for (int i = min; i >= TOY_AS_INTEGER(first) && i <= TOY_AS_INTEGER(second) && assignIndex < assignLength; i += TOY_AS_INTEGER(third)) {
					result[ i ] = Toy_toCString(TOY_AS_STRING(assign))[ assignIndex++ ];
				}
				resultIndex = strlen(result);
			}

			//finally, swap out the compound for the result
			Toy_freeLiteral(compound);
			compound = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(result, resultIndex));

			TOY_FREE_ARRAY(char, result, TOY_MAX_STRING_LENGTH);
		}

		else if (TOY_IS_STRING(op) && Toy_equalsRefStringCString(TOY_AS_STRING(op), "+=")) {
			Toy_Literal tmp = addition(interpreter, compound, assign);
			Toy_freeLiteral(compound);
			compound = tmp; //don't clear tmp
		}
	}

	//leave the compound on the stack
	Toy_pushLiteralArray(&interpreter->stack, compound);

	Toy_freeLiteral(op);
	Toy_freeLiteral(assign);
	Toy_freeLiteral(third);
	Toy_freeLiteral(second);
	Toy_freeLiteral(first);
	Toy_freeLiteral(compound);
	Toy_freeLiteral(value);

	return 1;
}

int _set(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 3) {
		interpreter->errorOutput("Incorrect number of arguments to _set\n");
		return -1;
	}

	Toy_Literal idn = arguments->literals[0];
	Toy_Literal obj = arguments->literals[0];
	Toy_Literal key = arguments->literals[1];
	Toy_Literal val = arguments->literals[2];

	if (!TOY_IS_IDENTIFIER(idn)) {
		interpreter->errorOutput("Expected identifier in _set\n");
		return -1;
	}

	Toy_parseIdentifierToValue(interpreter, &obj);

	bool freeKey = false;
	if (TOY_IS_IDENTIFIER(key)) {
		Toy_parseIdentifierToValue(interpreter, &key);
		freeKey = true;
	}

	bool freeVal = false;
	if (TOY_IS_IDENTIFIER(val)) {
		Toy_parseIdentifierToValue(interpreter, &val);
		freeVal = true;
	}

	switch(obj.type) {
		case TOY_LITERAL_ARRAY: {
			Toy_Literal typeLiteral = Toy_getScopeType(interpreter->scope, key);

			if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_ARRAY) {
				Toy_Literal subtypeLiteral = ((Toy_Literal*)(TOY_AS_TYPE(typeLiteral).subtypes))[0];

				if (TOY_AS_TYPE(subtypeLiteral).typeOf != TOY_LITERAL_ANY && TOY_AS_TYPE(subtypeLiteral).typeOf != val.type) {
					interpreter->errorOutput("Bad argument type in _set\n");
					return -1;
				}
			}

			if (!TOY_IS_INTEGER(key)) {
				interpreter->errorOutput("Expected integer index in _set\n");
				return -1;
			}

			if (TOY_AS_ARRAY(obj)->count <= TOY_AS_INTEGER(key) || TOY_AS_INTEGER(key) < 0) {
				interpreter->errorOutput("Index out of bounds in _set\n");
				return -1;
			}

			//don't use pushLiteralArray, since we're setting
			Toy_freeLiteral(TOY_AS_ARRAY(obj)->literals[TOY_AS_INTEGER(key)]); //BUGFIX: clear any existing data first
			TOY_AS_ARRAY(obj)->literals[TOY_AS_INTEGER(key)] = Toy_copyLiteral(val);

			if (!Toy_setScopeVariable(interpreter->scope, idn, obj, true)) {
				interpreter->errorOutput("Incorrect type assigned to array in _set: \"");
				Toy_printLiteralCustom(val, interpreter->errorOutput);
				interpreter->errorOutput("\"\n");
				return -1;
			}

			break;
		}

		case TOY_LITERAL_DICTIONARY: {
			Toy_Literal typeLiteral = Toy_getScopeType(interpreter->scope, key);

			if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_DICTIONARY) {
				Toy_Literal keySubtypeLiteral = ((Toy_Literal*)(TOY_AS_TYPE(typeLiteral).subtypes))[0];
				Toy_Literal valSubtypeLiteral = ((Toy_Literal*)(TOY_AS_TYPE(typeLiteral).subtypes))[1];

				if (TOY_AS_TYPE(keySubtypeLiteral).typeOf != TOY_LITERAL_ANY && TOY_AS_TYPE(keySubtypeLiteral).typeOf != key.type) {
					interpreter->printOutput("bad argument type in _set\n");
					return -1;
				}

				if (TOY_AS_TYPE(valSubtypeLiteral).typeOf != TOY_LITERAL_ANY && TOY_AS_TYPE(valSubtypeLiteral).typeOf != val.type) {
					interpreter->printOutput("bad argument type in _set\n");
					return -1;
				}
			}

			Toy_setLiteralDictionary(TOY_AS_DICTIONARY(obj), key, val);

			if (!Toy_setScopeVariable(interpreter->scope, idn, obj, true)) {
				interpreter->errorOutput("Incorrect type assigned to dictionary in _set: \"");
				Toy_printLiteralCustom(val, interpreter->errorOutput);
				interpreter->errorOutput("\"\n");
				return -1;
			}

			break;
		}

		default:
			interpreter->errorOutput("Incorrect compound type in _set: ");
			Toy_printLiteralCustom(obj, interpreter->errorOutput);
			interpreter->errorOutput("\"\n");
			return -1;
	}

	Toy_freeLiteral(obj);

	if (freeKey) {
		Toy_freeLiteral(key);
	}

	if (freeVal) {
		Toy_freeLiteral(val);
	}

	return 0;
}

int _get(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to _get");
		return -1;
	}

	Toy_Literal obj = arguments->literals[0];
	Toy_Literal key = arguments->literals[1];

	bool freeObj = false;
	if (TOY_IS_IDENTIFIER(obj)) {
		Toy_parseIdentifierToValue(interpreter, &obj);
		freeObj = true;
	}

	bool freeKey = false;
	if (TOY_IS_IDENTIFIER(key)) {
		Toy_parseIdentifierToValue(interpreter, &key);
		freeKey = true;
	}

	switch(obj.type) {
		case TOY_LITERAL_ARRAY: {
			if (!TOY_IS_INTEGER(key)) {
				interpreter->errorOutput("Expected integer index in _get\n");
				return -1;
			}

			if (TOY_AS_ARRAY(obj)->count <= TOY_AS_INTEGER(key) || TOY_AS_INTEGER(key) < 0) {
				interpreter->errorOutput("Index out of bounds in _get\n");
				return -1;
			}

			Toy_pushLiteralArray(&interpreter->stack, TOY_AS_ARRAY(obj)->literals[TOY_AS_INTEGER(key)]);

			if (freeObj) {
				Toy_freeLiteral(obj);
			}

			if (freeKey) {
				Toy_freeLiteral(key);
			}

			return 1;
		}

		case TOY_LITERAL_DICTIONARY: {
			Toy_Literal dict = Toy_getLiteralDictionary(TOY_AS_DICTIONARY(obj), key);
			Toy_pushLiteralArray(&interpreter->stack, dict);
			Toy_freeLiteral(dict);

			if (freeObj) {
				Toy_freeLiteral(obj);
			}

			if (freeKey) {
				Toy_freeLiteral(key);
			}

			return 1;
		}

		default:
			interpreter->errorOutput("Incorrect compound type in _get \"");
			Toy_printLiteralCustom(obj, interpreter->errorOutput);
			interpreter->errorOutput("\"\n");
			return -1;
	}
}

int _push(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 2) {
		interpreter->errorOutput("Incorrect number of arguments to _push\n");
		return -1;
	}

	Toy_Literal idn = arguments->literals[0];
	Toy_Literal obj = arguments->literals[0];
	Toy_Literal val = arguments->literals[1];

	if (!TOY_IS_IDENTIFIER(idn)) {
		interpreter->errorOutput("Expected identifier in _push\n");
		return -1;
	}

	Toy_parseIdentifierToValue(interpreter, &obj);

	bool freeVal = false;
	if (TOY_IS_IDENTIFIER(val)) {
		Toy_parseIdentifierToValue(interpreter, &val);
		freeVal = true;
	}

	switch(obj.type) {
		case TOY_LITERAL_ARRAY: {
			Toy_Literal typeLiteral = Toy_getScopeType(interpreter->scope, val);

			if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_ARRAY) {
				Toy_Literal subtypeLiteral = ((Toy_Literal*)(TOY_AS_TYPE(typeLiteral).subtypes))[0];

				if (TOY_AS_TYPE(subtypeLiteral).typeOf != TOY_LITERAL_ANY && TOY_AS_TYPE(subtypeLiteral).typeOf != val.type) {
					interpreter->errorOutput("Bad argument type in _push");
					return -1;
				}
			}

			Toy_pushLiteralArray(TOY_AS_ARRAY(obj), val);

			if (!Toy_setScopeVariable(interpreter->scope, idn, obj, true)) { //TODO: could definitely be more efficient than overwriting the whole original object
				interpreter->errorOutput("Incorrect type assigned to array in _push: \"");
				Toy_printLiteralCustom(val, interpreter->errorOutput);
				interpreter->errorOutput("\"\n");
				return -1;
			}

			Toy_freeLiteral(obj);

			if (freeVal) {
				Toy_freeLiteral(val);
			}

			return 0;
		}

		default:
			interpreter->errorOutput("Incorrect compound type in _push: ");
			Toy_printLiteralCustom(obj, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			return -1;
	}
}

int _pop(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _pop\n");
		return -1;
	}

	Toy_Literal idn = arguments->literals[0];
	Toy_Literal obj = arguments->literals[0];

	if (!TOY_IS_IDENTIFIER(idn)) {
		interpreter->errorOutput("Expected identifier in _pop\n");
		return -1;
	}

	Toy_parseIdentifierToValue(interpreter, &obj);

	switch(obj.type) {
		case TOY_LITERAL_ARRAY: {
			Toy_Literal lit = Toy_popLiteralArray(TOY_AS_ARRAY(obj));
			Toy_pushLiteralArray(&interpreter->stack, lit);
			Toy_freeLiteral(lit);

			if (!Toy_setScopeVariable(interpreter->scope, idn, obj, true)) { //TODO: could definitely be more efficient than overwriting the whole original object
				interpreter->errorOutput("Incorrect type assigned to array in _pop: ");
				Toy_printLiteralCustom(obj, interpreter->errorOutput);
				interpreter->errorOutput("\n");
				return -1;
			}

			Toy_freeLiteral(obj);

			return 1;
		}

		default:
			interpreter->errorOutput("Incorrect compound type in _pop: ");
			Toy_printLiteralCustom(obj, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			return -1;
	}
}

int _length(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _length\n");
		return -1;
	}

	Toy_Literal obj = arguments->literals[0];

	bool freeObj = false;
	if (TOY_IS_IDENTIFIER(obj)) {
		Toy_parseIdentifierToValue(interpreter, &obj);
		freeObj = true;
	}

	switch(obj.type) {
		case TOY_LITERAL_ARRAY: {
			Toy_Literal lit = TOY_TO_INTEGER_LITERAL( TOY_AS_ARRAY(obj)->count );
			Toy_pushLiteralArray(&interpreter->stack, lit);
			Toy_freeLiteral(lit);
			break;
		}

		case TOY_LITERAL_DICTIONARY: {
			Toy_Literal lit = TOY_TO_INTEGER_LITERAL( TOY_AS_DICTIONARY(obj)->count );
			Toy_pushLiteralArray(&interpreter->stack, lit);
			Toy_freeLiteral(lit);
			break;
		}

		case TOY_LITERAL_STRING: {
			Toy_Literal lit = TOY_TO_INTEGER_LITERAL( TOY_AS_STRING(obj)->length );
			Toy_pushLiteralArray(&interpreter->stack, lit);
			Toy_freeLiteral(lit);
			break;
		}

		default:
			interpreter->errorOutput("Incorrect compound type in _length: ");
			Toy_printLiteralCustom(obj, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			return -1;
	}

	if (freeObj) {
		Toy_freeLiteral(obj);
	}

	return 1;
}

int _clear(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//if wrong number of arguments, fail
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to _clear\n");
		return -1;
	}

	Toy_Literal idn = arguments->literals[0];
	Toy_Literal obj = arguments->literals[0];

	if (!TOY_IS_IDENTIFIER(idn)) {
		interpreter->errorOutput("expected identifier in _clear\n");
		return -1;
	}

	Toy_parseIdentifierToValue(interpreter, &obj);

	//NOTE: just pass in new compounds

	switch(obj.type) {
		case TOY_LITERAL_ARRAY: {
			Toy_LiteralArray* array = TOY_ALLOCATE(Toy_LiteralArray, 1);
			Toy_initLiteralArray(array);

			Toy_Literal obj = TOY_TO_ARRAY_LITERAL(array);

			if (!Toy_setScopeVariable(interpreter->scope, idn, obj, true)) {
				interpreter->errorOutput("Incorrect type assigned to array in _clear: ");
				Toy_printLiteralCustom(obj, interpreter->errorOutput);
				interpreter->errorOutput("\n");
				return -1;
			}

			Toy_freeLiteral(obj);

			break;
		}

		case TOY_LITERAL_DICTIONARY: {
			Toy_LiteralDictionary* dictionary = TOY_ALLOCATE(Toy_LiteralDictionary, 1);
			Toy_initLiteralDictionary(dictionary);

			Toy_Literal obj = TOY_TO_DICTIONARY_LITERAL(dictionary);

			if (!Toy_setScopeVariable(interpreter->scope, idn, obj, true)) {
				interpreter->errorOutput("Incorrect type assigned to dictionary in _clear: ");
				Toy_printLiteralCustom(obj, interpreter->errorOutput);
				interpreter->errorOutput("\n");
				return -1;
			}

			Toy_freeLiteral(obj);

			break;
		}

		default:
			interpreter->errorOutput("Incorrect compound type in _clear: ");
			Toy_printLiteralCustom(obj, interpreter->errorOutput);
			interpreter->errorOutput("\n");
			return -1;
	}

	Toy_freeLiteral(obj);
	return 1;
}
