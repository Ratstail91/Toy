<p align="center">
  <image src="toylogo.png" />
</p>

# Preamble

The Toy programming language is a procedural bytecode-intermediate interpreted language. It isn't intended to operate on it's own, but rather as part of another program, the "host". This process is intended to allow a decent amount of easy customisation by the host's end user, by exposing logic in script files. Alternatively, binary files in a custom format can be used as well.

The host will provide all of the extensions needed on a case-by-case basis. Script files have the `.toy` file extension, while binary files have the `.tb` file extension.

```
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

# Nifty Features

* Simple C-like syntax
* Bytecode intermediate compilation
* Optional, but robust type system
* functions and types are first-class citizens
* `import` and `export` variables from the host program
* Fancy slice notation for strings, arrays and dictionaries
* Can re-direct output, error and assertion failure messages
* Open source under the zlib license

# Getting Started

* [Quick Start Guide](quick-start-guide)
* Tutorials
  * [Embedding Toy](embedding-toy)
  * [Compiling Toy](compiling-toy)
  * [Using Toy](using-toy)
  * ~~[Standard Libary](standard-library)~~
  * [Types](types)
* [Developing Toy](developing-toy)
* [Roadmap](roadmap)
* ~~[Game Engine](game-engine)~~

# Version Differences

There have been a number of versions of Toy over the years, the current actively developed version is called `0.6.0` for the time being. It is recommended that you use the most recent version available.
