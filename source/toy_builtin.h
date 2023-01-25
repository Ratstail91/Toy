#pragma once

#include "toy_interpreter.h"

//the _index function is a historical oddity - it's used whenever a compound is indexed
int Toy_private_index(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);

//globally available native functions
int Toy_private_set(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);
int Toy_private_get(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);
int Toy_private_push(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);
int Toy_private_pop(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);
int Toy_private_length(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);
int Toy_private_clear(Toy_Interpreter* interpreter, Toy_LiteralArray* arguments);
