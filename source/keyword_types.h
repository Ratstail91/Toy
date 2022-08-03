#pragma once

#include "token_types.h"

typedef struct {
	TokenType type;
	char* keyword;
} KeywordType;

extern KeywordType keywordTypes[];

//for debugging
char* findKeywordByType(TokenType type);
