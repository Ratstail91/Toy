# Premise

The Toy programming language is a procedural bytecode-intermediate interpreted language. It isn't intended to operate on it's own, but rather as part of another program (the "host"). This process is intended to allow a decent amount of easy customisation by the host's end user, by exposing logic in script files. Alternatively, binary files in a custom format can be used as well.

The host will provide all of the extensions needed on a case-by-case basis. Script files have the `.toy` file extension, while binary files have the `.tb` file extension.

# Definition And Syntax

## Comments

Toy supports two types of comments, `//single line` and `/* block */`. Comments are used to leave notes for yourself in the code; they are ignored by the lexer.

## Naming

Names used in the language must start with a letter or underscore, and must contain only letters, numbers and underscores. They can also not exceed 256 characters in length.

## List of Reserved Keywords

The following list of keywords cannot be used as names, due to their significance (or potential later use) in the language.

* any
* as
* astype
* assert
* bool
* break
* class (reserved)
* const
* continue
* do (reserved)
* else
* export
* false
* float
* fn
* for
* foreach (reserved)
* if
* import
* in (reserved)
* int
* null
* of (reserved)
* print
* return
* string
* true
* type
* typeof
* var
* while

## List of Operators

The following mathematical operators are available. A definition is omitted here, as they are commonly used in most programming languages.

```
+  -  *  /  %  +=  -=  *=  /=  %=  ++(prefix)  --(prefix)  (postfix)++  (postfix)--
```

Likewise, the following logical operators are available (`&&` is more tightly bound than `||` due to historical reasons):

```
(  )  [  ]  {  }  !  !=  ==  <  >  <=  >=  &&  ||
```

Other operators used throughout the language are: the assignment, colon, semicolon, comma, dot and rest operators:

```
= : ; , . ...
```

## Types

Variables in Toy may have a type. These types are:

* null - no value assigned (cannot be specified as a type, only a value)
* boolean - either `true` or `false`
* integer - any whole number (limits are implementation dependant)
* float - any floating point number (limits are implementation dependant)
* string - a string of characters enclosed in double quotation marks
* array - a collection of 0-indexed variables
* dictionary - a collection of indexable key-value pairs
* function - chunks of reusable code
* type - the type of types
* any - any of the above

Types are optional attachments to names - they can be specified as such:

```
//declare the variable named "x" as an integer with the initial value 0
var x : int = 0;
```

Types are not interoperable, but in some cases they can be converted from one type to another by "casting":

```
//declare the variable named "y" as a float, and assign it the value stored in x (which is being cast to a float)
var y : float = float x;
```

Defining the type of a variable is not required - in such a case, the type defaults to "any".

```
//define the variable named "z" with any type
var z = 1;
```

Types are first-class citizens, meaning they can be stored in and used from variables:

```
//this was originally unintended, but interesting
var t: type = int;
var u: t = 42;
```

To force a type value instead of an array, use the `astype` keyword:

```
var a = [type]; //array containing the type "type"
var b = astype [type]; //type of array of types

var d = b; //types can be re-assigned to other variables
```

When printed, a type is surrounded by `<>` markers in the output.

```
print a; //[<type>]
print b; //<[<type>]>
```

## Const

The "const" keyword can be appended to the type of a variable to fix the value in place - constants, as they're known, can't be changed once they are given a value at declaration.

```
var foo: int const = 42;

foo++; //Error!
```

Also, entire compounds (arrays or dictionaries) can be const, or the indiviudal members can be const without the entire compound being const.

```
var a: [int const] = [1, 2, 3]; //1, 2, and 3 can't be altered, but a can have new members added
var b: [int] const = [1, 2, 3]; //1, 2, and 3 can't be altered, nor can b be altered
```

## Truthyness

Everything is considered "truthy" except the value `false`. Trying to use `null` in a conditional (except assert) will give an error.

## Print

the "print" keyword is used largely for debugging. It takes a single value, and by default prints to stdout (appending a newline), however this can be overwritten by the host program for a variety of reasons. It can also take variables as values.

```
print "Hello world";
```

## Var

To declare a variable, use the keyword `var`, followed by it's name, an optional colon and type, and finally an optional assign operator and initial value:

```
var foo: int = 42;

var bar = "hello world";

var buzz;
```

