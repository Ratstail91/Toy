/*
 * disassembler.c
 *
 *  Created on: 10 ago. 2023
 *      Original Author: Emiliano Augusto Gonzalez (egonzalez . hiperion @ gmail . com)
 * 
 * Further modified by Kayne Ruse, and added to the Toy Programming Language tool repository.
 */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdbool.h>

#include "utils.h"
#include "disassembler.h"

#define SPC(n)  printf("%.*s", n, "| | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | | |");
#define EP(x)   [x] = #x

const char *OP_STR[] = {
        EP(DIS_OP_EOF),                       //
        EP(DIS_OP_PASS),                      //
        EP(DIS_OP_ASSERT),                    //
        EP(DIS_OP_PRINT),                     //
        EP(DIS_OP_LITERAL),                   //
        EP(DIS_OP_LITERAL_LONG),              //
        EP(DIS_OP_LITERAL_RAW),               //
        EP(DIS_OP_NEGATE),                    //
        EP(DIS_OP_ADDITION),                  //
        EP(DIS_OP_SUBTRACTION),               //
        EP(DIS_OP_MULTIPLICATION),            //
        EP(DIS_OP_DIVISION),                  //
        EP(DIS_OP_MODULO),                    //
        EP(DIS_OP_GROUPING_BEGIN),            //
        EP(DIS_OP_GROUPING_END),              //
        EP(DIS_OP_SCOPE_BEGIN),               //
        EP(DIS_OP_SCOPE_END),                 //
        EP(DIS_OP_TYPE_DECL_removed),         //
        EP(DIS_OP_TYPE_DECL_LONG_removed),    //
        EP(DIS_OP_VAR_DECL),                  //
        EP(DIS_OP_VAR_DECL_LONG),             //
        EP(DIS_OP_FN_DECL),                   //
        EP(DIS_OP_FN_DECL_LONG),              //
        EP(DIS_OP_VAR_ASSIGN),                //
        EP(DIS_OP_VAR_ADDITION_ASSIGN),       //
        EP(DIS_OP_VAR_SUBTRACTION_ASSIGN),    //
        EP(DIS_OP_VAR_MULTIPLICATION_ASSIGN), //
        EP(DIS_OP_VAR_DIVISION_ASSIGN),       //
        EP(DIS_OP_VAR_MODULO_ASSIGN),         //
        EP(DIS_OP_TYPE_CAST),                 //
        EP(DIS_OP_TYPE_OF),                   //
        EP(DIS_OP_IMPORT),                    //
        EP(DIS_OP_EXPORT_removed),            //
        EP(DIS_OP_INDEX),                     //
        EP(DIS_OP_INDEX_ASSIGN),              //
        EP(DIS_OP_INDEX_ASSIGN_INTERMEDIATE), //
        EP(DIS_OP_DOT),                       //
        EP(DIS_OP_COMPARE_EQUAL),             //
        EP(DIS_OP_COMPARE_NOT_EQUAL),         //
        EP(DIS_OP_COMPARE_LESS),              //
        EP(DIS_OP_COMPARE_LESS_EQUAL),        //
        EP(DIS_OP_COMPARE_GREATER),           //
        EP(DIS_OP_COMPARE_GREATER_EQUAL),     //
        EP(DIS_OP_INVERT),                    //
        EP(DIS_OP_AND),                       //
        EP(DIS_OP_OR),                        //
        EP(DIS_OP_JUMP),                      //
        EP(DIS_OP_IF_FALSE_JUMP),             //
        EP(DIS_OP_FN_CALL),                   //
        EP(DIS_OP_FN_RETURN),                 //
        EP(DIS_OP_POP_STACK),                 //
        EP(DIS_OP_TERNARY),                   //
        EP(DIS_OP_FN_END),                    //
        };

