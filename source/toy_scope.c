#include "toy_scope.h"

#include "toy_memory.h"

//run up the ancestor chain, freeing anything with 0 references left
static void freeAncestorChain(Toy_Scope* scope) {
	scope->references--;

	//free scope chain
	if (scope->ancestor != NULL) {
		freeAncestorChain(scope->ancestor);
	}

	if (scope->references > 0) {
		return;
	}

	Toy_freeLiteralDictionary(&scope->variables);
	Toy_freeLiteralDictionary(&scope->types);

	TOY_FREE(Toy_Scope, scope);
}

//return false if invalid type
static bool checkType(Toy_Literal typeLiteral, Toy_Literal original, Toy_Literal value, bool constCheck) {
	//for constants, fail if original != value
	if (constCheck && TOY_AS_TYPE(typeLiteral).constant && !Toy_literalsAreEqual(original, value)) {
		return false;
	}

	//for any types
	if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_ANY) {
		return true;
	}

	//don't allow null types
	if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_NULL) {
		return false;
	}

	//always allow null values
	if (TOY_IS_NULL(value)) {
		return true;
	}

	//for each type, if a mismatch is found, return false
	if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_BOOLEAN && !TOY_IS_BOOLEAN(value)) {
		return false;
	}

	if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_INTEGER && !TOY_IS_INTEGER(value)) {
		return false;
	}

	if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_FLOAT && !TOY_IS_FLOAT(value)) {
		return false;
	}

	if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_STRING && !TOY_IS_STRING(value)) {
		return false;
	}

	if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_ARRAY && !TOY_IS_ARRAY(value)) {
		return false;
	}

	if (TOY_IS_ARRAY(value)) {
		//check value's type
		if (TOY_AS_TYPE(typeLiteral).typeOf != TOY_LITERAL_ARRAY) {
			return false;
		}

		//if null, assume it's a new array variable that needs checking
		if (TOY_IS_NULL(original)) {
			for (int i = 0; i < TOY_AS_ARRAY(value)->count; i++) {
				if (!checkType( ((Toy_Literal*)(TOY_AS_TYPE(typeLiteral).subtypes))[0], TOY_TO_NULL_LITERAL, TOY_AS_ARRAY(value)->literals[i], constCheck)) {
					return false;
				}
			}

			return true;
		}

		//check children
		for (int i = 0; i < TOY_AS_ARRAY(value)->count; i++) {
			if (TOY_AS_ARRAY(original)->count <= i) {
				return true; //assume new entry pushed
			}

			if (!checkType(((Toy_Literal*)(TOY_AS_TYPE(typeLiteral).subtypes))[0], TOY_AS_ARRAY(original)->literals[i], TOY_AS_ARRAY(value)->literals[i], constCheck)) {
				return false;
			}
		}
	}

	if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_DICTIONARY && !TOY_IS_DICTIONARY(value)) {
		return false;
	}

	if (TOY_IS_DICTIONARY(value)) {
		//check value's type
		if (TOY_AS_TYPE(typeLiteral).typeOf != TOY_LITERAL_DICTIONARY) {
			return false;
		}

		//if null, assume it's a new dictionary variable that needs checking
		if (TOY_IS_NULL(original)) {
			for (int i = 0; i < TOY_AS_DICTIONARY(value)->capacity; i++) {
				//check the type of key and value
				if (!checkType(((Toy_Literal*)(TOY_AS_TYPE(typeLiteral).subtypes))[0], TOY_TO_NULL_LITERAL, TOY_AS_DICTIONARY(value)->entries[i].key, constCheck)) {
					return false;
				}

				if (!checkType(((Toy_Literal*)(TOY_AS_TYPE(typeLiteral).subtypes))[1], TOY_TO_NULL_LITERAL, TOY_AS_DICTIONARY(value)->entries[i].value, constCheck)) {
					return false;
				}
			}

			return true;
		}

		//check each child of value against the child of original
		for (int i = 0; i < TOY_AS_DICTIONARY(value)->capacity; i++) {
			if (TOY_IS_NULL(TOY_AS_DICTIONARY(value)->entries[i].key)) { //only non-tombstones
				continue;
			}

			//find the internal child of original that matches this child of value
			Toy_private_entry* ptr = NULL;

			for (int j = 0; j < TOY_AS_DICTIONARY(original)->capacity; j++) {
				if (Toy_literalsAreEqual(TOY_AS_DICTIONARY(original)->entries[j].key, TOY_AS_DICTIONARY(value)->entries[i].key)) {
					ptr = &TOY_AS_DICTIONARY(original)->entries[j];
					break;
				}
			}

			//if not found, assume it's a new entry
			if (!ptr) {
				continue;
			}

			//check the type of key and value
			if (!checkType(((Toy_Literal*)(TOY_AS_TYPE(typeLiteral).subtypes))[0], ptr->key, TOY_AS_DICTIONARY(value)->entries[i].key, constCheck)) {
				return false;
			}

			if (!checkType(((Toy_Literal*)(TOY_AS_TYPE(typeLiteral).subtypes))[1], ptr->value, TOY_AS_DICTIONARY(value)->entries[i].value, constCheck)) {
				return false;
			}
		}
	}

	if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_FUNCTION && !TOY_IS_FUNCTION(value)) {
		return false;
	}

	if (TOY_AS_TYPE(typeLiteral).typeOf == TOY_LITERAL_TYPE && !TOY_IS_TYPE(value)) {
		return false;
	}

	return true;
}

