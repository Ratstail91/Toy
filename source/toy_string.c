#include "toy_string.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//utils
static void deepCopyUtil(char* dest, Toy_String* str) {
	//sometimes, "clever" can be a bad thing...
	if (str->type == TOY_STRING_NODE) {
		deepCopyUtil(dest, str->as.node.left);
		deepCopyUtil(dest + str->as.node.left->length, str->as.node.right);
	}

	else {
		memcpy(dest, str->as.leaf.data, str->length);
	}
}

static void incrementRefCount(Toy_String* str) {
	str->refCount++;
	if (str->type == TOY_STRING_NODE) {
		incrementRefCount(str->as.node.left);
		incrementRefCount(str->as.node.right);
	}
}

static void decrementRefCount(Toy_String* str) {
	str->refCount--;
	if (str->type == TOY_STRING_NODE) {
		decrementRefCount(str->as.node.left);
		decrementRefCount(str->as.node.right);
	}
}

//exposed functions
Toy_String* Toy_createString(Toy_Bucket** bucket, const char* cstring) {
	int length = strlen(cstring);

	return Toy_createStringLength(bucket, cstring, length);
}

Toy_String* Toy_createStringLength(Toy_Bucket** bucket, const char* cstring, int length) {
	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucket, sizeof(Toy_String) + length + 1); //TODO: compensate for partitioning more space than bucket capacity

	ret->type = TOY_STRING_LEAF;
	ret->length = length;
	ret->refCount = 1;
	memcpy(ret->as.leaf.data, cstring, length);
	ret->as.leaf.data[length] = '\0';

	return ret;
}

Toy_String* Toy_copyString(Toy_Bucket** bucket, Toy_String* str) {
	if (str->refCount == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't copy a string with refcount of zero\n" TOY_CC_RESET);
		exit(-1);
	}
	incrementRefCount(str);
	return str;
}

Toy_String* Toy_deepCopyString(Toy_Bucket** bucket, Toy_String* str) {
	if (str->refCount == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't deep copy a string with refcount of zero\n" TOY_CC_RESET);
		exit(-1);
	}
	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucket, sizeof(Toy_String) + str->length + 1); //TODO: compensate for partitioning more space than bucket capacity

	//TODO
	ret->type = TOY_STRING_LEAF;
	ret->length = str->length;
	ret->refCount = 1;
	deepCopyUtil(ret->as.leaf.data, str); //copy each leaf into the buffer
	ret->as.leaf.data[ret->length] = '\0';

	return ret;
}

Toy_String* Toy_concatString(Toy_Bucket** bucket, Toy_String* left, Toy_String* right) {
	if (left->refCount == 0 || right->refCount == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't concatenate a string with refcount of zero\n" TOY_CC_RESET);
		exit(-1);
	}

	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucket, sizeof(Toy_String));

	ret->type = TOY_STRING_NODE;
	ret->length = left->length + right->length;
	ret->refCount = 1;
	ret->as.node.left = left;
	ret->as.node.right = right;

	incrementRefCount(left);
	incrementRefCount(right);

	return ret;
}

void Toy_freeString(Toy_String* str) {
	decrementRefCount(str); //TODO: tool for checking the bucket is empty, and freeing it
}

int Toy_getStringLength(Toy_String* str) {
	return str->length;
}

int Toy_getStringRefCount(Toy_String* str) {
	return str->refCount;
}

char* Toy_getStringRawBuffer(Toy_String* str) {
	if (str->refCount == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't get raw string buffer of a string with refcount of zero\n" TOY_CC_RESET);
		exit(-1);
	}

	char* buffer = malloc(str->length + 1);

	deepCopyUtil(buffer, str);
	buffer[str->length] = '\0';

	return buffer;
}
