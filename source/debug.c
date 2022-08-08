#include "debug.h"

#include "keyword_types.h"
#include "lexer.h"
#include "parser.h"
#include "compiler.h"

#include <stdio.h>

//utils
static unsigned char printByte(unsigned char* tb, int* count) {
	unsigned char ret = *(unsigned char*)(tb + *count);
	printf("%u ", ret);
	*count += 1;
	return ret;
}

static unsigned short printShort(unsigned char* tb, int* count) {
	unsigned short ret = *(unsigned short*)(tb + *count);
	printf("%d ", ret);
	*count += 2;
	return ret;
}

static int printInt(unsigned char* tb, int* count) {
	int ret = *(int*)(tb + *count);
	printf("%d ", ret);
	*count += 4;
	return ret;
}

static float printFloat(unsigned char* tb, int* count) {
	float ret = *(float*)(tb + *count);
	printf("%f ", ret);
	*count += 4;
	return ret;
}

static unsigned char* printString(unsigned char* tb, int* count) {
	unsigned char* ret = tb + *count;
	*count += printf("%s ", (char*)ret); //return includes the space, but not the null terminator
	return ret;
}

static void consumeByte(unsigned char byte, unsigned char* str, int* count) {
	if (byte != str[*count]) {
		printf("Failed to consume the correct byte");
	}
	*count += 1;
}

static void consumeShort(unsigned short bytes, unsigned char* str, int* count) {
	if (bytes != *(unsigned short*)(str + *count)) {
		printf("Failed to consume the correct bytes");
	}
	*count += 2;
}

void dissectBytecode(unsigned char* tb, int size) {
	int count = 0;

	//header
	printf("--header--\n");
	printByte(tb, &count);
	printByte(tb, &count);
	printByte(tb, &count);
	printString(tb, &count);
	consumeByte(OP_SECTION_END, tb, &count);

	printf("\n");

	//data
	printf("--data--\n");
	const short literalCount = printShort(tb, &count);

	for (int i = 0; i < literalCount; i++) {
		const unsigned char literalType = printByte(tb, &count);

		switch(literalType) {
			case LITERAL_NULL:
				//NO-OP
				printf("(null)");
			break;

			case LITERAL_BOOLEAN: {
				const bool b = printByte(tb, &count);
				printf("(boolean %s)", b ? "true" : "false");
			}
			break;

			case LITERAL_INTEGER: {
				const int d = printInt(tb, &count);
				printf("(integer %d)", d);
			}
			break;

			case LITERAL_FLOAT: {
				const float f = printFloat(tb, &count);
				printf("(float %f)", f);
			}
			break;

			case LITERAL_STRING: {
				const unsigned char* s = printString(tb, &count);
				printf("(string)");
			}
			break;
		}

		printf("\n");
	}

	consumeByte(OP_SECTION_END, tb, &count);

	//code
	printf("--bytecode--\n");
	while(tb[count] != OP_EOF) {
		const unsigned char opcode = printByte(tb, &count);

		switch (opcode) {
			case OP_ASSERT:
				printf("assert\n");
			break;

			case OP_PRINT:
				printf("print\n");
			break;

			case OP_LITERAL:
				printf("literal ");
				printByte(tb, &count);
				printf("\n");
			break;

			case OP_LITERAL_LONG:
				printf("long literal ");
				printShort(tb, &count);
				printf("\n");
			break;

			case OP_NEGATE:
				printf("negate\n");
			break;

			case OP_ADDITION:
				printf("+\n");
			break;

			case OP_SUBTRACTION:
				printf("-\n");
			break;
			
			case OP_MULTIPLICATION:
				printf("*\n");
			break;
			
			case OP_DIVISION:
				printf("/\n");
			break;

			case OP_MODULO:
				printf("%%\n");
			break;

			case OP_GROUPING_BEGIN:
				printf("(\n");
			break;

			case OP_GROUPING_END:
				printf(")\n");
			break;

			case OP_SECTION_END: {
				printf("--SECTION END--\n");
			}
			break;

			default:
				printf("Unknown opcode found\n");
		}
	}

	consumeByte(OP_EOF, tb, &count);
}