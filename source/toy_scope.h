#pragma once

/*!
# toy_scope.h

This header defines the scope structure, which stores all of the variables used within a given block of code.

Scopes are arranged into a linked list of ancestors, each of which is reference counted. When a scope is popped off the end of the chain, every ancestor scope has it's reference counter reduced by 1 and, if any reach 0, they are freed.

This is also where Toy's type system lives.
!*/

#include "toy_literal.h"
#include "toy_literal_array.h"
#include "toy_literal_dictionary.h"

typedef struct Toy_Scope {
	Toy_LiteralDictionary variables; //only allow identifiers as the keys
	Toy_LiteralDictionary types; //the types, indexed by identifiers
	struct Toy_Scope* ancestor;
	int references; //how many scopes point here
} Toy_Scope;

/*!
## Defined Functions
!*/

/*!
### Toy_Scope* Toy_pushScope(Toy_Scope* scope)

This function creates a new `Toy_scope` with `scope` as it's ancestor, and returns it.
!*/
TOY_API Toy_Scope* Toy_pushScope(Toy_Scope* scope);

/*!
### Toy_Scope* Toy_popScope(Toy_Scope* scope)

This function frees the given `scope`, and returns it's ancestor.
!*/
TOY_API Toy_Scope* Toy_popScope(Toy_Scope* scope);

/*!
### Toy_Scope* Toy_copyScope(Toy_Scope* original)

This function copies an existing scope, and returns the copy.

This copies the internal dictionaries, so it can be memory intensive.
!*/
TOY_API Toy_Scope* Toy_copyScope(Toy_Scope* original);

/*!
### bool Toy_declareScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal type)

This function declares a new variable `key` within `scope`, giving it the type of `type`.

This function returns true on success, otherwise it returns failure (such as if the given key already exists).
!*/
TOY_API bool Toy_declareScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal type);

/*!
### bool Toy_isDelcaredScopeVariable(Toy_Scope* scope, Toy_Literal key)

This function checks to see if a given variable with the name `key` has been previously declared.
!*/
TOY_API bool Toy_isDelcaredScopeVariable(Toy_Scope* scope, Toy_Literal key);

/*!
### bool Toy_setScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal value, bool constCheck)

This function sets an existing variable named `key` to the value of `value`. This function fails if `constCheck` is true and the given key's type has the constaant flag set. It also fails if the given key doesn't exist.

This function returns true on success, otherwise it returns false.
!*/
TOY_API bool Toy_setScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal value, bool constCheck);

/*!
### bool Toy_getScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal* value)

This function sets the literal pointed to by `value` to equal the variable named `key`.

This function returns true on success, otherwise it returns false.
!*/
TOY_API bool Toy_getScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal* value);

/*!
### Toy_Literal Toy_getScopeType(Toy_Scope* scope, Toy_Literal key)

This function returns a new `Toy_Literal` representing the type of the variable named `key`.
!*/
TOY_API Toy_Literal Toy_getScopeType(Toy_Scope* scope, Toy_Literal key);
