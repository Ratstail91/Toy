#include "lib_random.h"

#include "toy_memory.h"

static int hashInt(int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

typedef struct Toy_RandomGenerator {
	int seed; //mutated with each call
} Toy_RandomGenerator;

//Toy native functions
static int nativeCreateRandomGenerator(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to createRandomGenerator\n");
		return -1;
	}

	//get the seed argument
	Toy_Literal seedLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal seedLiteralIdn = seedLiteral;
	if (TOY_IS_IDENTIFIER(seedLiteral) && Toy_parseIdentifierToValue(interpreter, &seedLiteral)) {
		Toy_freeLiteral(seedLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(seedLiteral)) {
		Toy_freeLiteral(seedLiteral);
		return -1;
	}

	if (!TOY_IS_INTEGER(seedLiteral)) {
		interpreter->errorOutput("Incorrect literal type passed to createRandomGenerator");
		Toy_freeLiteral(seedLiteral);
		return -1;
	}

	//generate the generator object
	Toy_RandomGenerator* generator = TOY_ALLOCATE(Toy_RandomGenerator, 1);
	generator->seed = TOY_AS_INTEGER(seedLiteral);
	Toy_Literal generatorLiteral = TOY_TO_OPAQUE_LITERAL(generator, TOY_OPAQUE_TAG_RANDOM);

	//return and cleanup
	Toy_pushLiteralArray(&interpreter->stack, generatorLiteral);

	Toy_freeLiteral(seedLiteral);
	Toy_freeLiteral(generatorLiteral);

	return 1;
}

static int nativeGenerateRandomNumber(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to generateRandomNumber\n");
		return -1;
	}

	//get the runner object
	Toy_Literal generatorLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal generatorLiteralIdn = generatorLiteral;
	if (TOY_IS_IDENTIFIER(generatorLiteral) && Toy_parseIdentifierToValue(interpreter, &generatorLiteral)) {
		Toy_freeLiteral(generatorLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(generatorLiteral)) {
		Toy_freeLiteral(generatorLiteral);
		return -1;
	}

	if (TOY_GET_OPAQUE_TAG(generatorLiteral) != TOY_OPAQUE_TAG_RANDOM) {
		interpreter->errorOutput("Unrecognized opaque literal in generateRandomNumber\n");
		return -1;
	}

	Toy_RandomGenerator* generator = TOY_AS_OPAQUE(generatorLiteral);

	//generate the new value and package up the return
	generator->seed = hashInt(generator->seed);

	Toy_Literal resultLiteral = TOY_TO_INTEGER_LITERAL(generator->seed);

	Toy_pushLiteralArray(&interpreter->stack, resultLiteral);

	//cleanup
	Toy_freeLiteral(generatorLiteral);
	Toy_freeLiteral(resultLiteral);

	return 0;
}

static int nativeFreeRandomGenerator(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments) {
	//no arguments
	if (arguments->count != 1) {
		interpreter->errorOutput("Incorrect number of arguments to freeRandomGenerator\n");
		return -1;
	}

	//get the runner object
	Toy_Literal generatorLiteral = Toy_popLiteralArray(arguments);

	Toy_Literal generatorLiteralIdn = generatorLiteral;
	if (TOY_IS_IDENTIFIER(generatorLiteral) && Toy_parseIdentifierToValue(interpreter, &generatorLiteral)) {
		Toy_freeLiteral(generatorLiteralIdn);
	}

	if (TOY_IS_IDENTIFIER(generatorLiteral)) {
		Toy_freeLiteral(generatorLiteral);
		return -1;
	}

	if (TOY_GET_OPAQUE_TAG(generatorLiteral) != TOY_OPAQUE_TAG_RANDOM) {
		interpreter->errorOutput("Unrecognized opaque literal in freeRandomGenerator\n");
		return -1;
	}

	Toy_RandomGenerator* generator = TOY_AS_OPAQUE(generatorLiteral);

	//clear out the runner object
	TOY_FREE(Toy_RandomGenerator, generator);
	Toy_freeLiteral(generatorLiteral);

	return 0;
}

//call the hook
typedef struct Natives {
	const char* name;
	Toy_NativeFn fn;
} Natives;

int Toy_hookRandom(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias) {
	//build the natives list
	Natives natives[] = {
		{"createRandomGenerator", nativeCreateRandomGenerator},
		{"generateRandomNumber", nativeGenerateRandomNumber},
		{"freeRandomGenerator", nativeFreeRandomGenerator},
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
