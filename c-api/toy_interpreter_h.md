# toy_interpreter.h

This header defines the structure `Toy_Interpreter`, which is the beating heart of Toy.

The `Toy_Interpreter` is a stack-based, bytecode-driven interpreter with a number of customisation options, including "hooks"; native C functions wrapped in `Toy_Literal` instances, injected in order to give the Toy scripts access to libraries via the `import` keyword. The hooks, when invoked this way, can then inject further native functions into the interpreter's current scope. Exactly which hooks are made available varies by host program, but `standard` is the most commonly included one.

Another useful customisation feature is the ability to redicrect output from the `print` and `assert` keywords, as well as any internal errors that occur. This can allow you to add in a logging system, or even hook the `print` statement up to some kind of HUD.

## Defined Interfaces

### typedef void (*Toy_PrintFn)(const char*)

This is the interface used by "print functions" - that is, functions used to print messages from the `print` and `assert` keywords, as well as internal interpreter errors.

## Defined Functions

### void Toy_initInterpreter(Toy_Interpreter* interpreter)

This function initializes the `Toy_Interpreter`. It allocates memory for internal systems such as the stack, and zeroes-out systems that have yet to be invoked. Internally, it also invokes `Toy_resetInterpreter` to initialize the environment.

### void Toy_runInterpreter(Toy_Interpreter* interpreter, const unsigned char* bytecode, size_t length)

This function takes a `Toy_Interpreter` and `bytecode` (as well as the `length` of the bytecode), checks its version information, parses and un-flattens the literal cache, and executes the compiled program stored in the bytecode. This function also consumes the bytecode, so the `bytecode` argument is no longer valid after calls.

If the given bytecode's embedded version is not compatible with the current interpreter, then this function will refuse to execute.

Re-using a `Toy_Interpreter` instance without first resetting it is possible (that's how the repl works), however doing so may have unintended consequences if the scripts are not intended to be used in such a way. Any variables declared will persist.

### void Toy_resetInterpreter(Toy_Interpreter* interpreter)

This function frees any environment that the scripts have built up, and generates a new one. It also injects several globally available functions:

* set
* get
* push
* pop
* length
* clear

### void Toy_freeInterpreter(Toy_Interpreter* interpreter)

This function frees a `Toy_Interpreter`, clearing all of the memory used within. That interpreter is no longer valid for use, and must be re-initialized.

### bool Toy_injectNativeFn(Toy_Interpreter* interpreter, const char* name, Toy_NativeFn func)

This function will inject the given native function `func` into the `Toy_Interpreter`'s current scope, with the name passed as `name`. Both the name and function will be converted into literals internally before being stored. It will return true on success, otherwise it will return false.

The primary use of this function is within hooks.

### bool Toy_injectNativeHook(Toy_Interpreter* interpreter, const char* name, Toy_HookFn hook)

This function will inject the given native function `hook` into the `Toy_Interpreter`'s hook cache, with the name passed in as `name`. Both the name and the function will be converted into literals internally before being stored. It will return true on success, otherwise it will return false.

### bool Toy_callLiteralFn(Toy_Interpreter* interpreter, Toy_Literal func, Toy_LiteralArray* arguments, Toy_LiteralArray* returns)

This function calls a `Toy_Literal` which contains a function, with the arguments to that function passed in as `arguments` and the results stored in `returns`. It returns true on success, otherwise it returns false.

The literal `func` can be either a native function or a Toy function, but it won't execute a hook.

### bool Toy_callFn(Toy_Interpreter* interpreter, const char* name, Toy_LiteralArray* arguments, Toy_LiteralArray* returns)

This utility function will find a `Toy_literal` within the `Toy_Interpreter`'s scope with a name that matches `name`, and will invoke it using `Toy_callLiteralFn` (passing in `arguments` and `returns` as expected).

### bool Toy_parseIdentifierToValue(Toy_Interpreter* interpreter, Toy_Literal* literalPtr)

Note: Bugfixes related to this function are currently in prgress.

### void Toy_setInterpreterPrint(Toy_Interpreter* interpreter, Toy_PrintFn printOutput)

This function sets the function called by the `print` keyword. By default, the following wrapper is used:

```c
static void printWrapper(const char* output) {
	printf("%s\n", output);
}
```

### void Toy_setInterpreterAssert(Toy_Interpreter* interpreter, Toy_PrintFn assertOutput)

This function sets the function called by the `assert` keyword on failure. By default, the following wrapper is used:

```c
static void assertWrapper(const char* output) {
	fprintf(stderr, "Assertion failure: %s\n", output);
}
```

### void Toy_setInterpreterError(Toy_Interpreter* interpreter, Toy_PrintFn errorOutput)

This function sets the function called when an error occurs within the interpreter. By default, the following wrapper is used:

```c
static void errorWrapper(const char* output) {
	fprintf(stderr, "%s", output); //no newline
}
```

