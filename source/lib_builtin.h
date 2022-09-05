#pragma once

#include "interpreter.h"

int _index(Interpreter* interpreter, LiteralArray* arguments);
int _dot(Interpreter* interpreter, LiteralArray* arguments);
int _set(Interpreter* interpreter, LiteralArray* arguments);
int _get(Interpreter* interpreter, LiteralArray* arguments);
int _push(Interpreter* interpreter, LiteralArray* arguments);
int _pop(Interpreter* interpreter, LiteralArray* arguments);
int _length(Interpreter* interpreter, LiteralArray* arguments);
int _clear(Interpreter* interpreter, LiteralArray* arguments);

