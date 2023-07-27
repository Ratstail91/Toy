# Developing Toy

Here you'll find some of the implementation details.

## Bytecode Header Format

The bytecode header format must not change.

Every instance of Toy bytecode will be divided up into several sections, by necessity - however the first one to be read is the header. This section is used to define what version of Toy is currently running, as well as to prevent any future version/fork clashes.

The header consists of four values:

* TOY_VERSION_MAJOR
* TOY_VERSION_MINOR
* TOY_VERSION_PATCH
* TOY_VERSION_BUILD

The first three are single unsigned bytes, embedded at the beginning of the bytecode in sequence. These represent the major, minor and patch versions of the language. The fourth value is a null-terminated string of unspecified data, which is *intended* but not required to specify the time that the langauge's compiler was itself compiled. The build string can hold arbitrary data, such as the current maintainer's name, current fork of the language, or other versioning info.

There are some strict rules when interpreting these values (mimicking, but not conforming to [semver.org](https://semver.org/)):

* Under no circumstance, should you ever run bytecode whose major version is different - there are definitely broken APIs involved.
* Under no circumstance, should you ever run bytecode whose minor version is above the interpreter's minor version - the bytecode could potentially use unimplemented features.
* You may, at your own risk, attempt to run bytecode whose patch version is different.
* You may, at your own risk, attempt to run bytecode whose build version is different.

All interpreter implementations retain the right to reject any bytecode whose header data does not conform to the above specification.

The latest version information can be found in [toy_common.h](https://github.com/Ratstail91/Toy/blob/main/source/toy_common.h)


# Parser Structure and Operations

TODO

# Compiler Structure and Operations

No.

# Interpreter Structure and Operations

The Toy interpreter is, at it's core, just a big loop that reads bytes from memory and acts on them. Here, I'll break down exactly how it works, from a top-down perspective.

### Running the Interpreter

There are four main functions for running the interpreter:

* `Toy_initInterpreter`
* `Toy_runInterpreter`
* `Toy_resetInterpreter`
* `Toy_freeInterpreter`

First, `init` zeroes out the interpreter, sets up the printing functions, and delegates to `reset`, which in turn sets up the program's scope (and injects the default global functions). The initialization function is split into two this way so that `reset` can be used independantly on a "dirty" interpreter to ready it for another script (or another run of the same script). `reset` is usually not needed and may be removed in future.

`free` simply frees the interpreter after execution.

Interestingly, `run` doesn't jump straight into exection. Instead, it first does it's own bit of setup, before reading out the bytecode's header. If the header indicates an incompatible version, then the interpreter will refuse to run, to prevent mistakes from ruining the program.

`run` will also delegate to a function called `readInterpreterSections()`, which reads and reconstructs the "literalCache" - a collection of all values within the program (variable identifiers, variable values, function bytecode, etc.)

Next, `run` will pass to a function called `execInterpreter()`, which contains the program's loop.

Finally, `run` will automatically free the bytecode and associated literalCache (this may change at some point).

### Bytecode Layout

I don't know.

To put it bluntly, the layout of the compressed bytecode was very adhoc, and as such it was not documented at the time. This was partially because I (wrongly) believed that the layout didn't matter much, only the final execution.

I can say a few things about it though -

* Literal compounds are stored as arrays of integers which reference previously declared literals
* Functions are stored *after* the literal cache, in their own section and are referenced in the literal cache by index
* Functions are structured very similarly to the program as a whole, and store their argument and return arrays within their own literalCaches

I will document this one day, but not any time soon.

### Executing the Interpreter

Opcodes within the bytecode are 1 byte in length, and specify a single action to take. Each possible action is definied within the interpreter in a function that begins with `exec`, and are called from within a big looping switch statement. If any of these `exec` functions encounters an error, they can simply return false to break the loop.

The interpeter is stack-based; most, if not all of the actions are preformed on literals within a specially designated array called `stack`. for example:

```c
	case TOY_OP_PRINT:
		if (!execPrint(interpreter)) {
			return;
		}
	break;
```

When a the opcode `TOY_OP_PRINT` is encountered, the top literal within the stack is popped off, and printed (more info on literals below).

```c
static bool execPrint(Toy_Interpreter* interpreter) {
	//get the top literal
	Toy_Literal lit = Toy_popLiteralArray(&interpreter->stack);

	//if the top literal is an identifier, get it's value
	Toy_Literal idn = lit;
	if (TOY_IS_IDENTIFIER(lit) && Toy_parseIdentifierToValue(interpreter, &lit)) {
		Toy_freeLiteral(idn);
	}

	//print as a string to the current print method
	Toy_printLiteralCustom(lit, interpreter->printOutput);

	//free the literal
	Toy_freeLiteral(lit);

	//continue the loop
	return true;
}
```

### Identity Crisis

As in most programming languages, variables can be represented by names specified by the programmer; in Toy, these are called "identifiers". These identifiers can be passed around in place of their actual values, but can't be used directly. To retrieve a value, you must first "parse" it, like so:

```c
Toy_Literal idn = literal; //cache the literal, just in case it's an identifier
if (TOY_IS_IDENTIFIER(literal) && Toy_parseIdentifierToValue(interpreter, &literal)) { //if it is an identifier, parse it...
	Toy_freeLiteral(idn); //always remember to free the original identifier, otherwise you'll have a memory leak!
}
```

You will often see this pattern throughout the codebase.

### Other Utility Functions

Other functions are available at the top of the interpreter source file:

* printing utilities
* injection utilities
* parsing utilities
* bytecode utilities
* function utilities (these ones is at the very bottom of the source file)

# Literals

TODO

# Arrays & Dictionaries

TODO