const char *LIT_STR[] = {
        EP(DIS_LITERAL_NULL),                    //
        EP(DIS_LITERAL_BOOLEAN),                 //
        EP(DIS_LITERAL_INTEGER),                 //
        EP(DIS_LITERAL_FLOAT),                   //
        EP(DIS_LITERAL_STRING),                  //
        EP(DIS_LITERAL_ARRAY),                   //
        EP(DIS_LITERAL_DICTIONARY),              //
        EP(DIS_LITERAL_FUNCTION),                //
        EP(DIS_LITERAL_IDENTIFIER),              //
        EP(DIS_LITERAL_TYPE),                    //
        EP(DIS_LITERAL_OPAQUE),                  //
        EP(DIS_LITERAL_ANY),                     //
        EP(DIS_LITERAL_TYPE_INTERMEDIATE),       //
        EP(DIS_LITERAL_ARRAY_INTERMEDIATE),      //
        EP(DIS_LITERAL_DICTIONARY_INTERMEDIATE), //
        EP(DIS_LITERAL_FUNCTION_INTERMEDIATE),   //
        EP(DIS_LITERAL_FUNCTION_ARG_REST),       //
        EP(DIS_LITERAL_FUNCTION_NATIVE),         //
        EP(DIS_LITERAL_FUNCTION_HOOK),           //
        EP(DIS_LITERAL_INDEX_BLANK),             //
        };

enum DIS_ARG_TYPE {
    DIS_ARG_NONE,    //
    DIS_ARG_BYTE,    //
    DIS_ARG_WORD,    //
    DIS_ARG_INTEGER, //
    DIS_ARG_FLOAT,   //
    DIS_ARG_STRING   //
};

const uint8_t OP_ARGS[DIS_OP_END_OPCODES][2] = {
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_EOF
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_PASS
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_ASSERT
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_PRINT
        { DIS_ARG_BYTE, DIS_ARG_NONE }, // DIS_OP_LITERAL
        { DIS_ARG_WORD, DIS_ARG_NONE }, // DIS_OP_LITERAL_LONG
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_LITERAL_RAW
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_NEGATE
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_ADDITION
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_SUBTRACTION
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_MULTIPLICATION
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_DIVISION
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_MODULO
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_GROUPING_BEGIN
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_GROUPING_END
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_SCOPE_BEGIN
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_SCOPE_END
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_TYPE_DECL_removed
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_TYPE_DECL_LONG_removed
        { DIS_ARG_BYTE, DIS_ARG_BYTE }, // DIS_OP_VAR_DECL
        { DIS_ARG_WORD, DIS_ARG_WORD }, // DIS_OP_VAR_DECL_LONG
        { DIS_ARG_BYTE, DIS_ARG_BYTE }, // DIS_OP_FN_DECL
        { DIS_ARG_WORD, DIS_ARG_WORD }, // DIS_OP_FN_DECL_LONG
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_VAR_ASSIGN
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_VAR_ADDITION_ASSIGN
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_VAR_SUBTRACTION_ASSIGN
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_VAR_MULTIPLICATION_ASSIGN
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_VAR_DIVISION_ASSIGN
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_VAR_MODULO_ASSIGN
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_TYPE_CAST
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_TYPE_OF
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_IMPORT
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_EXPORT_removed
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_INDEX
        { DIS_ARG_BYTE, DIS_ARG_NONE }, // DIS_OP_INDEX_ASSIGN
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_INDEX_ASSIGN_INTERMEDIATE
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_DOT
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_COMPARE_EQUAL
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_COMPARE_NOT_EQUAL
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_COMPARE_LESS
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_COMPARE_LESS_EQUAL
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_COMPARE_GREATER
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_COMPARE_GREATER_EQUAL
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_INVERT
        { DIS_ARG_WORD, DIS_ARG_NONE }, // DIS_OP_AND
        { DIS_ARG_WORD, DIS_ARG_NONE }, // DIS_OP_OR
        { DIS_ARG_WORD, DIS_ARG_NONE }, // DIS_OP_JUMP
        { DIS_ARG_WORD, DIS_ARG_NONE }, // DIS_OP_IF_FALSE_JUMP
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_FN_CALL
        { DIS_ARG_WORD, DIS_ARG_NONE }, // DIS_OP_FN_RETURN
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_POP_STACK
        { DIS_ARG_NONE, DIS_ARG_NONE }, // DIS_OP_TERNARY
};

typedef struct dis_program_s {
    uint8_t *program;
    uint32_t len;
    uint32_t pc;
} dis_program_t;

typedef struct fun_code_s {
	uint32_t start;
	uint32_t len;
	char *fun;
} fun_code_t;

