<div align="center">
  <image src="toylogo.png" />
</div>

[![Running Comprehensive Tests](https://github.com/Ratstail91/Toy/actions/workflows/c-cpp.yml/badge.svg)](https://github.com/Ratstail91/Toy/actions/workflows/c-cpp.yml)

# Preamble

The Toy programming language is a procedural bytecode-intermediate interpreted language. It isn't intended to operate on its own, but rather as part of another program, the "host". This process is intended to allow a decent amount of easy customisation by the host's end user, by exposing logic in script files. Alternatively, binary files in a custom format can be used as well.

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

<hr />

* Simple C-like syntax
* Bytecode intermediate compilation
* Optional, but robust type system (including `opaque` for arbitrary data)
* Functions and types are first-class citizens
* Import external libraries
* Fancy slice notation for strings, arrays and dictionaries
* Can re-direct output, error and assertion failure messages
* Open source under the zlib license

# Getting Started

<hr />

* [Quick Start Guide](getting-started/quick-start-guide)
* [Types](getting-started/types)
* [About Library](getting-started/about-library)
* [Standard Library](getting-started/standard-library)
* [Random Library](getting-started/random-library)
* [Runner Library](getting-started/runner-library)
* [Game Engine](getting-started/game-engine)

# Deep Dive

<hr />

* [Theorizing Toy](deep-dive/theorizing-toy)
* [Building Toy](deep-dive/building-toy)
* [Embedding Toy](deep-dive/embedding-toy)
* [Compiling Toy](deep-dive/compiling-toy)
* [Developing Toy](deep-dive/developing-toy)
* [Testing Toy](deep-dive/testing-toy)
* [Roadmap](deep-dive/roadmap)

# Public C API

<hr />

* [repl_tools.h](c-api/repl_tools_h.md)
* [toy.h](c-api/toy_h.md)
* [toy_common.h](c-api/toy_common_h.md)
* [toy_compiler.h](c-api/toy_compiler_h.md)
* [toy_drive_system.h](c-api/toy_drive_system_h.md)
* [toy_interpreter.h](c-api/toy_interpreter_h.md)
* [toy_lexer.h](c-api/toy_lexer_h.md)
* [toy_literal_array.h](c-api/toy_literal_array_h.md)
* [toy_literal_dictionary.h](c-api/toy_literal_dictionary_h.md)
* [toy_literal.h](c-api/toy_literal_h.md)
* [toy_memory.h](c-api/toy_memory_h.md)
* [toy_parser.h](c-api/toy_parser_h.md)
* [toy_refstring.h](c-api/toy_refstring_h.md)
* [toy_scope.h](c-api/toy_scope_h.md)
