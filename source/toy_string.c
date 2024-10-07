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
Toy_String* Toy_createString(Toy_Bucket** bucketHandle, const char* cstring) {
	int length = strlen(cstring);

	return Toy_createStringLength(bucketHandle, cstring, length);
}

Toy_String* Toy_createStringLength(Toy_Bucket** bucketHandle, const char* cstring, int length) {
	if (length > TOY_STRING_MAX_LENGTH) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't create a string longer than %d\n" TOY_CC_RESET, TOY_STRING_MAX_LENGTH);
		exit(-1);
	}

	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucketHandle, sizeof(Toy_String) + length + 1); //TODO: compensate for partitioning more space than bucket capacity

	ret->type = TOY_STRING_LEAF;
	ret->length = length;
	ret->refCount = 1;
	ret->cachedHash = 0; //don't calc until needed
	memcpy(ret->as.leaf.data, cstring, length + 1);
	ret->as.leaf.data[length] = '\0';

	return ret;
}

TOY_API Toy_String* Toy_createNameString(Toy_Bucket** bucketHandle, const char* cname) {
	int length = strlen(cname);

	if (length > TOY_STRING_MAX_LENGTH) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't create a name string longer than %d\n" TOY_CC_RESET, TOY_STRING_MAX_LENGTH);
		exit(-1);
	}

	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucketHandle, sizeof(Toy_String) + length + 1); //TODO: compensate for partitioning more space than bucket capacity

	ret->type = TOY_STRING_NAME;
	ret->length = length;
	ret->refCount = 1;
	ret->cachedHash = 0; //don't calc until needed
	memcpy(ret->as.name.data, cname, length + 1);
	ret->as.name.data[length] = '\0';

	return ret;
}

Toy_String* Toy_copyString(Toy_Bucket** bucketHandle, Toy_String* str) {
	if (str->refCount == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't copy a string with refcount of zero\n" TOY_CC_RESET);
		exit(-1);
	}
	incrementRefCount(str);
	return str;
}

Toy_String* Toy_deepCopyString(Toy_Bucket** bucketHandle, Toy_String* str) {
	if (str->refCount == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't deep copy a string with refcount of zero\n" TOY_CC_RESET);
		exit(-1);
	}
	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucketHandle, sizeof(Toy_String) + str->length + 1); //TODO: compensate for partitioning more space than bucket capacity

	if (str->type == TOY_STRING_NODE || str->type == TOY_STRING_LEAF) {
		ret->type = TOY_STRING_LEAF;
		ret->length = str->length;
		ret->refCount = 1;
		ret->cachedHash = str->cachedHash;
		deepCopyUtil(ret->as.leaf.data, str); //copy each leaf into the buffer
		ret->as.leaf.data[ret->length] = '\0';
	}
	else {
		ret->type = TOY_STRING_NAME;
		ret->length = str->length;
		ret->refCount = 1;
		ret->cachedHash = str->cachedHash;
		memcpy(ret->as.name.data, str->as.name.data, str->length + 1);
		ret->as.name.data[ret->length] = '\0';
	}

	return ret;
}

Toy_String* Toy_concatStrings(Toy_Bucket** bucketHandle, Toy_String* left, Toy_String* right) {
	if (left->type == TOY_STRING_NAME || right->type == TOY_STRING_NAME) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't concatenate a name string\n" TOY_CC_RESET);
		exit(-1);
	}

	if (left->refCount == 0 || right->refCount == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't concatenate a string with refcount of zero\n" TOY_CC_RESET);
		exit(-1);
	}

	if (left->length + right->length > TOY_STRING_MAX_LENGTH) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't concat a string longer than %d\n" TOY_CC_RESET, TOY_STRING_MAX_LENGTH);
		exit(-1);
	}

	Toy_String* ret = (Toy_String*)Toy_partitionBucket(bucketHandle, sizeof(Toy_String));

	ret->type = TOY_STRING_NODE;
	ret->length = left->length + right->length;
	ret->refCount = 1;
	ret->cachedHash = 0; //don't calc until needed
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
	if (str->type == TOY_STRING_NAME) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't get raw string buffer of a name string\n" TOY_CC_RESET);
		exit(-1);
	}

	if (str->refCount == 0) {
		fprintf(stderr, TOY_CC_ERROR "ERROR: Can't get raw string buffer of a string with refcount of zero\n" TOY_CC_RESET);
		exit(-1);
	}

	char* buffer = malloc(str->length + 1);

	deepCopyUtil(buffer, str);
	buffer[str->length] = '\0';

	return buffer;
}

