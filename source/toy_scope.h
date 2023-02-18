#pragma once

#include "toy_literal.h"
#include "toy_literal_array.h"
#include "toy_literal_dictionary.h"

typedef struct Toy_Scope {
	Toy_LiteralDictionary variables; //only allow identifiers as the keys
	Toy_LiteralDictionary types; //the types, indexed by identifiers
	struct Toy_Scope* ancestor;
	int references; //how many scopes point here
} Toy_Scope;

TOY_API Toy_Scope* Toy_pushScope(Toy_Scope* scope);
TOY_API Toy_Scope* Toy_popScope(Toy_Scope* scope);
TOY_API Toy_Scope* Toy_copyScope(Toy_Scope* original);

//returns false if error
TOY_API bool Toy_declareScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal type);
TOY_API bool Toy_isDelcaredScopeVariable(Toy_Scope* scope, Toy_Literal key);

//return false if undefined
TOY_API bool Toy_setScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal value, bool constCheck);
TOY_API bool Toy_getScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal* value);

TOY_API Toy_Literal Toy_getScopeType(Toy_Scope* scope, Toy_Literal key);
