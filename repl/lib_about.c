#include "lib_about.h"

#include "toy_memory.h"

int Toy_hookAbout(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias) {
	//the about keys
	Toy_Literal majorKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("major"));
	Toy_Literal minorKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("minor"));
	Toy_Literal patchKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("patch"));
	Toy_Literal buildKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("build"));
	Toy_Literal authorKeyLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("author"));

	//the about identifiers
	Toy_Literal majorIdentifierLiteral = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString("major"));
	Toy_Literal minorIdentifierLiteral = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString("minor"));
	Toy_Literal patchIdentifierLiteral = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString("patch"));
	Toy_Literal buildIdentifierLiteral = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString("build"));
	Toy_Literal authorIdentifierLiteral = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString("author"));

	//the about values
	Toy_Literal majorLiteral = TOY_TO_INTEGER_LITERAL(TOY_VERSION_MAJOR);
	Toy_Literal minorLiteral = TOY_TO_INTEGER_LITERAL(TOY_VERSION_MINOR);
	Toy_Literal patchLiteral = TOY_TO_INTEGER_LITERAL(TOY_VERSION_PATCH);
	Toy_Literal buildLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString(TOY_VERSION_BUILD));
	Toy_Literal authorLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("Kayne Ruse, KR Game Studios"));

	//store as an aliased dictionary
	if (!TOY_IS_NULL(alias)) {
		//make sure the name isn't taken
		if (Toy_isDeclaredScopeVariable(interpreter->scope, alias)) {
			interpreter->errorOutput("Can't override an existing variable\n");
			Toy_freeLiteral(alias);

			Toy_freeLiteral(majorKeyLiteral);
			Toy_freeLiteral(minorKeyLiteral);
			Toy_freeLiteral(patchKeyLiteral);
			Toy_freeLiteral(buildKeyLiteral);
			Toy_freeLiteral(authorKeyLiteral);

			Toy_freeLiteral(majorIdentifierLiteral);
			Toy_freeLiteral(minorIdentifierLiteral);
			Toy_freeLiteral(patchIdentifierLiteral);
			Toy_freeLiteral(buildIdentifierLiteral);
			Toy_freeLiteral(authorIdentifierLiteral);

			Toy_freeLiteral(majorLiteral);
			Toy_freeLiteral(minorLiteral);
			Toy_freeLiteral(patchLiteral);
			Toy_freeLiteral(buildLiteral);
			Toy_freeLiteral(authorLiteral);

			return -1;
		}

		//create the dictionary to load up with values
		Toy_LiteralDictionary* dictionary = TOY_ALLOCATE(Toy_LiteralDictionary, 1);
		Toy_initLiteralDictionary(dictionary);

		//set each key/value pair
		Toy_setLiteralDictionary(dictionary, majorKeyLiteral, majorLiteral);
		Toy_setLiteralDictionary(dictionary, minorKeyLiteral, minorLiteral);
		Toy_setLiteralDictionary(dictionary, patchKeyLiteral, patchLiteral);
		Toy_setLiteralDictionary(dictionary, buildKeyLiteral, buildLiteral);
		Toy_setLiteralDictionary(dictionary, authorKeyLiteral, authorLiteral);

		//build the type
		Toy_Literal type = TOY_TO_TYPE_LITERAL(TOY_LITERAL_DICTIONARY, true);
		Toy_Literal strType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_STRING, true);
		Toy_Literal anyType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_ANY, true);
		TOY_TYPE_PUSH_SUBTYPE(&type, strType);
		TOY_TYPE_PUSH_SUBTYPE(&type, anyType);

		//set scope
		Toy_Literal dict = TOY_TO_DICTIONARY_LITERAL(dictionary);
		Toy_declareScopeVariable(interpreter->scope, alias, type);
		Toy_setScopeVariable(interpreter->scope, alias, dict, false);

		//cleanup
		Toy_freeLiteral(dict);
		Toy_freeLiteral(type);
	}

	//store globally
	else {
		//make sure the names aren't taken
		if (Toy_isDeclaredScopeVariable(interpreter->scope, majorKeyLiteral) ||
			Toy_isDeclaredScopeVariable(interpreter->scope, minorKeyLiteral) ||
			Toy_isDeclaredScopeVariable(interpreter->scope, patchKeyLiteral) ||
			Toy_isDeclaredScopeVariable(interpreter->scope, buildKeyLiteral) ||
			Toy_isDeclaredScopeVariable(interpreter->scope, authorKeyLiteral)) {
			interpreter->errorOutput("Can't override an existing variable\n");
			Toy_freeLiteral(alias);

			Toy_freeLiteral(majorKeyLiteral);
			Toy_freeLiteral(minorKeyLiteral);
			Toy_freeLiteral(patchKeyLiteral);
			Toy_freeLiteral(buildKeyLiteral);
			Toy_freeLiteral(authorKeyLiteral);

			Toy_freeLiteral(majorIdentifierLiteral);
			Toy_freeLiteral(minorIdentifierLiteral);
			Toy_freeLiteral(patchIdentifierLiteral);
			Toy_freeLiteral(buildIdentifierLiteral);
			Toy_freeLiteral(authorIdentifierLiteral);

			Toy_freeLiteral(majorLiteral);
			Toy_freeLiteral(minorLiteral);
			Toy_freeLiteral(patchLiteral);
			Toy_freeLiteral(buildLiteral);
			Toy_freeLiteral(authorLiteral);

			return -1;
		}

		Toy_Literal intType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_INTEGER, true);
		Toy_Literal strType = TOY_TO_TYPE_LITERAL(TOY_LITERAL_STRING, true);

		//major
		Toy_declareScopeVariable(interpreter->scope, majorIdentifierLiteral, intType);
		Toy_setScopeVariable(interpreter->scope, majorIdentifierLiteral, majorLiteral, false);

		//minor
		Toy_declareScopeVariable(interpreter->scope, minorIdentifierLiteral, intType);
		Toy_setScopeVariable(interpreter->scope, minorIdentifierLiteral, minorLiteral, false);

		//patch
		Toy_declareScopeVariable(interpreter->scope, patchIdentifierLiteral, intType);
		Toy_setScopeVariable(interpreter->scope, patchIdentifierLiteral, patchLiteral, false);

		//build
		Toy_declareScopeVariable(interpreter->scope, buildIdentifierLiteral, strType);
		Toy_setScopeVariable(interpreter->scope, buildIdentifierLiteral, buildLiteral, false);

		//author
		Toy_declareScopeVariable(interpreter->scope, authorIdentifierLiteral, strType);
		Toy_setScopeVariable(interpreter->scope, authorIdentifierLiteral, authorLiteral, false);

		Toy_freeLiteral(intType);
		Toy_freeLiteral(strType);
	}

	//cleanup
	Toy_freeLiteral(majorKeyLiteral);
	Toy_freeLiteral(minorKeyLiteral);
	Toy_freeLiteral(patchKeyLiteral);
	Toy_freeLiteral(buildKeyLiteral);
	Toy_freeLiteral(authorKeyLiteral);

	Toy_freeLiteral(majorIdentifierLiteral);
	Toy_freeLiteral(minorIdentifierLiteral);
	Toy_freeLiteral(patchIdentifierLiteral);
	Toy_freeLiteral(buildIdentifierLiteral);
	Toy_freeLiteral(authorIdentifierLiteral);

	Toy_freeLiteral(majorLiteral);
	Toy_freeLiteral(minorLiteral);
	Toy_freeLiteral(patchLiteral);
	Toy_freeLiteral(buildLiteral);
	Toy_freeLiteral(authorLiteral);

	return 0;
}
