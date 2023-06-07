#pragma once

#include "toy_common.h"

const unsigned char* Toy_readFile(const char* path, size_t* fileSize);
int Toy_writeFile(const char* path, const unsigned char* bytes, size_t size);

const unsigned char* Toy_compileString(const char* source, size_t* size);

void Toy_runBinary(const unsigned char* tb, size_t size);
void Toy_runBinaryFile(const char* fname);
void Toy_runSource(const char* source);
void Toy_runSourceFile(const char* fname);

void Toy_parseBinaryFileHeader(const char* fname);