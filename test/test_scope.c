#include "toy_scope.h"

#include "toy_memory.h"
#include "toy_console_colors.h"

#include <stdio.h>

int main() {
	{
		//test init & quit
		Toy_Scope* scope = Toy_pushScope(NULL);
		scope = Toy_popScope(scope);
	}

	{
		//prerequisites
		char* idn_raw = "foobar";

		Toy_Literal identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString(idn_raw));
		Toy_Literal value = TOY_TO_INTEGER_LITERAL(42);
		Toy_Literal type = TOY_TO_TYPE_LITERAL(value.type, false);

		//test declarations & assignments
		Toy_Scope* scope = Toy_pushScope(NULL);

		//declare & assign
		if (!Toy_declareScopeVariable(scope, identifier, type)) {
			printf(TOY_CC_ERROR "Failed to declare scope variable" TOY_CC_RESET);
			return -1;
		}

		if (!Toy_setScopeVariable(scope, identifier, value, true)) {
			printf(TOY_CC_ERROR "Failed to sete scope variable" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		scope = Toy_popScope(scope);

		Toy_freeLiteral(identifier);
		Toy_freeLiteral(value);
		Toy_freeLiteral(type);
	}

	{
		//prerequisites
		char* idn_raw = "foobar";

		Toy_Literal identifier = TOY_TO_IDENTIFIER_LITERAL(Toy_createRefString(idn_raw));
		Toy_Literal type = TOY_TO_TYPE_LITERAL(TOY_LITERAL_INTEGER, false);

		//test declarations & assignments
		Toy_Scope* scope = Toy_pushScope(NULL);

		//declare & assign
		if (!Toy_declareScopeVariable(scope, identifier, type)) {
			printf(TOY_CC_ERROR "Failed to declare the scope variable" TOY_CC_RESET);
			return -1;
		}

		if (!Toy_setScopeVariable(scope, identifier, TOY_TO_INTEGER_LITERAL(42), true)) {
			printf(TOY_CC_ERROR "Failed to set the scope variable" TOY_CC_RESET);
			return -1;
		}

		//deeper scope
		scope = Toy_pushScope(scope);

		//test shadowing
		Toy_Literal ref;
		if (!Toy_getScopeVariable(scope, identifier, &ref)) {
			printf(TOY_CC_ERROR "Failed to get the scope variable" TOY_CC_RESET);
			return -1;
		}

		if (TOY_AS_INTEGER(ref) != 42) {
			printf(TOY_CC_ERROR "Failed to retreive the correct variable value" TOY_CC_RESET);
			return -1;
		}

		if (!Toy_declareScopeVariable(scope, identifier, type)) {
			printf(TOY_CC_ERROR "Failed to declare the scope variable (shadowing)" TOY_CC_RESET);
			return -1;
		}

		if (!Toy_setScopeVariable(scope, identifier, TOY_TO_INTEGER_LITERAL(69), true)) {
			printf(TOY_CC_ERROR "Failed to set the scope variable (shadowing)" TOY_CC_RESET);
			return -1;
		}

		if (!Toy_getScopeVariable(scope, identifier, &ref)) {
			printf(TOY_CC_ERROR "Failed to get the scope variable (shadowing)" TOY_CC_RESET);
			return -1;
		}

		if (TOY_AS_INTEGER(ref) != 69) {
			printf(TOY_CC_ERROR "Failed to retreive the correct variable value (shadowing)" TOY_CC_RESET);
			return -1;
		}

		//unwind
		scope = Toy_popScope(scope);

		if (!Toy_getScopeVariable(scope, identifier, &ref)) {
			printf(TOY_CC_ERROR "Failed to get the scope variable" TOY_CC_RESET);
			return -1;
		}

		if (TOY_AS_INTEGER(ref) != 42) {
			printf(TOY_CC_ERROR "Failed to retreive the correct variable value" TOY_CC_RESET);
			return -1;
		}

		//cleanup
		scope = Toy_popScope(scope);

		Toy_freeLiteral(identifier);
		Toy_freeLiteral(type);
	}

	printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
	return 0;
}

