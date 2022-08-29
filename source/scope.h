#pragma once

#include "literal_array.h"
#include "literal_dictionary.h"

typedef struct Scope {
	LiteralDictionary variables; //only allow identifiers as the keys
	LiteralDictionary types; //the types, indexed by identifiers
	struct Scope* ancestor;
	int references; //how many scopes point here
} Scope;

Scope* pushScope(Scope* scope);
Scope* popScope(Scope* scope);
Scope* copyScope(Scope* original);

//returns false if error
bool declareScopeVariable(Scope* scope, Literal key, Literal type);
bool isDelcaredScopeVariable(Scope* scope, Literal key);

//return false if undefined
bool setScopeVariable(Scope* scope, Literal key, Literal value, bool constCheck);
bool getScopeVariable(Scope* scope, Literal key, Literal* value);

Literal getScopeType(Scope* scope, Literal key);
