# Game Engine

The Toy programming langauge was designed from the beginning as an embedded scripting language for some kind of game engine. Different iterations have existed with different implementations, some of which could charitably be said to function. The current version, and the most stable and feature complete so far, has reached a point where it needs some kind of concrete engine to improve any further.

The best way to create a game engine is to build a game first, then extract the engine from the finished product. Therefore, a game is being built, currently under the codename "airport", around the Toy langauge.

[https://github.com/Ratstail91/airport](https://github.com/Ratstail91/airport)

This game has a simple design (it's an idle clicker), but should support enough features to make the engine worthwhile.

More details, such as a name for the engine, will come eventually.

## Engine Structure

The engine uses a node-based structure, inspired by Godot's own node structure. Each node has a script attached, and can programmatically load other nodes.

The engine also uses Toy's memory model i.e. `Toy_reallocate()` wrapped in a number of macros.

