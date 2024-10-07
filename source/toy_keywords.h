#pragma once

#include "toy_token_types.h"
#include "toy_common.h"

typedef struct {
	const Toy_TokenType type;
	const char* keyword;
} Toy_KeywordTypeTuple;

extern const Toy_KeywordTypeTuple Toy_private_keywords[];

//access
const char* Toy_private_findKeywordByType(const Toy_TokenType type);
Toy_TokenType Toy_private_findTypeByKeyword(const char* keyword);

