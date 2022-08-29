#include "lexer.h"

#include "console_colors.h"

#include <stdio.h>
#include <string.h>

int main() {
	{
		//source
		char* source = "print null;";

		//test init & quit
		Lexer lexer;
		initLexer(&lexer, source);

		//get each token
		Token print = scanLexer(&lexer);
		Token null = scanLexer(&lexer);
		Token semi = scanLexer(&lexer);
		Token eof = scanLexer(&lexer);

		//test each token is correct
		if (strncmp(print.lexeme, "print", print.length)) {
			fprintf(stderr, ERROR "ERROR: print lexeme is wrong: %s" RESET, print.lexeme);
			return -1;
		}


		if (strncmp(null.lexeme, "null", null.length)) {
			fprintf(stderr, ERROR "ERROR: null lexeme is wrong: %s" RESET, null.lexeme);
			return -1;
		}

		if (strncmp(semi.lexeme, ";", semi.length)) {
			fprintf(stderr, ERROR "ERROR: semicolon lexeme is wrong: %s" RESET, semi.lexeme);
			return -1;
		}

		if (eof.type != TOKEN_EOF) {
			fprintf(stderr, ERROR "ERROR: Failed to find EOF token" RESET);
			return -1;
		}
	}

	printf(NOTICE "All good\n" RESET);
	return 0;
}

