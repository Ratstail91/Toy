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
	freeLiteralDictionary(&scope->types);

	FREE(Scope, scope);
}

//return false if invalid type
static bool checkType(Literal typeLiteral, Literal value) {
	//for any types
	if (AS_TYPE(typeLiteral).typeOf == LITERAL_ANY) {
		return true;
	}

	//don't allow null types
	if (AS_TYPE(typeLiteral).typeOf == LITERAL_NULL) {
		return false;
	}

	//always allow null values
	if (IS_NULL(value)) {
		return true;
	}

	//for each type, if a mismatch is found, return false
	if (AS_TYPE(typeLiteral).typeOf == LITERAL_BOOLEAN && !IS_BOOLEAN(value)) {
		return false;
	}

	if (AS_TYPE(typeLiteral).typeOf == LITERAL_INTEGER && !IS_INTEGER(value)) {
		return false;
	}

	if (AS_TYPE(typeLiteral).typeOf == LITERAL_FLOAT && !IS_FLOAT(value)) {
		return false;
	}

	if (AS_TYPE(typeLiteral).typeOf == LITERAL_STRING && !IS_STRING(value)) {
		return false;
	}

	if (IS_ARRAY(value)) {
		//check value's type
		if (AS_TYPE(typeLiteral).typeOf != LITERAL_ARRAY) {
			return false;
		}

		//check children
		for (int i = 0; i < AS_ARRAY(value)->count; i++) {
			if (!checkType(((Literal*)(AS_TYPE(typeLiteral).subtypes))[0], AS_ARRAY(value)->literals[i])) {
				return false;
			}
		}
	}

	if (IS_DICTIONARY(value)) {
		//check value's type
		if (AS_TYPE(typeLiteral).typeOf != LITERAL_DICTIONARY) {
			return false;
		}

		//check children
		for (int i = 0; i < AS_DICTIONARY(value)->capacity; i++) {
			//only assigned and non-tombstoned keys
			if (!IS_NULL(AS_DICTIONARY(value)->entries[i].key)) {
				if (!checkType(((Literal*)(AS_TYPE(typeLiteral).subtypes))[0], AS_DICTIONARY(value)->entries[i].key)) {
					return false;
				}

				if (!checkType(((Literal*)(AS_TYPE(typeLiteral).subtypes))[1], AS_DICTIONARY(value)->entries[i].value)) {
					return false;
				}
			}
		}
	}

	if (IS_FUNCTION(value)) {
		//check value's type
		if (AS_TYPE(typeLiteral).typeOf != LITERAL_FUNCTION) {
			return false;
		}
	}

	if (AS_TYPE(typeLiteral).typeOf == LITERAL_TYPE && !IS_TYPE(value)) {
		return false;
	}

	return true;
}

//exposed functions
Scope* pushScope(Scope* ancestor) {
	Scope* scope = ALLOCATE(Scope, 1);
	scope->ancestor = ancestor;
	initLiteralDictionary(&scope->variables);
	initLiteralDictionary(&scope->types);

	//tick up all scope reference counts
	scope->references = 0;
	for (Scope* ptr = scope; ptr != NULL; ptr = ptr->ancestor) {
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
	//don't redefine a variable within this scope
	if (existsLiteralDictionary(&scope->variables, key)) {
		return false;
	}

	//store the type, for later checking on assignment
	setLiteralDictionary(&scope->types, key, type);

	setLiteralDictionary(&scope->variables, key, TO_NULL_LITERAL);
	return true;
}

bool isDelcaredScopeVariable(Scope* scope, Literal key) {
	if (scope == NULL) {
		return false;
	}

	//if it's not in this scope, keep searching up the chain
	if (!existsLiteralDictionary(&scope->variables, key)) {
		return isDelcaredScopeVariable(scope->ancestor, key);
	}

	return true;
}

//return false if undefined, or can't be assigned
bool setScopeVariable(Scope* scope, Literal key, Literal value, bool constCheck) {
	//dead end
	if (scope == NULL) {
		return false;
	}

	//if it's not in this scope, keep searching up the chain
	if (!existsLiteralDictionary(&scope->variables, key)) {
		return setScopeVariable(scope->ancestor, key, value, constCheck);
	}

	//type checking
	Literal typeLiteral = getLiteralDictionary(&scope->types, key);

	if (!checkType(typeLiteral, value)) {
		return false;
	}

	//const check
	if (constCheck && (AS_TYPE(typeLiteral).constant)) {
		return false;
	}

	//actually assign
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
