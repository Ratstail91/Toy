#include "lib_compound.h"

#include "toy_memory.h"

#include <ctype.h>
#include <stdio.h>

static int nativeToLower(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to clock\n");
		return -1;
	}

	//get the argument to a C-string
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_RefString* selfRefString = TOY_AS_STRING(selfLiteral);
	char* self = Toy_toCString(selfRefString);

	//allocate buffer space for the result
	char* result = TOY_ALLOCATE(char, Toy_lengthRefString(selfRefString) + 1);

	//set each new character
	for (int i = 0; i < Toy_lengthRefString(selfRefString); i++) {
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

static int nativeToUpper(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to clock\n");
		return -1;
	}

	//get the argument to a C-string
	Toy_Literal selfLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal selfLiteralIdn = selfLiteral;
	if (TOY_IS_IDENTIFIER(selfLiteral) && Toy_parseIdentifierToValue(interpreter, &selfLiteral)) {
		Toy_freeLiteral(selfLiteralIdn);
	}

	Toy_RefString* selfRefString = TOY_AS_STRING(selfLiteral);
	char* self = Toy_toCString(selfRefString);

	//allocate buffer space for the result
	char* result = TOY_ALLOCATE(char, Toy_lengthRefString(selfRefString) + 1);

	//set each new character
	for (int i = 0; i < Toy_lengthRefString(selfRefString); i++) {
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
		interpreter->errorOutput("Incorrect number of arguments to clock\n");
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

	//unwrap the arguments
	Toy_RefString* trimCharsRefString = TOY_AS_STRING(trimCharsLiteral);
	Toy_RefString* selfRefString = TOY_AS_STRING(selfLiteral);

	//allocate space for the new string
	int bufferBegin = 0;
	int bufferEnd = Toy_lengthRefString(selfRefString);

	//for each character in self, check it against each character in trimChars - on a fail, go to end
	for (int i = 0; i < Toy_lengthRefString(selfRefString); i++) {
		int trimIndex = 0;

		while (trimIndex < Toy_lengthRefString(trimCharsRefString)) {
			//there is a match - DON'T increment anymore
			if (Toy_toCString(selfRefString)[i] == Toy_toCString(trimCharsRefString)[trimIndex]) {
				break;
			}

			trimIndex++;
		}

		//if the match is found, increment buffer begin
		if (trimIndex < Toy_lengthRefString(trimCharsRefString)) {
			bufferBegin++;
			continue;
		}
		else {
			break;
		}
	}

	//again, from the back
	for (int i = Toy_lengthRefString(selfRefString); i >= 0; i--) {
		int trimIndex = 0;

		while (trimIndex < Toy_lengthRefString(trimCharsRefString)) {
			//there is a match - DON'T increment anymore
			if (Toy_toCString(selfRefString)[i-1] == Toy_toCString(trimCharsRefString)[trimIndex]) {
				break;
			}

			trimIndex++;
		}

		//if the match is found, increment buffer begin
		if (trimIndex < Toy_lengthRefString(trimCharsRefString)) {
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

int Toy_hookCompound(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias) {
	//build the natives list
	Natives natives[] = {
		// {"_concat", native}, //array, dictionary, string
		// {"_containsKey", native}, //dictionary
		// {"_containsValue", native}, //array, dictionary
		// {"_every", native}, //array, dictionary, string
		// {"_filter", native}, //array, dictionary
		// {"_indexOf", native}, //array, string
		// {"_insert", native}, //array, dictionary, string
		// {"_keys", native}, //dictionary
		// {"_map", native}, //array, dictionary
		// {"_reduce", native}, //array, dictionary
		// {"_remove", native}, //array, dictionary
		// {"_replace", native}, //string
		// {"_some", native}, //array, dictionary, string
		// {"_sort", native}, //array
		{"_toLower", nativeToLower}, //string
		// {"_toString", native}, //array, dictionary
		{"_toUpper", nativeToUpper}, //string
		{"_trim", nativeTrim}, //string
		// {"_values", native}, //dictionary
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