Variables can be used in place of values at any time, and can be altered and re-assigned.

## If-Else

The keywords `if` and `else` are used to determine different paths of execution that the program can take, based on a condition. If the condition is truthy (i.e. not `false`), then the `if`-path executes, otherwise the `else`-path does. The else keyword and path are optional.

```
if (1 < 2) {
	print "this will always print to the console";
} else {
	print "this will not";
}
```

multiple `if`-`else` statements can be chained in seqence:

```
if (value == 1) {
	print "one";
}
else if (value == 2) {
	print "two";
}
else if (value == 3) {
	print "three";
}
else {
	print "unknown value";
}
```

The braces around the different paths are optional if there is only a single statement that path.

```
if (true)
	print "Will always execute";
```


## While

The `while` keyword loops over a block of code as long as the condition is truthy:

```
//prints 0-9
var counter: int = 0;
while (counter < 10) {
	print counter++;
}
```

The braces around the body of the loop are optional if there is only a single statement.

```
//will print "hello world" forever
while(true)
	print "Hello world";
```

## For

`while` loops can be quite verbose - a way to write a loop quickly and concisely is with the `for` loop. The first clause in the for loop is executed before the loop begins, then the second clause is checked for truthyness before each execution of the loop body. Finally, the third clause is executed after each execution of the loop body:

```
//prints 0-9
for (var i: int = 0; i < 10; i++) {
	print i;
}
```

Side Note: You *can* declare a `const` in the first clause, but it won't be very usefull 99.999% of the time.

The braces around the body of the `for` loop are optional if there is only a single statement.

## Break and Continue

During a loop, you may want to exit or restart early for some reason. For this, the `break` and `continue` keywords are provided.

```
//exit a loop early
for (var i: int = 0; i < 10; i++) {
	print i;
	if (i >= 7) {
		break;
	}
}

//restart a loop early (will still execute the third clause before continuing)
for (var i: int = 0; i < 10; i++) {
	if (i > 7) {
		continue;
	}
	print i;
}
```

## Functions and Returns

Functions, which are chunks of reusable code, can be declared with the `fn` keyword.

```
fn doSomething() {
	//
}
```

Functions can take a set number of parameters within the parenthesis following their names, and can return a value to the calling context using the return keyword.

```
fn combine(a, b, c) { //return type isn't required by default
	return [a, b, c];
}
```

You can define the required return type using a colon after the parameter list, followed by the type.

```
fn combine(a, b, c): [int] {
	return [a, b, c];
}
```

Functions are called by invoking their names, followed by the parenthesis-enclosed list of arguments.

```
simple();

var a = combine(1, 2, 3); //how to get a returned value from a function
```

Functions are first-class citizens, meaning they can be declared and treated like variables. Closures are explicitly supported.

```
fn counterFactory(): fn { //returns a function
	var total: int = 0;

	fn counter(): int {
		return ++total;
	}

	return counter;
}

var tally = counterFactory();

print tally(); //1
print tally(); //2
print tally(); //3
```

Using the "..." operator, an optional number of parameters can be allowed for functions.

```
fn omitFirstArgument(arg1, ...rest) {
	//"rest" comes in as an array of any type
	return rest;
}
```

## Assert

The `assert` keyword takes 2 parameters, separated by a comma. If the first parameter resolves to be false or is null, then the program terminates, and the value of the second parameter is displayed to the user as an error. By default, the error is printed to stderr, but this can be overwritten by the host program.

```
assert true, "This is fine"; //Good!
assert false, "This is not"; //Error!
```

## Import-As

`import` is used to load variables from the host - several optional libraries will be provided this way, as well. The import keyword can only take a valid variable name as it's argument, followed by an optional "as" which stores the results under a different name.

```
import standard;

print clock(); //the clock function is provided by standard, via a library hook

import standard as std;

print std.clock(); //standard becomes a dictionary called "std"
```

## Export-As

`export` is the complement to `import` - it's used to expose certain variables to the host. `as` can rename the exported variable.

```
fn hello() {
	print "hello world";
}

//both exported variables have the same literal value
export hello;
export hello as world;
```

## String

Strings are a series of characters, and are created by surrounding said characters with a pair of double quotation marks `"`. Strings cannot exceed 4096 bytes in length (this amount can be tweaked in the source).

