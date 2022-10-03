# Roadmap

There's a few things I'd like to do with the langauge, namely bugfixes, new features and implementing a game with it.

## Game And Game Engine

The Toy programming langauge was designed from the beginning as though it was supposed to be embedded into an imaginary game engine. Well, now that the lang is nearly feature complete, it's time to start on that engine.

To that end, I've begun working on this: [Airport Game](https://github.com/Ratstail91/airport).

This is a simple game concept, which I can implement within a reasonable amount of time, before extracting parts to create the engine proper. It feels almost like a mobile game, so I'm hoping this engine will be runnable on android (though at the time of writing, I've yet to investigate how).

## Bugfixes

This is probably the easiest goal to accomplish, but also the least urgent. The [issue tracker](https://github.com/Ratstail91/Toy/issues) provides a list of known bugs and issues that need to be addressed, but for the time being, my attention is focused elsewhere.

## New Features

Some things I'd like to add in the future include:

* A fully featured standard library
* An external script runner utility
* Multiple return values from functions
* Ternary operator

These have always been planned, but were sidelined or are incomplete for one reason or another.

## Nope Features

Some things I simply don't want to include at the current time include:

* Classes & Structures
* Do-while loops

This is because reworking the internals to add an entirely new system like this would be incredibly difficult for very little gain.

Ironically, I've never used a do-while loop seriously until I started implementing this language.


