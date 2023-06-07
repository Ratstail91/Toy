<p align="center">
  <image src="toylogo.png" />
</p>

# Toy

The Toy programming language is an imperative bytecode-intermediate embedded scripting language. It isn't intended to operate on its own, but rather as part of another program, the "host". This process is intended to allow a decent amount of easy customisation by the host's end user, by exposing logic in script files. Alternatively, binary files in a custom format can be used as well.

The host will provide all of the extensions needed on a case-by-case basis. Script files have the `.toy` file extension, while binary files have the `.tb` file extension.

This is the Toy programming language interpreter, written in C.

# Nifty Features

* Simple C-like syntax
* Bytecode intermediate compilation
* Optional, but robust type system (including `opaque` for arbitrary data)
* Functions and types are first-class citizens
* Import native libraries from the host
* Fancy slice notation for strings, arrays and dictionaries
* Can re-direct output, error and assertion failure messages
* Open source under the zlib license

## Building

For Windows(mingw32 & cygwin), Linux and MacOS, simply run `make` in the root directory.

For Windows(MSVC), Visual Studio project files are included.

Note: MacOS and Windows(MSVC) are not officially supported, but we'll do our best!

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
```

# License

This source code is covered by the zlib license (see [LICENSE.md](LICENSE.md)).

# Patrons via Patreon

* Seth A. Robinson

Special thanks to http://craftinginterpreters.com/ for their fantastic book that set me on this path.