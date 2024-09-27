#include "toy_memory.h"

#include <stdio.h>

unsigned char* readFile(const char* path, int* size) {
	//open the file
	FILE* file = fopen(path, "rb");
	if (file == NULL) {
		return NULL;
	}

	//determine the file's length
	fseek(file, 0L, SEEK_END);
	*size = ftell(file);
	rewind(file);

	//make some space
	unsigned char* buffer = TOY_ALLOCATE(unsigned char, *size);
	if (buffer == NULL) {
		fclose(file);
		return NULL;
	}

	//
	if (fread(buffer, sizeof(unsigned char), *size, file) < *size) {
		fclose(file);
		*size = -1; //singal a read error
		return NULL;
	}

	fclose(file);
	return buffer;
}


int main(int argc, char* argv[]) {
	int size = 0;
	unsigned char* buffer = readFile("../repl/main.c", &size); //for now, just grab the main.c file as a test

	if (buffer == NULL) {
		fprintf(stderr, "Failed to open the file\n");
	}

	if (size < 0) {
		fprintf(stderr, "Failed to read the file\n");
	}

	TOY_FREE_ARRAY(unsigned char, buffer, size);

	printf("All good\n");
	return 0;
}
