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
static bool checkType(Literal typeLiteral, Literal original, Literal value, bool constCheck) {
	//for constants, fail if original != value
	if (constCheck && AS_TYPE(typeLiteral).constant && !literalsAreEqual(original, value)) {
		return false;
	}

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

	if (AS_TYPE(typeLiteral).typeOf == LITERAL_ARRAY && !IS_ARRAY(value)) {
		return false;
	}

	if (IS_ARRAY(value)) {
		//check value's type
		if (AS_TYPE(typeLiteral).typeOf != LITERAL_ARRAY) {
			return false;
		}

		//if null, assume it's a new entry
		if (IS_NULL(original)) {
			return true;
		}

		//check children
		for (int i = 0; i < AS_ARRAY(value)->count; i++) {
			if (AS_ARRAY(original)->count <= i) {
				return true; //assume new entry pushed
			}

			if (!checkType(((Literal*)(AS_TYPE(typeLiteral).subtypes))[0], AS_ARRAY(original)->literals[i], AS_ARRAY(value)->literals[i], constCheck)) {
				return false;
			}
		}
	}

	if (AS_TYPE(typeLiteral).typeOf == LITERAL_DICTIONARY && !IS_DICTIONARY(value)) {
		return false;
	}

	if (IS_DICTIONARY(value)) {
		//check value's type
		if (AS_TYPE(typeLiteral).typeOf != LITERAL_DICTIONARY) {
			return false;
		}

		//if null, assume it's a new entry
		if (IS_NULL(original)) {
			return true;
		}

		//check each child of value against the child of original
		for (int i = 0; i < AS_DICTIONARY(value)->capacity; i++) {
			if (IS_NULL(AS_DICTIONARY(value)->entries[i].key)) { //only non-tombstones
				continue;
			}

			//find the internal child of original that matches this child of value
			_entry* ptr = NULL;

			for (int j = 0; j < AS_DICTIONARY(original)->capacity; j++) {
				if (literalsAreEqual(AS_DICTIONARY(original)->entries[j].key, AS_DICTIONARY(value)->entries[i].key)) {
					ptr = &AS_DICTIONARY(original)->entries[j];
					break;
				}
			}

			//if not found, assume it's a new entry
			if (!ptr) {
				continue;
			}

			//check the type of key and value
			if (!checkType(((Literal*)(AS_TYPE(typeLiteral).subtypes))[0], ptr->key, AS_DICTIONARY(value)->entries[i].key, constCheck)) {
				return false;
			}

			if (!checkType(((Literal*)(AS_TYPE(typeLiteral).subtypes))[1], ptr->value, AS_DICTIONARY(value)->entries[i].value, constCheck)) {
				return false;
			}
		}
	}

	if (AS_TYPE(typeLiteral).typeOf == LITERAL_FUNCTION && !IS_FUNCTION(value)) {
		return false;
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
	if (scope == NULL) { //CAN pop a null
		return NULL;
	}

	Scope* ret = scope->ancestor;

	//BUGFIX: when freeing a scope, free the function's scopes manually
	for (int i = 0; i < scope->variables.capacity; i++) {
		//handle keys, just in case
		if (IS_FUNCTION(scope->variables.entries[i].key)) {
			popScope(AS_FUNCTION(scope->variables.entries[i].key).scope);
			AS_FUNCTION(scope->variables.entries[i].key).scope = NULL;
		}

		if (IS_FUNCTION(scope->variables.entries[i].value)) {
			popScope(AS_FUNCTION(scope->variables.entries[i].value).scope);
			AS_FUNCTION(scope->variables.entries[i].value).scope = NULL;
		}
	}

	freeAncestorChain(scope);

	return ret;
}

Scope* copyScope(Scope* original) {
	Scope* scope = ALLOCATE(Scope, 1);
	scope->ancestor = original->ancestor;
	initLiteralDictionary(&scope->variables);
	initLiteralDictionary(&scope->types);

	//tick up all scope reference counts
	scope->references = 0;
	for (Scope* ptr = scope; ptr != NULL; ptr = ptr->ancestor) {
		ptr->references++;
	}

	//copy the contents of the dictionaries
	for (int i = 0; i < original->variables.capacity; i++) {
		if (!IS_NULL(original->variables.entries[i].key)) {
			setLiteralDictionary(&scope->variables, original->variables.entries[i].key, original->variables.entries[i].value);
		}
	}

	for (int i = 0; i < original->types.capacity; i++) {
		if (!IS_NULL(original->types.entries[i].key)) {
			setLiteralDictionary(&scope->types, original->types.entries[i].key, original->types.entries[i].value);
		}
	}

	return scope;
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
	Literal original = getLiteralDictionary(&scope->variables, key);

	if (!checkType(typeLiteral, original, value, constCheck)) {
		freeLiteral(typeLiteral);
		freeLiteral(original);
		return false;
	}

	//actually assign
	setLiteralDictionary(&scope->variables, key, value);

	freeLiteral(typeLiteral);
	freeLiteral(original);

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

Literal getScopeType(Scope* scope, Literal key) {
	//dead end
	if (scope == NULL) {
		return TO_NULL_LITERAL;
	}

	//if it's not in this scope, keep searching up the chain
	if (!existsLiteralDictionary(&scope->types, key)) {
		return getScopeType(scope->ancestor, key);
	}

	return getLiteralDictionary(&scope->types, key);
}
