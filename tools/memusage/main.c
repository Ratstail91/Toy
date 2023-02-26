#include "repl_tools.h"
#include "toy_memory.h"
#include "toy_console_colors.h"
#include "lib_runner.h"

#include <stdio.h>
#include <stdlib.h>

//tracker allocator
int currentMemoryUsed = 0;
int maxMemoryUsed = 0;
int memoryAllocCalls = 0;
int memoryAllocFree = 0;
int memoryAllocRealloc = 0;

static void* trackerAllocator(void* pointer, size_t oldSize, size_t newSize) {
	//the number of raw calls
	memoryAllocCalls++;

	//causes issues, so just skip out with a NO-OP
	if (newSize == 0 && oldSize == 0) {
		return NULL;
	}

	//track the changes
	currentMemoryUsed = currentMemoryUsed - oldSize + newSize;
	maxMemoryUsed = currentMemoryUsed > maxMemoryUsed ? currentMemoryUsed : maxMemoryUsed;

	if (newSize == 0) {
		//the number of frees
		memoryAllocFree++;
		free(pointer);

		return NULL;
	}

	//the number of reallocations
	memoryAllocRealloc++;
	void* mem = realloc(pointer, newSize);

	if (mem == NULL) {
		fprintf(stderr, TOY_CC_ERROR "[internal] Memory allocation error (requested %d, replacing %d)\n" TOY_CC_RESET, (int)newSize, (int)oldSize);
		exit(-1);
	}

	return mem;
}

int main(int argc, const char* argv[]) {
	if (argc <= 1) {
		return -1;
	}

	//not used, except for print
	Toy_initCommandLine(argc, argv);

	//setup for runner
	Toy_initDriveDictionary();

	Toy_Literal driveLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("scripts"));
	Toy_Literal pathLiteral = TOY_TO_STRING_LITERAL(Toy_createRefString("scripts"));

	Toy_setLiteralDictionary(Toy_getDriveDictionary(), driveLiteral, pathLiteral);

	Toy_freeLiteral(driveLiteral);
	Toy_freeLiteral(pathLiteral);

	Toy_setMemoryAllocator(trackerAllocator);

	//run memory tests
	for (int fileCounter = 1; fileCounter < argc; fileCounter++) {
		Toy_runSourceFile(argv[fileCounter]);
	}

	//lib cleanup
	Toy_freeDriveDictionary();

	//report output
	printf("Heap Memory Report:\n\t%d max bytes\n\t%d calls to the allocator\n\t%d calls to realloc()\n\t%d calls to free()\n\t%d discrepancies\n", maxMemoryUsed, memoryAllocCalls, memoryAllocRealloc, memoryAllocFree, memoryAllocCalls - memoryAllocRealloc - memoryAllocFree);

	return 0;
}
