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

static void* trackerAllocator(void* pointer, size_t oldSize, size_t newSize) {
	if (newSize == 0 && oldSize == 0) {
		//causes issues, so just skip out with a NO-OP
		return NULL;
	}

	memoryAllocCalls++;

	//track the changes
	currentMemoryUsed = currentMemoryUsed - oldSize + newSize;
	maxMemoryUsed = currentMemoryUsed > maxMemoryUsed ? currentMemoryUsed : maxMemoryUsed;

	if (newSize == 0) {
		free(pointer);

		return NULL;
	}

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
	printf("Memory report: %d max bytes, %d calls\n", maxMemoryUsed, memoryAllocCalls);

	return 0;
}
