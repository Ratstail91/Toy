#pragma once

#include "interpreter.h"

//the _index function is a historical oddity - it's used whenever a compound is indexed
int _index(Interpreter* interpreter, LiteralArray* arguments);

//globally available native functions
int _set(Interpreter* interpreter, LiteralArray* arguments);
int _get(Interpreter* interpreter, LiteralArray* arguments);
int _push(Interpreter* interpreter, LiteralArray* arguments);
int _pop(Interpreter* interpreter, LiteralArray* arguments);
int _length(Interpreter* interpreter, LiteralArray* arguments);
int _clear(Interpreter* interpreter, LiteralArray* arguments);
