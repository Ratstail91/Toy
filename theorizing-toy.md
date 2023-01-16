# Theorizing Toy

Sooner or later, every coder will try to create their own programming language. In my case, it took me over a decade and a half to realize that was an option, but once I did I read through a fantastic book called [Crafting Interpreters](https://craftinginterpreters.com/). This sent me down the rabbit hole, so to speak.

Now, several years later, after multiple massive revisions, I think my language is nearing version 1.0 - not bad for a side project.

The main driving idea behind the language has remained the same from the very beginning. I wanted a scripting language that could be embedded into a larger host program, which could be easily modified by the end user. Specifically, I wanted to enable easy modding for games made in an imaginary game engine.

At the time of writing, I've started working on said engine, building it around Toy and adjusting Toy to fit the engine as needed. I don't know how long the engine's development will take, but I personally think the best way to build an engine is to build a game first, and then extract the engine from it. Thus, the project is currently called "airport", though the engine proper will likely have a name like "toybox".

But this post isn't about the engine, it's about Toy - I want to explain, in some detail, my thought processes when developing it. Let's start at the beginning:

```
print "Hello world";
```

I've drawn the `print` keyword from Crafting Interpreter's Lox language, for much the same reason as explained in the book - it's a simple and easy way to debug issues - it's not intended for actual production use. You'll be able to print out any kind of value or variable from this statement - but it loses some context like function implementations and the values of `opaque` literals.

Let's touch on variables quickly - There's about a dozen variable types that can be used, depending on how you count them. This grew as the language progressed, and there are actually several literal types which you can't directly access (they're only used internally).

There's also functions (which are also a type of literal), which are reusable chunks of bytecode that can be invoked with their names. OH! I haven't even talked about bytecode yet - one of the interesting aspects of Toy is that the source code must be compiled into an intermediate bytecode format (a trait also inherited from Lox) before it can be executed by the interpreter. Even I'm not entirely sure how the internals of the bytecode is layed out in it's entirety, as the parsing and compilation steps take liberties when producing the final usable chunk of memory.

I was originally not entirely certain that compiling to bytecode was the right choice as, for most programs to function and remain moddable, the source will need to be compiled on-the-fly. But after extensive benchmarking, it turns out that the compilation is the fastest part of execution.

There's one major feature of most programming languages that Toy is missing - input! Currently, there's no standardized way to receive input from the user - however this will likely be aleviated by various custom libraries down the road.

One such example would be a game controller library - something which takes in button presses, and calls certain Toy functions to move a character around the game world. The thing is, not every game will need a controller - that's why each library is optional, and can be provided or omitted at the host's discretion. As a result, Toy is almost infinitely extensible, as most good scripting languages are.

There's one unusual (and possibly unique) feature of function names - if they begin with an underscore, then you can use it with the "dot" operator, as so:

```
var arr = [1, 2, 3]; //declare an array

print arr.length(); //equivilent to _length(arr);
```

By replacing the underscore with the dot, you can essentially pass the left-hand variable in as the first argument - this feature was included for stylistic reasons, as during the development I got really sick of doing just that in C. The intent was twofold: to make the language more familiar for people, and to differentiate functions intended for this from others.

The language doesn't have a class system, but it does have compounds types - arrays, dictionaries and strings (the last of which is a bit iffy in this category). These variable types contain other variables - arrays store multiple values in sequence within memory, and dictionaries are key-value hashes. Strings actually use a custom string-system called "refstrings", which reduces the amount of needless memory copying.

By combining compounds with some globally available underscore functions, you can have some really nice (and familiar) looking code.

Something that isn't covered in the quick-start guide is just how complex types are - for instance, types are first-class citizens. What this means is that they are considered values, just like `0`, `true`, and `"Hello world!"`. As such, they can be stored in variables, and can even represent complex compound structures.

```
var t = astype [int]; //t is a type, representing an array of integers

var arr: t = [1, 2, 3]; //the actual array, with "t" as it's type

var complex = astype [string: [int]]; //complex dictionary of named integer arrays

var dict: complex = [
	"foo": [1, 2, 3]
];
```

The `astype` keyword is a hint to the parser that the following value should be interpreted as a type, since the simple syntax of Toy could otherwise be ambiguous. There were a few issues like this that cropped up, but none quite as obvious as that one.

[To Be Continued]

