#pragma once

#include "common.h"

char* readFile(char* path, size_t* fileSize);
void writeFile(char* path, unsigned char* bytes, size_t size);

unsigned char* compileString(char* source, size_t* size);

void runBinary(unsigned char* tb, size_t size);
void runBinaryFile(char* fname);
void runSource(char* source);
void runSourceFile(char* fname);

