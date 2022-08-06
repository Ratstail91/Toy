#include "literal.h"
#include "memory.h"

#include <stdio.h>
#include <string.h>

void printLiteral(Literal literal) {
	switch(literal.type) {
		case LITERAL_NULL:
			printf("null");
			break;

		case LITERAL_BOOLEAN:
			printf(AS_BOOLEAN(literal) ? "true" : "false");
			break;

		case LITERAL_INTEGER:
			printf("%d", AS_INTEGER(literal));
			break;

		case LITERAL_FLOAT:
			printf("%g", AS_FLOAT(literal));
			break;

		case LITERAL_STRING:
			printf("%.*s", STRLEN(literal), AS_STRING(literal));
			break;

		default:
			//should never bee seen
			fprintf(stderr, "[Internal] Unrecognized literal type: %d", literal.type);
	}
}

void freeLiteral(Literal literal) {
	if (IS_STRING(literal)) {
		FREE(char, AS_STRING(literal));
		return;
	}
}

bool _isTruthy(Literal x) {
	return (IS_NULL(x) || (IS_BOOLEAN(x) && AS_BOOLEAN(x)) || (IS_INTEGER(x) && AS_INTEGER(x) != 0) || (IS_FLOAT(x) && AS_FLOAT(x) != 0));
}

Literal _toStringLiteral(char* cstr) {
	return ((Literal){LITERAL_STRING, { .string.ptr = (char*)cstr, .string.length = strlen((char*)cstr) }});
}

char* copyString(char* original, int length) {
	char* buffer = ALLOCATE(char, length + 1);
	strncpy(buffer, original, length);
	buffer[length] = '\0';
	return buffer;
}