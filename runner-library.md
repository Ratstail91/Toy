# Runner Library

The runner library is used to execute one script from inside another. It also has functions that allow you to retrieve variables from the other script.

The runner library has a concept called a "dirty" script - dirty scripts are those which have already been run, and whose variables can be accessed. Dirty scripts must be reset before it is run again.

When using this library, you must first initialize the "drives" that are available. A drive is a simple way to ensure that the user and any modders don't have access to the entire hard drive. To set up the drives, you must designate a name for each folder you wish to enable access to, like so:

```c
//it's a good idea to place this early in the program's execution, where it will only be run once
int main(int argc, const char* argv[]) {
    //the drive system uses a LiteralDictionary, which must be initialized with this
    Toy_initDriveDictionary();

    //create a pair of literals, the first for the drive name, the second for the path
    Toy_Literal driveLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("scripts"));
    Toy_Literal pathLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("C:/path/to/scripts"));

    //set these within the drive dictionary
    Toy_setLiteralDictionary(Toy_getDriveDictionary(), driveLiteral, pathLiteral);

    //these literals are no longer needed
    Toy_freeLiteral(driveLiteral);
    Toy_freeLiteral(pathLiteral);

    //run the rest of your program
    repl();

    //clean up the drive dictionary when you're done
    Toy_freeDriveDictionary();

    return 0;
}
```

The runner library can usually be accessed with the `import` keyword:

```toy
import runner;
```

## loadScript(path: string)

This is used to load an external script into an opaque variable.

This function does a lot of work:

* It validates the file path using the drive syntax (see above)
* It reads in the source code of the script file
* It compiles the source script into bytecode
* It constructs and intializes an Interpreter
* It packages it all into an opaque variable and returns it

## loadScriptBytecode(path: string)

This is used to load an external bytecode file into an opaque variable.

This function does a lot of work:

* It validates the file path using the drive syntax (see above)
* It constructs and intializes an Interpreter
* It packages it all into an opaque variable and returns it

Note: This function resembles `loadScript()`, but skips the compilation step.

## runScript(self: opaque)

This function executes an external script, which must first be loaded into an opaque variable with either `loadScript()` or `loadScriptBytecode()`.

## getScriptVar(self: opaque, name: string)

This function retrieves a variable from the top level of a script's environment.

## callScriptFn(self: opaque, name: string, ...rest)

This function retrieves a function from the top level of a script's environment, and calls it with `rest` as the argument list.

## resetScript(self: opaque)

This function resets the script so that it is no longer in a "dirty" state, and can be re-run using `runScript()`.

## freeScript(self: opaque)

This function frees a script's resources, cleaning up any memory that is no longer needed. Failing to call this will result in a memory leak.

