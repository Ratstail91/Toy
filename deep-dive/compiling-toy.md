# Compiling Toy

This tutorial is a sub-section of [Embedding Toy](deep-dive/embedding-toy) that has been spun off into it's own page for the sake of brevity/sanity. It's recommended that you read the main article first.

The exact phases outlined here are entirely implementation-dependent - that is, they aren't required, and are simply how the canonical version of Toy works.

## How the Compilation works

There are four main phases to running a Toy source file. These are:

```
lexing -> parsing -> compiling -> interpreting
```

Each phase has a dedicated set of functions and structures, and there are intermediate structures between these stages that carry the information from one set to another.

```
source   -> lexer       -> token
token    -> parser      -> AST
AST      -> compiler    -> bytecode
bytecode -> interpreter -> result
```

## Lexer

Exactly how the source code is loaded into memory is left up to the user, however once it's loaded, it can be bound to a `Lexer` structure.

```c
Toy_Lexer lexer;
Toy_initLexer(&lexer, source);
```

The lexer, when invoked, will break down the string of characters into individual `Tokens`.

The lexer does not need to be freed after use, however the source code does.

## Parser

The `Toy_Parser` structure takes a `Toy_Lexer` as an argument when initialized.

```c
Toy_Parser parser; 
Toy_initParser(&parser, &lexer);

Toy_ASTNode* node = Toy_scanParser(&parser);

Toy_freeParser(&parser);
```

The parser takes tokens, one at a time, and converts them into structures called Abstract Syntax Trees, or ASTs for short. Each AST represents a single top-level statement within the Toy script. You'll know when the parser is finished when `Toy_scanParser()` begins returning `NULL` pointers.

The AST Nodes produced by `Toy_scanParser()` must be freed manually, and the parser itself should not be used again.

## Compiler

The actual compilation phase has two steps - instruction writing and collation.

```c
size_t size;
Toy_Compiler compiler;

Toy_initCompiler(&compiler);
Toy_writeCompiler(&compiler, node);

unsigned char* tb = Toy_collateCompiler(&compiler, &size);

Toy_freeCompiler(&compiler);
```

The writing step is the process in which AST nodes are compressed into bytecode instructions, while literal values are extracted and placed aside in a cache (usually in an intermediate state).

The collation phase, however is when the bytecode instructions, along with the now flattened intermediate literals and function bodies are combined. The bytecode header specified in [Developing Toy](developing-toy) is placed at the beginning of this blob of bytes during this step.

The Toy bytecode (abbreviated to `tb`), along with the `size` variable indicating the size of the bytecode, are the result of the compilation.

This bytecode can be saved into a file for later consumption by the host at runtime - ensure that the file has the `.tb` extension.

The bytecode loaded in memory is consumed and freed by `Toy_runInterpreter()`.

## Interpreter

The interpreter acts based on the contents of the bytecode given to it.

```c
Toy_Interpreter interpreter;
Toy_initInterpreter(&interpreter);
Toy_runInterpreter(&interpreter, tb, size);
Toy_freeInterpreter(&interpreter);
```

Exactly how it accomplishes this task is up to it - as long as the result matches expectations.

## REPL

An example program, called `toyrepl`, is provided alongside Toy's core. This program can handle many things, such as loading, compiling and executing Toy scripts; it's capable of compiling any valid Toy program for later use, even those that rely on non-standard libraries.

To get a list of options, run `toyrepl -h`.