```
var greeting: string = "Hello world";

print greeting[0:4]; //Hello
print greeting[::-1]; //dlrow olleH

greeting[0:4] = "Goodnight";

print greeting; //Goodnight world

print greeting.indexOf("oo"); //1
print greeting.length(); //11
```

## Array

Arrays are collections of variables of the same type stored as one contiguous chunk of data. They can hold any number of values, but "holes" in the array are not permitted. You can access a specific element by using zero-indexing. Nesting arrays within themselves is not allowed.

```
var arr: [string] = ["hello", "world", "foo", "bar"];

print arr[1]; //world

var arr: [int] = []; //empty
```

To create an array of constants, use the `const` keyword in the type definition (read from from right to left):

```
var arr: [int const] = [1, 2, 3]; //1, 2 and 3 are constant, but arr is not

var arr: [int] const = [1, 2, 3]; //the entire thing is constant
```

Arrays can be indexed using traditional bracket notation.

## Dictionary

Dictionaries are key-value collections of variables. Every key has a single associated value; any missing value is considered `null`. `null` is an invalid key type. Nesting dictionaries within themselves is not allowed.

```
var dict: [string:string] = ["key one": "value one", "key two": "value two"];

print dict["key one"]; //value one

var dictTwo: [int:int] = [:]; //empty dictionary
```

Keys, values and dictionaries can be declared const, like so:

```
var dict: [string const:int const] const = [
	"one": 1,
	"two": 2,
	"three": 3
];
```

Dictionaries can be indexed using traditional bracket notation. Existing elements can be accessed or overwritten, or new ones inserted if they don't already exist this way.

```
dict["foo"] = "bar";
print dict["foo"];
```

## Slice Notation

Strings, arrays and dictionaries can be indexed in several ways, via the globally available functions (see below). Elements can be accessed using traditional bracket notation:

```
str[0];

arr[x];

dict["key"];
```

However, "slice notation" is also available for strings and arrays:

```
str[x:y];
```

Here, "x" and "y" are integers that indicate two indexes within a string, and returns a new substring beginning at "x" and ending at "y" (inclusive). If either is omitted, then the first and last element positions respectfully are used. The same applies for the array type.

Replacing parts of a string with another string, or part of an array with another array, is possible using slice notation:

```
var str: string = "hello world";
str[6:11] = "user";
print str; //hello user
```

A third argument is possible:

```
str[x:y:z];
```

Here, "z" indicates how to count the elements - a positive number starts from the beginning of the string or array, while a negative one counts from the end. Also, if a number other than 1 or -1 is used, then every nth element is selected:

```
print str; //Hello world
print str[::2]; //Hlowrd
print str[::-2]; //drwolH
```

0 cannot be used as the third argument.

### Globally Available Functions

The dot notation can be used to access the globally available functions, like so:

```
obj.function(); //passes in obj as the first argument
```

A list of globally available functions is:

```
//usable with arrays, dictionaries and strings - probably a good idea not to use this one, just use index and slice notation
fn _index(self, first, second, third, assign, op) {
	//native code
}

//usable with arrays and dictionaries
fn _set(self, key, value) {
	//native code
}

//usable with arrays and dictionaries
fn _get(self, key) {
	//native code
}

//usable with arrays
fn _push(self, val) {
	//native code
}

//usable with arrays
fn _pop(self) {
	//native code
}

//usable with arrays, dictionaries and strings
fn _length(self) {
	//native code
}

//usable with arrays and dictionaries
fn _clear(self) {
	//native code
}
```

# Standard Library

The standard library has a number of utility functions available, and is provided by default.

```
//gain access to the standard functions
import standard;
```

The following functions are available in the standard library.

