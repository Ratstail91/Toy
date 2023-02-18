# toy_scope.h

This header defines the `Toy_Scope` structure, which stores all of the variables used within a given block of code.

Scopes are arranged into a linked list of ancestors, each of which is reference counted. When a scope is popped off the end of the chain, every ancestor scope has it's reference counter reduced by 1 and, if any reach 0, they are freed.

## Defined Functions

### Toy_Scope* Toy_pushScope(Toy_Scope* scope)

This function creates a new `Toy_scope` with `scope` as it's ancestor, and returns it.

### Toy_Scope* Toy_popScope(Toy_Scope* scope)

This function frees the given `scope`, and returns it's ancestor.

### Toy_Scope* Toy_copyScope(Toy_Scope* original)

This function copies an existing scope, and returns the copy.

### bool Toy_declareScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal type)

This function declares a new variable `key` within `scope`, giving it the type of `type`.

This function returns true on success, otherwise it returns failure (such as if the given key already exists).

### bool Toy_isDelcaredScopeVariable(Toy_Scope* scope, Toy_Literal key)

This function checks to see if a given variable with the name `key` has been previously declared.

### bool Toy_setScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal value, bool constCheck)

This function sets an existing variable named `key` to the value of `value`. This function fails if `constCheck` is true and the given key's type has the constaant flag set. It also fails if the given key doesn't exist.

This function returns true on success, otherwise it returns false.

### bool Toy_getScopeVariable(Toy_Scope* scope, Toy_Literal key, Toy_Literal* value)

This function sets the literal pointed to by `value` to equal the variable named `key`.

This function returns true on success, otherwise it returns false.

### Toy_Literal Toy_getScopeType(Toy_Scope* scope, Toy_Literal key)

This function returns a new `Toy_Literal` representing the type of the variable named `key`.

