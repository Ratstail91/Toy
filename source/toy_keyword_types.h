#pragma once

#include "toy_token_types.h"

typedef struct {
	Toy_TokenType type;
	char* keyword;
} Toy_KeywordType;

extern Toy_KeywordType Toy_keywordTypes[];

char* Toy_findKeywordByType(Toy_TokenType type);

Toy_TokenType Toy_findTypeByKeyword(const char* keyword);
