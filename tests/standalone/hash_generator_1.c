//https://www.programiz.com/c-programming/online-compiler/
#include <stdio.h>

static unsigned int hashUInt(unsigned int x) {
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = ((x >> 16) ^ x) * 0x45d9f3b;
    x = (x >> 16) ^ x;
    return x;
}

int main() {
    //print the index/hash pairs
    for (unsigned int i = 0; i < 100; i++) {
        printf("{%u:%u}\n", i, hashUInt(i));
    }

    return 0;
}
