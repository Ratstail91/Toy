# Theorizing Toy

Sooner or later, every coder will try to create their own programming language. In my case, it took me over a decade and a half to realize that was an option, but once I did I read through a fantastic book called [Crafting Interpreters](https://craftinginterpreters.com/). This sent me down the rabbit hole, so to speak.

Now, several years later, after multiple massive revisions, I think my language is nearing version 1.0 - not bad for a side project.

The main driving idea behind the language has remained the same from the very beginning. I wanted a scripting language that could be embedded into a larger host program, which could be easily modified by the end user. Specifically, I wanted to enable easy modding for games made in an imaginary game engine.

At the time of writing, I've started working on said engine, building it around Toy and adjusting Toy to fit the engine as needed. I don't know how long the engine's development will take, but I personally think the best way to build an engine is to build a game first, and then extract the engine from it. Thus, the project is currently called "airport", though the engine proper will likely have a name like "box" or "toybox".

But this post isn't about the engine, it's about Toy - I want to explain, in some detail, my thought processes when developing it. Let's start at the beginning:

```
print "Hello world";
```

I've drawn the `print` keyword from Crafting Interpreter's Lox language, for much the same reason as explained in the book - it's a simple and easy way to debug issues - it's not intended for actual production use. You'll be able to print out any kind of value or variable from this statement - but it loses some context like function implementations and the values of `opaque` literals.

Let's touch on variables quickly - There's about a dozen variable types that can be used, depending on how you count them. This grew as the language progressed, and there are actually several literal types which you can't directly access (they're only used internally).

There's also functions (which are also a type of literal), which are reusable chunks of bytecode that can be invoked with their names. OH! I haven't even talked about bytecode yet - one of the interesting aspects of Toy is that the source code must be compiled into an intermediate bytecode format (a trait also inherited from Lox) before it can be executed by the interpreter. Even I'm not entirely sure how the internals of how the bytecode is layed out in it's entirety, as the parsing and compilation steps take liberties when producing the final usable chunk of memory.

I was originally not entirely certain that compiling to bytecode was the right choice as, for most programs to function and remain moddable, the source will need to be compiled on-the-fly. But after extensive benchmarking, it turns out that the compilation is the fastest part of execution.

There's one major feature of most programming languages that Toy is missing - input. Currently, there's no standardized way to receive input from the user - however this will likely be aleviated by various custom libraries down the road.

One such example would be a game controller library - something which takes in button presses, and calls certain Toy functions to move a character around the game world. The thing is, not every game will need a controller - that's why each library is optional, and can be provided or omitted at the host's discretion. As a result, Toy is almost infinitely extensible, as most good scripting languages are.

I don't know how well this langauge will do in the wild, once it gets some battle testing from actual users - but I do know that it'll become more and more of a grizzled beast as time goes on - that's inevitable for any piece of code. However, I would like to keep the core language nice and simple, as much as possible - something you can explain with just the quickstart page.

Feedback, and constructive criticism are always welcome.