static void dis_print_opcode(uint8_t op);

static uint8_t readByte(const uint8_t *tb, uint32_t *count) {
    uint8_t ret = *(uint8_t*) (tb + *count);
    *count += 1;
    return ret;
}

static uint16_t readWord(const uint8_t *tb, uint32_t *count) {
    uint16_t ret = 0;
    memcpy(&ret, tb + *count, 2);
    *count += 2;
    return ret;
}

static int32_t readInt(const uint8_t *tb, uint32_t *count) {
    int ret = 0;
    memcpy(&ret, tb + *count, 4);
    *count += 4;
    return ret;
}

static float readFloat(const uint8_t *tb, uint32_t *count) {
    float ret = 0;
    memcpy(&ret, tb + *count, 4);
    *count += 4;
    return ret;
}

static char* readString(const uint8_t *tb, uint32_t *count) {
    const unsigned char *ret = tb + *count;
    *count += strlen((char*) ret) + 1; //+1 for null character
    return (char*) ret;
}

static void consumeByte(uint8_t byte, uint8_t *tb, uint32_t *count) {
    if (byte != tb[*count]) {
        printf("[internal] Failed to consume the correct byte (expected %u, found %u)\n", byte, tb[*count]);
        exit(1);
    }

    *count += 1;
}

///////////////////////////////////////////////////////////////////////////////

static void dis_disassembler_init(dis_program_t **prg) {
    (*prg) = malloc(sizeof(struct dis_program_s));
    (*prg)->len = 0;
    (*prg)->pc = 0;
}

static void dis_disassembler_deinit(dis_program_t **prg) {
    free((*prg)->program);
    free((*prg));
}

static uint8_t dis_load_file(const char *filename, dis_program_t **prg) {
    FILE *f;
    size_t fsize, bytes;
    uint32_t count = 0;
    uint8_t buf = 0;

    f = fopen(filename, "r");
    if (f == NULL) {
        printf("Not able to open the file.\n");
        return 1;
    }

    fseek(f, 0, SEEK_END);
    fsize = ftell(f);
    fseek(f, 0, SEEK_SET);
    (*prg)->program = malloc(fsize * sizeof(uint8_t));

    while ((bytes = fread(&buf, sizeof(uint8_t), 1, f)) == 1)
        (*prg)->program[count++] = buf;

    (*prg)->len = fsize;
    printf("\nFile: %s\nSize: %zu\n", filename, fsize);

    fclose(f);
    return 0;
}

static void dis_read_header(dis_program_t **prg) {
    const unsigned char major = readByte((*prg)->program, &((*prg)->pc));
    const unsigned char minor = readByte((*prg)->program, &((*prg)->pc));
    const unsigned char patch = readByte((*prg)->program, &((*prg)->pc));
    const char *build = readString((*prg)->program, &((*prg)->pc));
    printf("[Header Version: %d.%d.%d (%s)]\n", major, minor, patch, build);
}

static void dis_print_opcode(uint8_t op) {
    if (op == 255) {
        printf("SECTION_END");
        return;
    }

    if (op < DIS_OP_END_OPCODES)
        printf("%s", (OP_STR[op] + 7));
    else
        printf("(OP UNKNOWN [%c])", op);
}

///////////////////////////////////////////////////////////////////////////////

#define S_OP(n) \
		switch (OP_ARGS[opcode][n]) { \
		    case DIS_ARG_NONE: \
		    break; \
		    case DIS_ARG_BYTE: \
		        uint = readByte((*prg)->program, &pc); \
		        printf(" b(%d)", uint); \
		    break; \
		    case DIS_ARG_WORD: \
		        uint = readWord((*prg)->program, &pc);\
		        printf(" w(%d)", uint); \
		    break; \
		    case DIS_ARG_INTEGER: \
		        intg = readInt((*prg)->program, &pc); \
		        printf(" i(%d)", intg); \
		    break; \
		    case DIS_ARG_FLOAT: \
		        flt = readFloat((*prg)->program, &pc); \
		        printf(" f(%f)", flt); \
		    break; \
		    case DIS_ARG_STRING: \
		        str = readString((*prg)->program, &pc); \
		        printf(" s(%s)", str); \
		    break; \
		    default: \
		        printf("ERROR, unknown argument type\n"); \
		        exit(1); \
		}

