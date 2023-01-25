#pragma once

#include "toy_interpreter.h"

//the _index function is a historical oddity - it's used whenever a compound is indexed
int _index(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);

//globally available native functions
int _set(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);
int _get(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);
int _push(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);
int _pop(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);
int _length(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);
int _clear(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);
