# Quick Start Guide

This guide is intended to get you writing Toy code as fast as possible. As such, it's more of a reference for experienced coders to know what is available and what isn't.

Toy programs begin at the top of the file, and continue until the end, unless an error is encountered.

## Hello World

This prints to the stdout, and has a newline appended to the end. This can be altered by the host program.

```
print "Hello world";
```

## Names and Variables

Variables can store data of any kind, unless a type is specified. See [types](types). Names can be up to 256 characters long.

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
_set(self, key, value)
_get(self, key)
_push(self, value)
_pop(self)
_length(self)
_clear(self)
```

The `_index` function is simply used for slice notation, so it's recommended that you don't use it or overwrite it.

## Slice Notation

When indexing a compound value, you can use slice notation to manipulate it's elements:

```
var greeting = "Hello world";

print greeting[::-1]; //dlrow olleH

greeting[0:4] = "Goodnight"; //changes greeting to equal "Goodnight world"
```

