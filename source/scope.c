#include "scope.h"

#include "memory.h"

//run up the ancestor chain, freeing anything with 0 references left
static void freeAncestorChain(Scope* scope) {
	scope->references--;

	//free scope chain
	if (scope->ancestor != NULL) {
		freeAncestorChain(scope->ancestor);
	}

	if (scope->references > 0) {
		return;
	}

	freeLiteralDictionary(&scope->variables);

	FREE(Scope, scope);
}

//exposed functions
Scope* pushScope(Scope* ancestor) {
	Scope* scope = ALLOCATE(Scope, 1);
	scope->ancestor = ancestor;
	initLiteralDictionary(&scope->variables);

	//tick up all scope reference counts
	scope->references = 0;
	for (Scope* ptr = scope; ptr; ptr = ptr->ancestor) {
		ptr->references++;
	}

	return scope;
}

Scope* popScope(Scope* scope) {
	Scope* ret = scope->ancestor;

	freeAncestorChain(scope);

	return ret;
}

//returns false if error
bool declareScopeVariable(Scope* scope, Literal key, Literal type) {
	//store the type, for later checking on assignment
	//TODO

	//don't redefine a variable within this scope
	if (existsLiteralDictionary(&scope->variables, key)) {
		return false;
	}

	setLiteralDictionary(&scope->variables, key, TO_NULL_LITERAL);
	return true;
}

//return false if undefined
bool setScopeVariable(Scope* scope, Literal key, Literal value) {
	//dead end
	if (scope == NULL) {
		return false;
	}

	//if it's not in this scope, keep searching up the chain
	if (!existsLiteralDictionary(&scope->variables, key)) {
		return setScopeVariable(scope->ancestor, key, value);
	}

	setLiteralDictionary(&scope->variables, key, value);
	return true;
}

bool getScopeVariable(Scope* scope, Literal key, Literal* valueHandle) {
	//dead end
	if (scope == NULL) {
		return false;
	}

	//if it's not in this scope, keep searching up the chain
	if (!existsLiteralDictionary(&scope->variables, key)) {
		return getScopeVariable(scope->ancestor, key, valueHandle);
	}

	*valueHandle = getLiteralDictionary(&scope->variables, key);
	return true;
}
