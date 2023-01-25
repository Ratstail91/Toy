#include "lib_standard.h"

#include "toy_memory.h"

#include <time.h>
#include <sys/time.h>

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
	int len = strlen(timestr) - 1; //-1 for the newline
	Toy_Literal timeLiteral = TOY_TO_STRING_LITERAL(Toy_createRefStringLength(timestr, len));

	//push to the stack
	Toy_pushLiteralArray(&interpreter->stack, timeLiteral);

	//cleanup
	Toy_freeLiteral(timeLiteral);

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
		{"clock", nativeClock},
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