//exposed functions
Toy_Scope* Toy_pushScope(Toy_Scope* ancestor) {
	Toy_Scope* scope = TOY_ALLOCATE(Toy_Scope, 1);
	scope->ancestor = ancestor;
	Toy_initLiteralDictionary(&scope->variables);
	Toy_initLiteralDictionary(&scope->types);

	//tick up all scope reference counts
	scope->references = 0;
	for (Toy_Scope* ptr = scope; ptr != NULL; ptr = ptr->ancestor) {
		ptr->references++;
	}

	return scope;
}

Toy_Scope* Toy_popScope(Toy_Scope* scope) {
	if (scope == NULL) { //CAN pop a null
		return NULL;
	}

	Toy_Scope* ret = scope->ancestor;

	//BUGFIX: when freeing a scope, free the function's scopes manually
	for (int i = 0; i < scope->variables.capacity; i++) {
		//handle keys, just in case
		if (TOY_IS_FUNCTION(scope->variables.entries[i].key)) {
			Toy_popScope(TOY_AS_FUNCTION(scope->variables.entries[i].key).scope);
			TOY_AS_FUNCTION(scope->variables.entries[i].key).scope = NULL;
		}

		if (TOY_IS_FUNCTION(scope->variables.entries[i].value)) {
			Toy_popScope(TOY_AS_FUNCTION(scope->variables.entries[i].value).scope);
			TOY_AS_FUNCTION(scope->variables.entries[i].value).scope = NULL;
		}
	}

	freeAncestorChain(scope);

	return ret;
}

Toy_Scope* Toy_copyScope(Toy_Scope* original) {
	Toy_Scope* scope = TOY_ALLOCATE(Toy_Scope, 1);
	scope->ancestor = original->ancestor;
	Toy_initLiteralDictionary(&scope->variables);
	Toy_initLiteralDictionary(&scope->types);

	//tick up all scope reference counts
	scope->references = 0;
	for (Toy_Scope* ptr = scope; ptr != NULL; ptr = ptr->ancestor) {
		ptr->references++;
	}

	//copy the contents of the dictionaries
	for (int i = 0; i < original->variables.capacity; i++) {
		if (!TOY_IS_NULL(original->variables.entries[i].key)) {
			Toy_setLiteralDictionary(&scope->variables, original->variables.entries[i].key, original->variables.entries[i].value);
		}
	}

	for (int i = 0; i < original->types.capacity; i++) {
		if (!TOY_IS_NULL(original->types.entries[i].key)) {
			Toy_setLiteralDictionary(&scope->types, original->types.entries[i].key, original->types.entries[i].value);
		}
	}

	return scope;
}

//returns false if error
bool Toy_declareScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal type) {
	//don't redefine a variable within this scope
	if (Toy_existsLiteralDictionary(&scope->variables, key)) {
		return false;
	}

	if (!TOY_IS_TYPE(type)) {
		return false;
	}

	//store the type, for later checking on assignment
	Toy_setLiteralDictionary(&scope->types, key, type);

	Toy_setLiteralDictionary(&scope->variables, key, TOY_TO_NULL_LITERAL);
	return true;
}

bool Toy_isDelcaredScopeVariable(Toy_Scope* scope, Toy_Literal key) {
	if (scope == NULL) {
		return false;
	}

	//if it's not in this scope, keep searching up the chain
	if (!Toy_existsLiteralDictionary(&scope->variables, key)) {
		return Toy_isDelcaredScopeVariable(scope->ancestor, key);
	}

	return true;
}

//return false if undefined, or can't be assigned
bool Toy_setScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal value, bool constCheck) {
	//dead end
	if (scope == NULL) {
		return false;
	}

	//if it's not in this scope, keep searching up the chain
	if (!Toy_existsLiteralDictionary(&scope->variables, key)) {
		return Toy_setScopeVariable(scope->ancestor, key, value, constCheck);
	}

	//type checking
	Toy_Literal typeLiteral = Toy_getLiteralDictionary(&scope->types, key);
	Toy_Literal original = Toy_getLiteralDictionary(&scope->variables, key);

	if (!checkType(typeLiteral, original, value, constCheck)) {
		Toy_freeLiteral(typeLiteral);
		Toy_freeLiteral(original);
		return false;
	}

	//actually assign
	Toy_setLiteralDictionary(&scope->variables, key, value);

	Toy_freeLiteral(typeLiteral);
	Toy_freeLiteral(original);

	return true;
}

bool Toy_getScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal* valueHandle) {
	//dead end
	if (scope == NULL) {
		return false;
	}

	//if it's not in this scope, keep searching up the chain
	if (!Toy_existsLiteralDictionary(&scope->variables, key)) {
		return Toy_getScopeVariable(scope->ancestor, key, valueHandle);
	}

	*valueHandle = Toy_getLiteralDictionary(&scope->variables, key);
	return true;
}

Toy_Literal Toy_getScopeType(Toy_Scope* scope, Toy_Literal key) {
	//dead end
	if (scope == NULL) {
		return TOY_TO_NULL_LITERAL;
	}

	//if it's not in this scope, keep searching up the chain
	if (!Toy_existsLiteralDictionary(&scope->types, key)) {
		return Toy_getScopeType(scope->ancestor, key);
	}

	return Toy_getLiteralDictionary(&scope->types, key);
}