static void dis_disassemble_section(dis_program_t **prg, uint32_t pc,
		uint32_t len, uint8_t spaces, bool is_function, bool alt_fmt) {
	uint8_t opcode;
	uint32_t uint;
	int32_t intg;
	float flt;
	char *str;

	//first 4 bytes of the program section within a function are actually specifying the parameter and return lists
	if (is_function) {
		printf("\n");
		uint16_t args = readWord((*prg)->program, &pc);
		uint16_t rets = readWord((*prg)->program, &pc);
		if (!alt_fmt) {
			SPC(spaces);
			printf("| ");
		} else
			printf("  .comment args [%d], rets [%d]", args, rets);
	}

	uint32_t pc_start = pc;
	while (pc < len) {
		opcode = (*prg)->program[pc];
		if (alt_fmt && (opcode == 255 || opcode == 0)) {
			++pc;
			continue;
		}

		printf("\n");
		if (!alt_fmt) {
			SPC(spaces);
			printf("| ");
		} else
			printf("  ");

		printf("[%05d](%03d) ", (pc++) - pc_start, opcode);
		dis_print_opcode(opcode);

		if (opcode > DIS_OP_END_OPCODES)
			continue;

		S_OP(0);
		S_OP(1);
	}
}

#define LIT_ADD(a, b, c)  b[c] = a;  ++c;
static void dis_read_interpreter_sections(dis_program_t **prg, uint32_t *pc,
		uint8_t spaces, char *tree, bool alt_fmt) {
	uint32_t literal_count = 0;
	uint8_t literal_type[65536];

	const unsigned short literalCount = readWord((*prg)->program, pc);

	printf("\n");
	if (!alt_fmt) {
		SPC(spaces);
		printf("| ");
		printf("  ");
		printf("--- ( Reading %d literals from cache ) ---\n", literalCount);
	}

	for (int i = 0; i < literalCount; i++) {
		const unsigned char literalType = readByte((*prg)->program, pc);

		switch (literalType) {
		case DIS_LITERAL_NULL:
			LIT_ADD(DIS_LITERAL_NULL, literal_type, literal_count) ;
			if (!alt_fmt) {
				SPC(spaces);
				printf("| | ");
			} else
				printf("  ");
			printf("[%05d] ( null )\n", i);
			break;

		case DIS_LITERAL_BOOLEAN: {
			const bool b = readByte((*prg)->program, pc);
			LIT_ADD(DIS_LITERAL_BOOLEAN, literal_type, literal_count);
			if (!alt_fmt) {
				SPC(spaces);
				printf("| | ");
			} else
				printf("  ");
			printf("[%05d] ( boolean %s )\n", i, b ? "true" : "false");
		}
			break;

		case DIS_LITERAL_INTEGER: {
			const int d = readInt((*prg)->program, pc);
			LIT_ADD(DIS_LITERAL_INTEGER, literal_type, literal_count);
			if (!alt_fmt) {
				SPC(spaces);
				printf("| | ");
			} else
				printf("  ");
			printf("[%05d] ( integer %d )\n", i, d);
		}
			break;

		case DIS_LITERAL_FLOAT: {
			const float f = readFloat((*prg)->program, pc);
			LIT_ADD(DIS_LITERAL_FLOAT, literal_type, literal_count);
			if (!alt_fmt) {
				SPC(spaces);
				printf("| | ");
			} else
				printf("  ");
			printf("[%05d] ( float %f )\n", i, f);
		}
			break;

		case DIS_LITERAL_STRING: {
			const char *s = readString((*prg)->program, pc);
			LIT_ADD(DIS_LITERAL_STRING, literal_type, literal_count);
			if (!alt_fmt) {
				SPC(spaces);
				printf("| | ");
			} else
				printf("  ");
			printf("[%05d] ( string \"%s\" )\n", i, s);
		}
			break;

		case DIS_LITERAL_ARRAY_INTERMEDIATE:
		case DIS_LITERAL_ARRAY: {
			unsigned short length = readWord((*prg)->program, pc);
			if (!alt_fmt) {
				SPC(spaces);
				printf("| | ");
			} else
				printf("  ");
			printf("[%05d] ( array ", i);
			for (int i = 0; i < length; i++) {
				int index = readWord((*prg)->program, pc);
				printf("%d ", index);
				LIT_ADD(DIS_LITERAL_NULL, literal_type, literal_count);
				if (!(i % 15) && i != 0) {
					printf("\n");
					if (!alt_fmt) {
						SPC(spaces);
						printf("| | ");
					} else
						printf("  ");
					printf("                ");
				}
			}
			printf(")\n");
			LIT_ADD(DIS_LITERAL_ARRAY, literal_type, literal_count);
		}
			break;

		case DIS_LITERAL_DICTIONARY_INTERMEDIATE:
		case DIS_LITERAL_DICTIONARY: {
			unsigned short length = readWord((*prg)->program, pc);
			if (!alt_fmt) {
				SPC(spaces);
				printf("| | ");
			} else
				printf("  ");
			printf("[%05d] ( dictionary ", i);
			for (int i = 0; i < length / 2; i++) {
				int key = readWord((*prg)->program, pc);
				int val = readWord((*prg)->program, pc);
				printf("(key: %d, val:%d) ", key, val);
				if (!(i % 5) && i != 0) {
					printf("\n");
					if (!alt_fmt) {
						SPC(spaces);
						printf("| | ");
					} else
						printf("  ");
					printf("                     ");
				}
			}
			printf(")\n");
			LIT_ADD(DIS_LITERAL_DICTIONARY, literal_type, literal_count);
		}
			break;

		case DIS_LITERAL_FUNCTION: {
			unsigned short index = readWord((*prg)->program, pc);
			LIT_ADD(DIS_LITERAL_FUNCTION_INTERMEDIATE, literal_type,
					literal_count);
			if (!alt_fmt) {
				SPC(spaces);
				printf("| | ");
			} else
				printf("  ");
			printf("[%05d] ( function index: %d )\n", i, index);
		}
			break;

		case DIS_LITERAL_IDENTIFIER: {
			const char *str = readString((*prg)->program, pc);
			LIT_ADD(DIS_LITERAL_IDENTIFIER, literal_type, literal_count);
			if (!alt_fmt) {
				SPC(spaces);
				printf("| | ");
			} else
				printf("  ");
			printf("[%05d] ( identifier %s )\n", i, str);
		}
			break;

		case DIS_LITERAL_TYPE:
		case DIS_LITERAL_TYPE_INTERMEDIATE: {
			uint8_t literalType = readByte((*prg)->program, pc);
			uint8_t constant = readByte((*prg)->program, pc);
			if (!alt_fmt) {
				SPC(spaces);
				printf("| | ");
			} else
				printf("  ");
			printf("[%05d] ( type %s: %d)\n", i, (LIT_STR[literalType] + 12),
					constant);
			if (literalType == DIS_LITERAL_ARRAY) {
				uint16_t vt = readWord((*prg)->program, pc);
				if (!alt_fmt) {
					SPC(spaces);
					printf("| | ");
				} else
					printf("  ");
				printf("          ( subtype: %d)\n", vt);
			}

			if (literalType == DIS_LITERAL_DICTIONARY) {
				uint8_t kt = readWord((*prg)->program, pc);
				uint8_t vt = readWord((*prg)->program, pc);
				if (!alt_fmt) {
					SPC(spaces);
					printf("| | ");
				} else
					printf("  ");
				printf("          ( subtype: [%d, %d] )\n", kt, vt);
			}
			LIT_ADD(literalType, literal_type, literal_count);
		}
			break;

		case DIS_LITERAL_INDEX_BLANK:
			LIT_ADD(DIS_LITERAL_INDEX_BLANK, literal_type, literal_count);
			if (!alt_fmt) {
				SPC(spaces);
				printf("| | ");
			} else
				printf("  ");
			printf("[%05d] ( blank )\n", i);
			break;
		}
	}

	consumeByte(DIS_OP_SECTION_END, (*prg)->program, pc);

	if (!alt_fmt) {
		SPC(spaces);
		printf("| ");
		printf("--- ( end literal section ) ---\n");
	}


	int functionCount = readWord((*prg)->program, pc);
	int functionSize = readWord((*prg)->program, pc);

	if (functionCount) {
		if (!alt_fmt) {
			SPC(spaces);
			printf("|\n");
			SPC(spaces);
			printf("| ");
			printf("--- ( fn count: %d, total size: %d ) ---\n", functionCount, functionSize);
		}

		uint32_t fcnt = 0;
		char tree_local[2048];

		for (uint32_t i = 0; i < literal_count; i++) {
			if (literal_type[i] == DIS_LITERAL_FUNCTION_INTERMEDIATE) {
				size_t size = (size_t) readWord((*prg)->program, pc);

				uint32_t fpc_start = *pc;
				uint32_t fpc_end = *pc + size - 1;

				tree_local[0] = '\0';
				if (!alt_fmt) {
					sprintf(tree_local, "%s.%d", tree, fcnt);
					if (tree_local[0] == '_')
						memcpy(tree_local, tree_local + 1, strlen(tree_local));
				} else {
					sprintf(tree_local, "%s_%d", tree, fcnt);
					if (tree_local[0] == '_')
						memcpy(tree_local, tree_local + 1, strlen(tree_local));
				}

				if (!alt_fmt) {
					SPC(spaces);
					printf("| |\n");
					SPC(spaces);
					printf("| | ");
					printf("( fun %s [ start: %d, end: %d ] )", tree_local, fpc_start, fpc_end);
				} else
					printf("\nLIT_FUN_%s:", tree_local);



				if ((*prg)->program[*pc + size - 1] != DIS_OP_FN_END) {
					printf("\nERROR: Failed to find function end\n");
					exit(1);
				}

				dis_read_interpreter_sections(prg, &fpc_start, spaces + 4, tree_local, alt_fmt);

				if (!alt_fmt) {
					SPC(spaces);
					printf("| | |\n");
					SPC(spaces + 4);
					printf("| ");
					printf("--- ( reading code for %s ) ---", tree_local);
					dis_disassemble_section(prg, fpc_start, fpc_end, spaces + 4, true, alt_fmt);
					printf("\n");
					SPC(spaces + 4);
					printf("| ");
					printf("--- ( end code section ) ---\n");
				} else {
					fun_code_t *fun = malloc(sizeof(struct fun_code_s));
					fun->fun = malloc(strlen(tree_local) + 1);
					strcpy(fun->fun, tree_local);
					fun->start = fpc_start;
					fun->len = fpc_end;
					enqueue((void*) fun);
				}

				fcnt++;
				*pc += size;
			}
		}

		if (!alt_fmt) {
			SPC(spaces);
			printf("|\n");
			SPC(spaces);
			printf("| ");
			printf("--- ( end fn section ) ---\n");
		}
	}

	consumeByte(DIS_OP_SECTION_END, (*prg)->program, pc);
}

