#pragma once

#include "toy_common.h"

#include "toy_bucket.h"
#include "toy_value.h"

//TODO: Remove this (related to partitioning more space in a bucket issue)
#define TOY_STRING_MAX_LENGTH 1000

//rope pattern
typedef struct Toy_String {             //32 | 64 BITNESS
	enum Toy_StringType {
		TOY_STRING_NODE,
		TOY_STRING_LEAF,
		TOY_STRING_NAME,
	} type;                             //4  | 4

	unsigned int length;                //4  | 4
	unsigned int refCount;              //4  | 4
	unsigned int cachedHash;            //4  | 4

	union {
		struct {
			struct Toy_String* left;    //4  | 8
			struct Toy_String* right;   //4  | 8
		} node;                         //8  | 16

		struct {
			int _dummy;                 //4  | 4
			char data[];                //-  | -
		} leaf;                         //4  | 4

		struct {
			Toy_ValueType type;         //4  | 4
			char data[];                //-  | -
		} name;                         //4  | 4
	} as;                               //8  | 16
} Toy_String;                           //24 | 32

TOY_API Toy_String* Toy_createString(Toy_Bucket** bucketHandle, const char* cstring);
TOY_API Toy_String* Toy_createStringLength(Toy_Bucket** bucketHandle, const char* cstring, int length);

TOY_API Toy_String* Toy_createNameString(Toy_Bucket** bucketHandle, const char* cname, Toy_ValueType type); //for variable names

TOY_API Toy_String* Toy_copyString(Toy_String* str);
TOY_API Toy_String* Toy_deepCopyString(Toy_Bucket** bucketHandle, Toy_String* str);

TOY_API Toy_String* Toy_concatStrings(Toy_Bucket** bucketHandle, Toy_String* left, Toy_String* right);

TOY_API void Toy_freeString(Toy_String* str);

TOY_API int Toy_getStringLength(Toy_String* str);
TOY_API int Toy_getStringRefCount(Toy_String* str);

TOY_API char* Toy_getStringRawBuffer(Toy_String* str); //allocates the buffer on the heap, needs to be freed

TOY_API int Toy_compareStrings(Toy_String* left, Toy_String* right); //return value mimics strcmp()

TOY_API unsigned int Toy_hashString(Toy_String* string);
