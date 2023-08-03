#include "lib_io.h"

#include <stdio.h>

//call the hook
typedef struct Natives {
	char* name;
	Toy_NativeFn fn;
} Natives;


int Toy_hookIO(Toy_Interpreter* interpreter, Toy_Literal identifier, Toy_Literal alias) {
    return 1;
}