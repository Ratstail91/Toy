#pragma once

#include "literal.h"

char* copyString(char* original, int length);
bool literalsAreEqual(Literal lhs, Literal rhs);
int hashLiteral(Literal lit);

void printLiteral(Literal literal);
void printLiteralCustom(Literal literal, void (printFn)(const char*));