///////////////////////////////////////////////////////////////////////////////

void disassemble(const char *filename, bool alt_fmt) {
	dis_program_t *prg;
	queue_front = NULL;
	queue_rear = NULL;

	dis_disassembler_init(&prg);
	if (dis_load_file(filename, &prg))
		exit(1);

	dis_read_header(&prg);

	consumeByte(DIS_OP_SECTION_END, prg->program, &(prg->pc));

	if (alt_fmt)
		printf("\nLIT_MAIN:");
	dis_read_interpreter_sections(&prg, &(prg->pc), 0, "", alt_fmt);
	if (!alt_fmt) {
		printf("|\n| ");
		printf("--- ( reading main code ) ---");
	} else
		printf("\nMAIN:");
	dis_disassemble_section(&prg, prg->pc, prg->len, 0, false, alt_fmt);
	if (!alt_fmt) {
		printf("\n| ");
		printf("--- ( end main code section ) ---");
	} else
		printf("\n");

	if (alt_fmt) {
		while (queue_front != NULL) {
			fun_code_t *fun = (fun_code_t*)front();
			printf("\nFUN_%s:", fun->fun);
			free(fun->fun);

			dis_disassemble_section(&prg, fun->start, fun->len, 0, true, alt_fmt);

			dequeue();
			printf("\n");
		}

	}

	printf("\n");
	dis_disassembler_deinit(&prg);
}