* clock() - This function returns the current timestamp.
* concat(self: any, x: any): any - This function requires an array or dictionary with a matching type as "x". This function returns a new dictionary instance which contains the contents of the current array or dictionary combined with the contents of "x". In the event of a dictionary key clash, the key-value pair in the current dictionary is included, and the key-value pair from "x" is discarded.
* containsKey(self: [any, any], k: any): bool - This function returns true if the dictionary contains a key "k", otherwise it returns false.
* containsValue(self: any, v: any): bool - This function returns true if the array or dictionary contains a value "v", otherwise it returns false.
* every(self: any, cb: fn(k: any, v: any)(bool)): bool - This function calls "cb" once for every entry in the array or dictionary (self), with that element passed in as "k" and "v", until cb returns false, at which point "every" returns false. If it reaches the end of the array or dictionary, then "every" returns true.
* filter(self: any, cb: fn(k: any, v: any)(bool)): any - This function calls "cb" once for every entry in the array or dictionary (self), with that key and value passed in as "k" and "v", respectfully. This returns a new array or dictionary that contains every key-value pair for which the call to "cb" returned true.
* indexOf(self: string, str: string): int - This function returns the position of the first instance of "str" in the string "self".
* insert(self: any, key: any, x: any) - This function inserts "value" at the index/key "key", shifting the remaining entry up 1 index if it's an array. This alters the memory.
* keys(self: any): [type] - This function returns an array containing each key in the dictionary. The order of the keys is undefined.
* map(self: any, cb: fn(k: any, v: any)(any)): any - This function calls "cb" once for every entry in the array or dictionary, with that key passed in as "k" and value passed in as "v". It returns a new array or dictionary with the keys copied from the current "self", and values replaced with the results of calls to "cb".
* reduce(self: any, default: any, cb: fn(acc: any, k: any, v: any): any): any - This function calls "cb" once for every element in the array or dictionary "self", with that element passed in as "k" and "v", and the value of the previous call passed in as "acc". For the first call to "cb", "default" is used for "acc". The final value of "acc" is returned by "reduce".
* remove(self: any, k: any) - This function deletes the value at the index or key "k", shifting the remaining entries down 1 index if it's an array. This alters the memory.
* replace(self: string, pat: string, rep: string): string - For each instance of "pat" that it finds in the calling string "self", it replaces it with "rep", then returns the new string.
* some(self: [any, any], cb: fn(k: any, v: any)(bool)): bool - This function calls "cb" once for every entry in the array or dictionary (self), with that element passed in as "key" and "value", until cb returns true, at which point "some" returns true. If it reaches the end of the array or dictionary, then "some" returns false.
* sort(self: [any], cb: fn(lhs: any, rhs: any)(int)): [any] - This function sorts the entries of an array according to the callback "cb". "cb" may be called any number of times during the sorting process. "cb" must be a function that takes two parameters, "lhs" and "rhs" and returns an integer. If "lhs" is less than "rhs", then the returned integer should be negative. If they are equal, then it should be 0. Otherwise it should be positive. This returns the sorted array, leaving the original intact.
* toLower(self: string): string - This function returns a new string, which is the same as the calling string, except all characters are lower case.
* toString(self: any): string - This function returns a string representation of the input "self". For arrays and dictionaries, each element is converted to a string where possible, and separated by commas (and colons for dictionaries). Finally, the whole string is surrounded by brackets.
* toUpper(self: string): string - This function returns a new string, which is the same as the calling string, except all characters are upper case.
* trim(self: string, chars: string): string - Every character in the string "chars" is removed from the calling string's beginning and end, and the new string is returned. The original string is not modified.
* values(self: [any, any]): [any] - This function returns an array containing each value in the dictionary. The order of the values is undefined.

# For Consideration

The following features are under consideration, but will not be explicitly planned for during development.

## Foreach-Of and Foreach-In

These could be used to iterate over the keys and values of dictionaries, respectfully. They could also be applied to arrays and strings.

## Do-While

An alternative version of the while-loop, this structure may be implemented at some point - for the time being, due to the incredibly rare usage, it has been omitted.

## Ternary Operator

The `?:` operator may be implemented at some point due to it's ubiquity.

## Const-ness and Compounds

Ideally, I'd like internal elements of compounds and external members to not share const-ness. Unfortunately, implementation details makes this unviable at this time.

```
var a: [int const] = [1, 2, 3]; //1, 2, and 3 can't be altered, but a can have new members added
var b: [int] const = [1, 2, 3]; //1, 2, and 3 can be altered, but b can't be altered or re-assigned
var c: [int const] const = [1, 2, 3]; //nothing here can change
```

## Multiple Return Types

It would be nice to have mutliple return types from functions at some point.

## Classes, Inheritance and Prototypes

No.