<p align="center">
  <image src="toylogo.png" />
</p>

# Toy 0.6.0

This is the Toy programming language interpreter, written in C.

Special thanks to http://craftinginterpreters.com/ for their fantastic book that set me on this path.

## Nifty Features

* Simple C-like syntax
* Bytecode intermediate compilation
* `import` and `export` variables from the host program
* Optional, but robust type system
* functions and types are first-class citizens
* Fancy slice notation for strings, arrays and dictionaries (`print greeting[0:4:-1]; //prints "olleh"`)

## Building

For Windows and MacOS, simply run `make` in the root directory.

For Linux, run `make repl-static` in the root directory (see [this issue for details](https://github.com/Ratstail91/Toy/issues/26)).

Note: MacOS is not officially supported (no machines for testing), but we'll do our best!

## Syntax

```
import standard; //for a bunch of utility functions


print "Hello world"; //"print" is a keyword

var msg = "foobar"; //declare a variable like this

assert true, "This message won't be seen"; //assert is another keyword

//-------------------------

fn makeCounter() { //declare a function like this
	var total: int = 0; //declare a variable with a type like this

	fn counter(): int { //declare a return type like this
		return ++total;
	}

	return counter; //closures are explicitly supported
}

var tally = makeCounter();

print tally(); //1
print tally(); //2
print tally(); //3

export tally; //export this variable to the host program
```

# License

Copyright (c) 2020-2022 Kayne Ruse, KR Game Studios

This software is provided 'as-is', without any express or implied warranty. In no event will the authors be held liable for any damages arising from the use of this software.

Permission is granted to anyone to use this software for any purpose, including commercial applications, and to alter it and redistribute it freely, subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.

2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.

3. This notice may not be removed or altered from any source distribution.
