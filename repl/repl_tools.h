#pragma once

#include "toy_common.h"

char* Toy_readFile(char* path, size_t* fileSize);
int Toy_writeFile(char* path, unsigned char* bytes, size_t size);

unsigned char* Toy_compileString(char* source, size_t* size);

void Toy_runBinary(unsigned char* tb, size_t size);
void Toy_runBinaryFile(char* fname);
void Toy_runSource(char* source);
void Toy_runSourceFile(char* fname);