static int deepCompareUtil(Toy_String* left, Toy_String* right, const char** leftHead, const char** rightHead) {
	//WARNING: this function can't handle strings of zero length
	int result = 0;

	//if it's the same object, of course they match
	if (left == right) {
		return result;
	}

	//BUGFIX: if we're not currently iterating through the left leaf (and leftHead is not null), skip out
	if (left->type == TOY_STRING_LEAF && (*leftHead) != NULL && (**leftHead) != '\0' && ((*leftHead) < left->as.leaf.data || (*leftHead) > (left->as.leaf.data + strlen(left->as.leaf.data))) ) {
		return result;
	}

	//BUGFIX: if we're not currently iterating through the right leaf (and rightHead is not null), skip out
	if (right->type == TOY_STRING_LEAF && (*rightHead) != NULL && (**rightHead) != '\0' && ((*rightHead) < right->as.leaf.data || (*rightHead) > (right->as.leaf.data + strlen(right->as.leaf.data))) ) {
		return result;
	}

	//dig into left
	if (left->type == TOY_STRING_NODE) {
		if ((result = deepCompareUtil(left->as.node.left, right, leftHead, rightHead)) != 0) {
			return result;
		}
		if ((result = deepCompareUtil(left->as.node.right, right, leftHead, rightHead)) != 0) {
			return result;
		}

		//return zero to keep going
		return result;
	}

	//dig into right
	if (right->type == TOY_STRING_NODE) {
		if ((result = deepCompareUtil(left, right->as.node.left, leftHead, rightHead)) != 0) {
			return result;
		}

		if ((result = deepCompareUtil(left, right->as.node.right, leftHead, rightHead)) != 0) {
			return result;
		}

		//return zero to keep going
		return result;
	}

	//keep comparing the leaves
	if (left->type == TOY_STRING_LEAF && right->type == TOY_STRING_LEAF) {
		//initial head states can be null, or null characters
		if ((*leftHead) == NULL || (**leftHead) == '\0') {
			(*leftHead) = left->as.leaf.data;
		}

		if ((*rightHead) == NULL || (**rightHead) == '\0') {
			(*rightHead) = right->as.leaf.data;
		}

		//compare and increment
		while (**leftHead && (**leftHead == **rightHead)) {
			(*leftHead)++;
			(*rightHead)++;
		}

		//if both are not null, then it's a real result
		if ( (**leftHead == '\0' || **rightHead == '\0') == false) {
			result = *(const unsigned char*)(*leftHead) - *(const unsigned char*)(*rightHead);
		}
	}

	//if either are a null character, return 0 to check the next node
	return result;
}

int Toy_compareStrings(Toy_String* left, Toy_String* right) {
	//BUGFIX: since deepCompareUtil() can't handle strings of length zero, insert a check here
	if (left->length == 0 || right->length == 0) {
		return left->length - right->length;
	}

	if (left->type == TOY_STRING_NAME || right->type == TOY_STRING_NAME) {
		if (left->type != right->type) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Can't compare a name string to a non-name string\n" TOY_CC_RESET);
			exit(-1);
		}

		return strcmp(left->as.name.data, right->as.name.data);
	}

	//util pointers
	const char* leftHead = NULL;
	const char* rightHead = NULL;

	return deepCompareUtil(left, right, &leftHead, &rightHead);
}
