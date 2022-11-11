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

* A fully featured standard library (see below)
* An external script runner utility library
* A threading library
* A random generation library (numbers, perlin noise, wave function collapse?)
* A timer library (under development)
* Multiple return values from functions
* Ternary operator
* interpolated strings

Some of these have always been planned, but were sidelined or are incomplete for one reason or another.

## Planned Standard Library Functions

This is just random ideas for now, dumped here from another file.

* _concat(self: any, x: any): any - This function requires an array or dictionary with a matching type as "x". This function returns a new dictionary instance which contains the contents of the current array or dictionary combined with the contents of "x". In the event of a dictionary key clash, the key-value pair in the current dictionary is included, and the key-value pair from "x" is discarded.
* _containsKey(self: [any : any], k: any): bool - This function returns true if the dictionary contains a key "k", otherwise it returns false.
* _containsValue(self: any, v: any): bool - This function returns true if the array or dictionary contains a value "v", otherwise it returns false.
* _every(self: any, cb: fn(k: any, v: any): bool): bool - This function calls "cb" once for every entry in the array or dictionary (self), with that element passed in as "k" and "v", until cb returns false, at which point "every" returns false. If it reaches the end of the array or dictionary, then "every" returns true.
* _filter(self: any, cb: fn(k: any, v: any)(bool)): any - This function calls "cb" once for every entry in the array or dictionary (self), with that key and value passed in as "k" and "v", respectfully. This returns a new array or dictionary that contains every key-value pair for which the call to "cb" returned true.
* _indexOf(self: string, str: string): int - This function returns the position of the first instance of "str" in the string "self".
* _insert(self: any, key: any, x: any) - This function inserts "value" at the index/key "key", shifting the remaining entry up 1 index if it's an array. This alters the memory.
* _keys(self: any): [type] - This function returns an array containing each key in the dictionary. The order of the keys is undefined.
* _map(self: any, cb: fn(k: any, v: any): any): any - This function calls "cb" once for every entry in the array or dictionary, with that key passed in as "k" and value passed in as "v". It returns a new array or dictionary with the keys copied from the current "self", and values replaced with the results of calls to "cb".
* _reduce(self: any, default: any, cb: fn(acc: any, k: any, v: any): any): any - This function calls "cb" once for every element in the array or dictionary "self", with that element passed in as "k" and "v", and the value of the previous call passed in as "acc". For the first call to "cb", "default" is used for "acc". The final value of "acc" is returned by "reduce".
* _remove(self: any, k: any) - This function deletes the value at the index or key "k", shifting the remaining entries down 1 index if it's an array. This alters the memory.
* _replace(self: string, pat: string, rep: string): string - For each instance of "pat" that it finds in the calling string "self", it replaces it with "rep", then returns the new string.
* _some(self: any, cb: fn(k: any, v: any): bool): bool - This function calls "cb" once for every entry in the array or dictionary (self), with that element passed in as "key" and "value", until cb returns true, at which point "some" returns true. If it reaches the end of the array or dictionary, then "some" returns false.
* _sort(self: [any], cb: fn(lhs: any, rhs: any): int): [any] - This function sorts the entries of an array according to the callback "cb". "cb" may be called any number of times during the sorting process. "cb" must be a function that takes two parameters, "lhs" and "rhs" and returns an integer. If "lhs" is less than "rhs", then the returned integer should be negative. If they are equal, then it should be 0. Otherwise it should be positive. This returns the sorted array, leaving the original intact.
* _toLower(self: string): string - This function returns a new string, which is the same as the calling string, except all characters are lower case.
* _toString(self: any): string - This function returns a string representation of the input "self". For arrays and dictionaries, each element is converted to a string where possible, and separated by commas (and colons for dictionaries). Finally, the whole string is surrounded by brackets.
* _toUpper(self: string): string - This function returns a new string, which is the same as the calling string, except all characters are upper case.
* _trim(self: string, chars: string): string - Every character in the string "chars" is removed from the calling string's beginning and end, and the new string is returned. The original string is not modified.
* _values(self: [any : any]): [any] - This function returns an array containing each value in the dictionary. The order of the values is undefined.

## Nope Features

Some things I simply don't want to include at the current time include:

* Classes & Structures
* Do-while loops

This is because reworking the internals to add an entirely new system like this would be incredibly difficult for very little gain.

Ironically, I've never used a do-while loop seriously until I started implementing this language.


