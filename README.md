*For the feature complete version 1, [click here](https://github.com/Ratstail91/Toy/tree/v1).*

<p align="center">
  <image src="toylogo.png" />
</p>

# Toy v2.x

The Toy programming language is an imperative, bytecode-interpreted, embeddable scripting language. Rather than functioning independently, it serves as part of another program, the "host". This design allows for straightforward customization by both the host’s developer and end users, achieved by exposing program logic through text files.

This repository holds the reference implementation for Toy version 2.x, written in C.

# Nifty Features

* Simple C-like syntax
* Intermediate AST and bytecode
* Strong, but optional type system
* First-class and higher-order functions
* Extensible via external libraries
* Can re-direct output, error and assertion failure messages
* Open source under the zlib license

## Syntax

*Omitted for review.*

## Building

Supported platforms are: `linux-latest`, `windows-latest`, `macos-latest`, using [GitHub's standard runners](https://docs.github.com/en/actions/using-github-hosted-runners/using-github-hosted-runners/about-github-hosted-runners#standard-github-hosted-runners-for-public-repositories).

To build the library, run `make source`.

To build the library and repl, run `make repl`.

To build and run the test cases, run `make tests`.

To build and run the test cases under gdb, run `make tests-gdb`.

## Tools

*Omitted for review.*

# License

This source code is covered by the zlib license (see [LICENSE](LICENSE)).

# Past and Current Contributors

@hiperiondev - Disassembler, porting support and feedback  
@add00 - Library support  
@gruelingpine185 - Unofficial MacOS support  
@solar-mist - Minor bugfixes  
Unnamed Individuals - Feedback  

# Patrons via Patreon

* Seth A. Robinson

Special thanks to Robert Nystrom of http://craftinginterpreters.com/ for their fantastic book that set me on this path.
