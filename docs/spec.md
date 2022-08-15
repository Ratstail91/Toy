# Preamble

The Toy Programming Language is an experiment by yours truly to try and make a simple general purpose programming language. I've tried several times in the past to develop it, with varying levels of success. I'm currently writing this specification to hash out the basics of a potential version 0.6.0, which would be developed as a scripting language of some sort. I'm also marking this as RFC, since I want some feedback from the community - this post will be updated and developed as time progresses.

# Premise

The Toy programming language isn't intended to operate on it's own, but rather as part of another program (the "host"). This process should be able to allow a decent amount of easy customisation by the end user, by exposing logic in script files. Alternatively, binary files in a custom format can be used as well.

The host will provide all of the extensions needed on a case-by-case basis. Script files have the `.toy` file extension, while binary files have the `.tb` file extension.

# Definition And Syntax

## Comments

Toy supports two types of comments, `//single line` and `/* block */`. Comments are used to leave notes for yourself in the code; they are ignored by the parser.

## Naming

Names used in the language must start with a letter or underscore, and must contain only letters, numbers and underscores. They can also not exceed 256 characters in length.

## List of Reserved Keywords

The following list of keywords cannot be used as names, due to their significance (or potential later use) in the language.

* any
* array
* as
* assert
* bool
* break
* class
* const
* continue
* dictionary
* do
* else
* export
* false
* float
* fn
* for
* foreach
* if
* import
* in
* int
* null
* of
* print
* return
* string
* true
* type
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

Other operators used throughout the language are: the assignment, colon, semicolon, comma, dot, pipe, rest operators:

```
= : ; , . | ...
```

## Types

Variable names in Toy may have a type. These types are:

* null - no value assigned
* boolean - either `true` or `false`
* integer - any whole number
* float - any floating point number
* string - a string of characters enclosed in double quotation marks
* array - a collection of 0-indexed variables
* dictionary - a collection of indexable key-value pairs
* function - chunks of reusable code
* any - any of the above

Types are optional attachments to names - they can be specified as such:

```
//declare the variable named "x" as an integer with the initial value 0
var x : int = 0;
```

Types are not interoperable, but in some cases they can be converted from one type to another by "casting":

```
//declare the variable named "y" as a float, and assign it the value stored in x
var y : float = (float)x;
```

defining the type of a variable is not required - in such a case, the type is "any".

```
//define the variable named "v" with any type
var z = 1;
```

## Const

The "const" keyword can be appended to the type of a variable to fix the value in place - constants, as they're known, can't be changed once they are given a value at declaration.

```
var foo: int const = 42;

foo++; //Error!
```

Otherwise, constants act just like normal variables.

## Truthyness

Everything is considered "truthy" except:

* the value `null`
* the value `false`
* the integer and float value `0`

## Print

the "print" keyword is used largely for debugging. It takes a single value, and by default prints to stdout (appending a newline), however this can be overwritten by the host program for a variety of reasons. It can also take variables as values.

```
print "Hello world";
```

## Var

To declare a variable, use the keyword `var`, followed by it's name, and an optional colon and type:

```
var foo: int = 42;

var bar = "hello world";

//for "var", a value is not required
var buzz;
```

Variables can be used in place of values at any time, and can be altered and re-assigned. Multiple variables (which have been previously declared) can be assigned to the same value:

```
a = b = c = 1;
```

## If-Else

