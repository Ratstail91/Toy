#pragma once

#include "token_types.h"

typedef struct {
	TokenType type;
	char* keyword;
} KeywordType;

extern KeywordType keywordTypes[];

char* findKeywordByType(TokenType type);

TokenType findTypeByKeyword(const char* keyword);
