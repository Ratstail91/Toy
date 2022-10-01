# Quick Start Guide

This guide is intended to get you writing Toy code as fast as possible. As such, it's more of a reference for experienced coders to know what is available and what isn't.

Toy programs begin at the top of the file, and continue until the end, unless an error is encountered.

## Hello World

This prints to the stdout, and has a newline appended to the end. This can be altered by the host program.

```
print "Hello world";
```

## Names and Variables

Variables can store data of any kind, unless a type is specified; see [types](types). Names can be up to 256 characters long.

```
var b = true;
var i = 42;
var f = 3.14;
var s = "Hello world";
```

## Compounds

Larger containers of data are available - arrays and dictionaries. Arrays are collections of data stored sequentially, while dictionaries are hash-maps of key-value pairs:

```
var array = []; //define an array
var dict = [:]; //define a a dictionary

dict["foo"] = "bar"; //you can use indexing to add to a dictionary
array.push(42); //you must use a function to push to an array
```

## Control Flow

You can control the program flow with either `if`, `while` or `for`. The only falsy value is `false`.

```
if (check()) {
    //do this
}
else {
    //otherwise do this
}

var i = 0;
while (i < 10) {
    print i++;
}

for (var i = 0; i < 10; i++) {
    print i;
}
```

`continue` and `break` both behave as you'd expect.

## Functions

Functions are defined with the `fn` keyword, and can take any number of arguments. They can return a value as well.

```
fn combine(a, b, c) {
    return [a, b, c];
}

print combine(1, 2, 3);
```

Variable number of parameters, called rest parameters, can be passed in as an array.

```
fn combine(...rest) {
    return rest;
}

print combine(1, 2, 3);
```

## Dot Notation and Global Functions

Any function that begins with an underscore can be called using the dot notation, which is syntactic sugar:

```
fn _printMe(self) {
    print self;
}

array.printMe();
```

There are several underscore functions provided by default:

```
_index(self, first, second, third, assign, op) //don't use this
_set(self, key, value)                         //array, dictionary
_get(self, key)                                //array, dictionary
_push(self, value)                             //array
_pop(self)                                     //array
_length(self)                                  //array, dictionary, string
_clear(self)                                   //array, dictionary
```

The `_index` function is simply used for slice notation, so it's recommended that you don't use it or overwrite it.

## Slice Notation

When indexing a compound value, you can use slice notation to manipulate it's elements:

```
var greeting = "Hello world";

print greeting[::-1]; //dlrow olleH

greeting[0:4] = "Goodnight"; //changes greeting to equal "Goodnight world"
```

## Import and Export

The interpreter has a set of variables referred to collectively as the "exports region" - these are intended for interfacing with the host. To access these from the scripts, use `import`, like so:

```
import variable;

print variable; //prints whatever literal was given "variable" as the identifier
```

Alternatively, to add something to the exports region, use `export`:

```
var variable = 1;

export variable;
```

In the event of naming conflicts, you can rename imported and exported variables using the `as` keyword:

```
//assume "table" exists in the export region
import table as newName;

export newName as table2;
```

## External Libraries

The host may, at it's own discretion, make external libraries available to the scripts. To access these, you can use the `import` keyword once again:

```
import standard;

print clock();
```

## Assertion Tests

For testing purposes, there is the `assert` keyword. `assert` takes two arguments, separated by a comma; if the first resolves to a truthy value, then the whole statement is a no-op. Otherwise, the second argument, which MUST be a string, is displayed as an error and the script exits.

```
var answer = 42;

assert answer == 42, "This will not be seen";

//both false and null trigger assert's exit condition
assert null, "This will be seen before the script exits";
```