The keywords `if` and `else` are used to determine different paths of execution that the program can take, based on a condition. If the condition is truthy, then the `if`-path executes, otherwise the `else`-path does. The else keyword and path are optional.

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
	if (i >= 7) {
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

Functions can take a number of parameters within the parenthesis following their names, and can return a number of values to the calling context using the return keyword.

```
fn reverseOrder(a, b, c) { //return type isn't required by default
	return c, b, a;
}
```

You can define the required return types using a colon after the parameter list, followed by a comma separated list of required returns.

```
fn reverseIntegers(a: int, b: int, c: int): int, int, int {
	return c, b, a; //must return three integers
}
```

Functions are called by invoking their names, followed by the parenthesis-enclosed list of arguments. When returning multiple values, the target variables are assigned from left to right until there is a mismatched number of variables or values (remaining variables become null).

```
simple();

var a, b, c = reverseOrder(1, 2, 3);

var d, e, f = reverseIntegers(4, 5, 6);

var g = reverseIntegers(7, 8, 9); //only 9 is returned
```

Functions are first-class citizens, meaning they can be declared and treated like variables. Closures are explicitly supported.

```
fn counterFactory(): fn()(int) { //returns a function with 0 parameters and 1 integer return
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

fn omitFirstInteger(arg1: int, ...rest: [int]) {
	//rest comes in as an array of integers
	return rest;
}
```

## Assert

The `assert` keyword takes 2 parameters, separated by a comma. If the first parameter resolves to be falsy, then the program terminates, and the value of the second parameter is displayed to the user as an error. By default, the error is printed to stderr, but this can be overwritten by the host program.

```
assert true, "This is fine"; //Good!
assert false, "This is not"; //Error!
```

## Import-As

`import` is used to load variables from the host - several optional libraries will be provided this way, as well. The import keyword can only take a string as it's argument, followed by an optional "as" which stores the results under a different name.

```
import "standard";

print standard.clock(); //the clock function is provided by standard

import "standard" as std;

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

Strings are a series of characters, and are created by surrounding said characters with a pair of double quotation marks `"`. Strings cannot exceed 4096 bytes in length.

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

var arr: [int] const = [1, 2, 3]; //arr is constant, but it's members are not (the members can be altered)

var arr: [int const] const = [1, 2, 3]; //both arr and it's members are constant
```

Arrays can be indexed using traditional bracket notation.

## Dictionary

Dictionaries are key-value collections of variables. Every key has a single associated value; any missing value is considered `null`. `null` is an invalid key type. Nesting dictionaries within themselves is not allowed.

```
var dict: [string, string] = ["key one": "value one", "key two": "value two"];

print dict["key one"]; //value one

var dictTwo: [int, int] = [:]; //empty dictionary
```

Keys, values and dictionaries can be declared const, like so:

```
var dict: [string const, int const] const = [
	"one": 1,
	"two": 2,
	"three": 3
];
```

Dictionaries can be indexed using traditional bracket notation, or the dot operator when the keys are strings. Existing elements can be accessed or overwritten, or new ones inserted if they don't already exist this way by using the standard library functions.

```
dict["foo"] = "bar";
print dict["foo"];
print dict.foo; //syntactic sugar, only works if the key is not a built-in function
```

## Indexing, Slice and Dot Notation

Strings, arrays and dictionaries can be indexed in several ways, via the global `_get` and `_set` functions. Elements can be accessed using traditional bracket notation:

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

### _get() and _set()

The slice and dot notations (the latter of which only works on dictionaries) are simply syntactic sugar for the global `_get` and `_set` functions. These functions take a number of arguments, which correlate to the slice and dot notations:

```
fn _get(self, first, second, third) {
	//native code
}

fn _set(self, first, second, third, value) {
	//native code
}
```

These functions can be overwritten.

## Standard Library

The standard library has a number of utility functions available, and is provided by default.

```
//gain access to the standard functions
import "standard";
```

The following functions are available in the standard library.

* clear(self: any) - This function removes the contents of the array, dictionary or string, leaving an empty array. This alters the memory.
* clock() - This function returns the number of seconds since January 1st, 1970.
* concat(self: any, x: any): any - This function requires an array or dictionary with a matching type as "x". This function returns a new dictionary instance which contains the contents of the current array or dictionary combined with the contents of "x". In the event of a dictionary key clash, the key-value pair in the current dictionary is included, and the key-value pair from "x" is discarded.
* containsKey(self: [any, any], k: any): bool - This function returns true if the dictionary contains a key "k", otherwise it returns false.
* containsValue(self: any, v: any): bool - This function returns true if the array or dictionary contains a value "v", otherwise it returns false.
* copy(self: any): any - This function returns a deep copy of the array, dictionary or string.
* equals(self: any, x: any): bool - This returns true if the two values are equal - this does a deep type and value check on arrays and dictionaries.
* every(self: any, cb: fn(k: any, v: any)(bool)): bool - This function calls "cb" once for every entry in the array or dictionary (self), with that element passed in as "k" and "v", until cb returns false, at which point "every" returns false. If it reaches the end of the array or dictionary, then "every" returns true.
* filter(self: any, cb: fn(k: any, v: any)(bool)): any - This function calls "cb" once for every entry in the array or dictionary (self), with that key and value passed in as "k" and "v", respectfully. This returns a new array or dictionary that contains every key-value pair for which the call to "cb" returned true.
* indexOf(self: string, str: string): int - This function returns the position of the first instance of "str" in the string "self".
* insert(self: any, key: any, x: any) - This function inserts "value" at the index/key "key", shifting the remaining entry up 1 index if it's an array. This alters the memory.
* keys(self: any): [type] - This function returns an array containing each key in the dictionary. The order of the keys is undefined.
* lastIndexOf(self: string, str: string): int - This function returns the position of the last instance of "str" in the calling string "self".
* length(self: any): int - This function returns the length of the array, dictionary or string.
* map(self: any, cb: fn(k: any, v: any)(any)): any - This function calls "cb" once for every entry in the array or dictionary, with that key passed in as "k" and value passed in as "v". It returns a new array or dictionary with the keys copied from the current "self", and values replaced with the results of calls to "cb".
* pop(self: [any]): any - This function deletes the value at the end of the array, and returns that value.
* push(self: [any], x: type) - This function inserts the value of "x" at the end of the array.
* reduce(self: any, default: any, cb: fn(acc: any, k: any, v: any): any): any - This function calls "cb" once for every element in the array or dictionary "self", with that element passed in as "k" and "v", and the value of the previous call passed in as "acc". For the first call to "cb", "default" is used for "acc". The final value of "acc" is returned by "reduce".
* remove(self: any, k: any) - This function deletes the value at the index or key "k", shifting the remaining entries down 1 index if it's an array. This alters the memory.
* replace(self: string, pat: string, rep: string): string - For each instance of "pat" that it finds in the calling string "self", it replaces it with "rep", then returns the new string.
* reverse(self: any): any - This function returns the original array or string, except the entries or characters are reversed. The calling object is not modified.
* shift(self: [any]): any - This function deletes the value at the beginning of the array, and returns that value.
* some(self: [any, any], cb: fn(k: any, v: any)(bool)): bool - This function calls "cb" once for every entry in the array or dictionary (self), with that element passed in as "key" and "value", until cb returns true, at which point "some" returns true. If it reaches the end of the array or dictionary, then "some" returns false.
* sort(self: [any], cb: fn(lhs: any, rhs: any)(int)): [any] - This function sorts the entries of an array according to the callback "cb". "cb" may be called any number of times during the sorting process. "cb" must be a function that takes two parameters, "lhs" and "rhs" and returns an integer. If "lhs" is less than "rhs", then the returned integer should be negative. If they are equal, then it should be 0. Otherwise it should be positive. This returns the sorted array, leaving the original intact.
* toLower(self: string): string - This function returns a new string, which is the same as the calling string, except all characters are lower case.
* toString(self: any): string - This function returns a string representation of the input "self". For arrays and dictionaries, each element is converted to a string where possible, and separated by commas (and colons for dictionaries). Finally, the whole string is surrounded by brackets.
* toUpper(self: string): string - This function returns a new string, which is the same as the calling string, except all characters are upper case.
* toValue(self: string): any - This function tries to convert the string representation of a literal value to it's original type, while retaining its value. If it fails, it simply returns null. The original string is not modified.
* trim(self: string, chars: string): string - Every character in the string "chars" is removed from the calling string's beginning and end, and the new string is returned. The original string is not modified.
* unshift(self: [any], x: any) - This function inserts the value of "x" at the beginning of the array.
* values(self: [any, any]): [any] - This function returns an array containing each value in the dictionary. The order of the values is undefined.

# For Consideration

The following features are under consideration, but will not be explicitly planned for during development.

## Foreach-Of and Foreach-In

These could be used to iterate over the keys and values of dictionaries, respectfully. They could also be applied to arrays and strings.

## Classes, Inheritance and Prototypes

No.