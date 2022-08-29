#include "scope.h"

#include "memory.h"
#include "console_colors.h"

#include <stdio.h>

int main() {
	{
		//test init & quit
		Scope* scope = pushScope(NULL);
		scope = popScope(scope);
	}

	{
		//prerequisites
		char* idn_raw = "foobar";

		Literal identifier = TO_IDENTIFIER_LITERAL(copyString(idn_raw, strlen(idn_raw)), strlen(idn_raw));
		Literal value = TO_INTEGER_LITERAL(42);
		Literal type = TO_TYPE_LITERAL(value.type, false);

		//test declarations & assignments
		Scope* scope = pushScope(NULL);

		//declare & assign
		if (!declareScopeVariable(scope, identifier, type)) {
			printf(ERROR "Failed to declare scope variable" RESET);
			return -1;
		}

		if (!setScopeVariable(scope, identifier, value, true)) {
			printf(ERROR "Failed to sete scope variable" RESET);
			return -1;
		}

		//cleanup
		scope = popScope(scope);

		freeLiteral(identifier);
		freeLiteral(value);
		freeLiteral(type);
	}

	{
		//prerequisites
		char* idn_raw = "foobar";

		Literal identifier = TO_IDENTIFIER_LITERAL(copyString(idn_raw, strlen(idn_raw)), strlen(idn_raw));
		Literal type = TO_TYPE_LITERAL(LITERAL_INTEGER, false);

		//test declarations & assignments
		Scope* scope = pushScope(NULL);

		//declare & assign
		if (!declareScopeVariable(scope, identifier, type)) {
			printf(ERROR "Failed to declare the scope variable" RESET);
			return -1;
		}

		if (!setScopeVariable(scope, identifier, TO_INTEGER_LITERAL(42), true)) {
			printf(ERROR "Failed to set the scope variable" RESET);
			return -1;
		}

		//deeper scope
		scope = pushScope(scope);

		//test shadowing
		Literal ref;
		if (!getScopeVariable(scope, identifier, &ref)) {
			printf(ERROR "Failed to get the scope variable" RESET);
			return -1;
		}

		if (AS_INTEGER(ref) != 42) {
			printf(ERROR "Failed to retreive the correct variable value" RESET);
			return -1;
		}

		if (!declareScopeVariable(scope, identifier, type)) {
			printf(ERROR "Failed to declare the scope variable (shadowing)" RESET);
			return -1;
		}

		if (!setScopeVariable(scope, identifier, TO_INTEGER_LITERAL(69), true)) {
			printf(ERROR "Failed to set the scope variable (shadowing)" RESET);
			return -1;
		}

		if (!getScopeVariable(scope, identifier, &ref)) {
			printf(ERROR "Failed to get the scope variable (shadowing)" RESET);
			return -1;
		}

		if (AS_INTEGER(ref) != 69) {
			printf(ERROR "Failed to retreive the correct variable value (shadowing)" RESET);
			return -1;
		}

		//unwind
		scope = popScope(scope);

		if (!getScopeVariable(scope, identifier, &ref)) {
			printf(ERROR "Failed to get the scope variable" RESET);
			return -1;
		}

		if (AS_INTEGER(ref) != 42) {
			printf(ERROR "Failed to retreive the correct variable value" RESET);
			return -1;
		}

		//cleanup
		scope = popScope(scope);

		freeLiteral(identifier);
		freeLiteral(type);
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

