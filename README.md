<p align="center">
  <image src="toylogo.png" />
</p>

# Toy

This is the Toy programming language interpreter, written in C.

Special thanks to http://craftinginterpreters.com/ for their fantastic book that set me on this path.

# Nifty Features

* Simple C-like syntax
* Bytecode intermediate compilation
* Optional, but robust type system (including `opaque` for arbitrary data)
* Functions and types are first-class citizens
* Import external libraries
* Fancy slice notation for strings, arrays and dictionaries
* Can re-direct output, error and assertion failure messages
* Open source under the zlib license

## Building

For Windows(mingw32 & cygwin), Linux and MacOS, simply run `make` in the root directory.

Note: MacOS is not officially supported (no machines for testing), but we'll do our best!

## Tools

Run `make install-tools` to install a number of tools, including:

* VSCode syntax highlighting

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

This source code is covered by the zlib license (see [LICENSE.md](LICENSE.md)).

# Patrons via Patreon

* Seth A. Robinson

