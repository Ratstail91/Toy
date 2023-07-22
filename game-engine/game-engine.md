# Preface

The game engine is incomplete and still evolving, as such this page should be considered outdated at all times...

# Game Engine

The Toy programming langauge was designed from the beginning as an embedded scripting language for some kind of game engine. Different iterations have existed with different implementations, some of which could charitably be said to function. The current version, and the most stable and feature complete so far, has reached a point where it needs some kind of concrete engine to improve any further.

Currently, the engine exists under the `Box/` directory within the "airport" repository, and builds as part of that program - this will be packaged as a separate repo soon.

[https://github.com/Ratstail91/airport](https://github.com/Ratstail91/airport)

NOTE: This documentation page is a rushjob, and will be expanded into it's own subsection eventually.

## Engine Structure

The engine is invoked with just three lifecycle functions called directly from `main()`:

```c
#define SDL_MAIN_HANDLED

#include "box_engine.h"

int main(int argc, char* argv[]) {
	//the real main() has some extra junk, but this is all that's needed for the engine
	Box_initEngine();
	Box_execEngine();
	Box_freeEngine();
	return 0;
}
```

The engine proper holds the following elements:

* SDL2 window and renderer
* Framerate controls
* Toy interpreter
* Input keyboard mapping dictionaries
* The root node

Input and rendering in the game engine are handled by SDL2 - this allows for the game to, in theory, be built on multiple platforms. For the time being, however, we're just targeting Windows due to a lack of testing machines.

The engine is calibrated to run at 60 FPS - if the "simulation time" falls too far behind the real time, then frame rendering is skipped in favour of speeding up the game logic. Likewise, if the simulation time runs too fast, then the simulation step is skipped instead and an extra frame is drawn. This process can lead to multiple skips in a row, in both directions.

The Toy interpreter is the core of the scripting system. Other interpreters may be generated and cleared during Toy's internal processes, but this one lasts for the duration of the program.

The keyboard inputs can be remapped using API functions - these mappings are stored and accessed within a pair of Toy dictionaries that have simply been embedded directly into the engine for easy access.

The nodes form a deep tree-like structure, with the "root node" at it's base.

## Node Structure

The fundemental building block of the engine's logic is the node - nodes can represent anything within the game world, from entities to abstract global systems. You can think of entities as having a 1:1 mapping to Toy scripts, as each one is given bytecode on initialization that fills its internals.

A node holds the following elements:

* A reference to it's parent
* An array of references to its children, and bookkeeping variables for tracking them
* A dictionary of functions defined in the Toy script
* A single SDL texture reference, and controls for rendering it (including as an animated spritesheet)

The nodes are deeply integrated with Toy scripts, while Toy was written specifically for this purpose. The tree-like structure of the nodes all exist entirely within the computer's heap memory as a result of Toy's memory model - this comes with performance drawbacks and cleanup requirements. Child nodes should never be referenced directly, as they may be `NULL` references that have been released - the internal array doesn't shrink, and the tombstones don't get reused.

The rules of execution for scripts and functions is as follows:

* The script is executed during node initialization
* All functions (regardless of name) are stored within the node - effectively preserving the scope of the script as a whole
* These functions can now be invoked from elsewhere in the program
* Certian function names (listed below) are invoked at specific times during the game loop throughout the entire node tree
* If the specially named functions do not exist, the node is simply skipped
* Every function, which is intended to be called through `callNodeFn()` or at specific times in the loop must take the `opaque` node as its first argument

## Special Function Names

The following functions, which are defined within the node scripts, are invoked at specific times within the game loop. Note that if you want code to execute *during* node creation, place it within the script's root scope. Variables that you want to persist between calls should also be placed in the script's root.

* `onLoad(node: opaque)`
* `onInit(node: opaque)`
* `onStep(node: opaque)`
* `onFree(node: opaque)`
* `onDraw(node: opaque)`
* `onKeyDown(node: opaque, event: string)`
* `onKeyUp(node: opaque, event: string)`
* `onMouseMotion(node: opaque, x: int, y: int, xrel: int, yrel: int)`
* `onMouseButtonDown(node: opaque, x: int, y: int, button: string)`
* `onMouseButtonUp(node: opaque, x: int, y: int, button: string)`
* `onMouseWheel(node: opaque, xrel: int, yrel: int)`

These may change or expand as more input devices are added, and the engine matures.

NOTE: `onLoad()` is invoked every time a node is loaded - but `onInit()` is only invoked once by the engine. After that, `initNode()` must be called manually on any node children that are loaded later.

# Engine Libraries

A series of libraries are provided to allow Toy to interface and control the engine. In addition, the libraries stored within Toy's `repl/` directory are also available (see the main page for the list).

During startup, the script named `init.toy` in the root of the scripts directory is executed. This file can be used to configure input mappings, as well as initializing the window and node tree.

TODO: Complete the rest of this page

# Engine API

The engine API is for controlling the core of the engine directly.

## initWindow(caption: string, width: int, height: int, fullscreen: bool)

This function initializes the game's window, as well as specifying it's dimensions and properties. There is only one window - calling this multiple times is a fatal error. Also, the game's loop will simply not run if the window hasn't been initialized.

## loadRootNode(fname: string)

TODO

## getRootNode(): opaque

TODO

# Node API

The node API is for controlling individual nodes.

TODO

# Input API

The input API is for mapping different interface devices to different "event" names - currently only the keyboard inputs can be mapped this way, but more devices may be added in the future - thus, the input API was separated into a separate library from the engine proper.

TODO

# Box C API

TODO