#pragma once

/* toy.h - A Toy Programming Language

If you're looking how to use Toy directly, try https://toylang.com/
Otherwise, these headers may help learn how Toy works internally.

*/

/* utilities - these define a bunch of useful macros based on platform.

The most important one is `TOY_API`, which highlights functions intended for the end user.

*/

#include "toy_common.h"
#include "toy_console_colors.h"
#include "toy_memory.h"
#include "toy_drive_system.h"

/* core pipeline - from source to execution

Each step is as follows:

source   -> lexer       -> token
token    -> parser      -> AST
AST      -> compiler    -> bytecode
bytecode -> interpreter -> result

I should note that the parser -> compiler phase is actually made up of two steps - the write step
and the collate step. See `Toy_compileString()` in `repl/repl_tools.c` for an example of how to compile
properly.

*/

#include "toy_lexer.h"
#include "toy_parser.h"
#include "toy_compiler.h"
#include "toy_interpreter.h"

/* building block structures - the basic units of operation

Literals represent any value within the language, including some internal ones that you never see.
Literal Arrays are literally arrays within memory, and are the most heavily used structure in Toy.
Literal Dictionaries are unordered key-value hashmaps, that use a running strategy for collisions.

*/

#include "toy_literal.h"
#include "toy_literal_array.h"
#include "toy_literal_dictionary.h"

/* other components - you probably won't use these directly, but they're a good learning opportunity.

`Toy_Scope` holds the variables of a specific scope within Toy - be it a script, a function, a block, etc.
Scopes are also where the type system lives at runtime. They use identifier literals as keys, exclusively.

`Toy_RefString` is a utility class that wraps traditional C strings, making them less memory intensive and
faster to copy and move. In reality, since strings are considered immutable, multiple variables can point
to the same string to save memory, and you can just create a new one of these vars pointing to the original
rather than copying entirely for a speed boost. This module has it's own memory allocator system that is
plugged into the main memory allocator.

`Toy_RefFunction` acts similarly to `Toy_RefString`, but instead operates on function bytecode.

*/

#include "toy_scope.h"
#include "toy_refstring.h"
#include "toy_reffunction.h"
