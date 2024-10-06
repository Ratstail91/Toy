#include "toy_string.h"
#include "toy_console_colors.h"

#include "toy_bucket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int test_sizeof_string_64bit() {
	//test for the correct size
	{
		if (sizeof(Toy_String) != 32) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'Toy_String' is an unexpected size in memory: expected 32, found %d \n" TOY_CC_RESET, (int)sizeof(Toy_String));
			return -1;
		}
	}

	return 0;
}

int test_sizeof_string_32bit() {
	//test for the correct size
	{
		if (sizeof(Toy_String) != 24) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: 'Toy_String' is an unexpected size in memory: expected 24, found %d \n" TOY_CC_RESET, (int)sizeof(Toy_String));
			return -1;
		}
	}

	return 0;
}

int test_string_allocation() {
	//allocate a single string from a c-string
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);

		const char* cstring = "Hello world";
		Toy_String* str = Toy_createString(&bucket, cstring);

		//check
		if (str->type != TOY_STRING_LEAF ||
			str->length != 11 ||
			str->refCount != 1 ||
			strcmp(str->as.leaf.data, "Hello world") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a Toy_String with a private bucket\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//free the string
		Toy_freeString(str);

		//inspect the bucket
		if (bucket->capacity != 1024 ||
			bucket->count != sizeof(Toy_String) + 12 ||
			bucket->next != NULL)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected bucket state after a string was freed\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//inspect the (now freed) string's memory
		if (Toy_getStringRefCount(str) != 0) {
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected string state after it was freed\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		Toy_freeBucket(&bucket);
	}

	//copy and deep copy a string
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);

		const char* cstring = "Hello world";
		Toy_String* str = Toy_createString(&bucket, cstring);

		//shallow and deep
		Toy_String* shallow = Toy_copyString(&bucket, str);
		Toy_String* deep = Toy_deepCopyString(&bucket, str);

		if (str != shallow ||
			str == deep ||
			shallow->refCount != 2 ||
			deep->refCount != 1 ||
			strcmp(shallow->as.leaf.data, deep->as.leaf.data) != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to copy a string correctly\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		Toy_freeBucket(&bucket);
	}

	//allocate a zero-length string
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);

		const char* cstring = "";
		Toy_String* str = Toy_createString(&bucket, cstring);

		//check
		if (str->type != TOY_STRING_LEAF ||
			str->length != 0 ||
			str->refCount != 1 ||
			strcmp(str->as.leaf.data, "") != 0 ||
			bucket->count != sizeof(Toy_String) + 1) //1 for the null character
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a Toy_String with zero length\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//free the string
		Toy_freeString(str);

		Toy_freeBucket(&bucket);
	}

	return 0;
}

int test_string_concatenation() {
	//one big bucket o' fun
	Toy_Bucket* bucket = Toy_allocateBucket(1024);

	//concatenate two strings, and check the refcounts
	{
		//setup
		Toy_String* first = Toy_createString(&bucket, "Hello ");
		Toy_String* second = Toy_createString(&bucket, "world");

		//concatenate
		Toy_String* result = Toy_concatStrings(&bucket, first, second);

		//check the refcounts
		if (first->refCount != 2 ||
			second->refCount != 2 ||
			result->refCount != 1 ||
			result->length != 11)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected state for string refcounts after concatenation\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//clean up the separate strings
		Toy_freeString(first);
		Toy_freeString(second);

		//check the refcounts again
		if (first->refCount != 1 ||
			second->refCount != 1 ||
			result->refCount != 1 ||
			result->length != 11)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected state for string refcounts after concatenation and free\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//clean up
		Toy_freeString(result);
	}

	//concatenate two strings, and check the resulting buffer
	{
		//setup
		Toy_String* first = Toy_createString(&bucket, "Hello ");
		Toy_String* second = Toy_createString(&bucket, "world");

		//concatenate
		Toy_String* result = Toy_concatStrings(&bucket, first, second);

		char* buffer = Toy_getStringRawBuffer(result);

		//check the refcounts
		if (strlen(buffer) != 11 ||
			strcmp(buffer, "Hello world") != 0)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to get the raw buffer from concatenated string\n" TOY_CC_RESET);
			free(buffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		free(buffer);
		Toy_freeString(result);
		Toy_freeString(first);
		Toy_freeString(second);
	}

	Toy_freeBucket(&bucket);
	return 0;
}

int test_string_with_stressed_bucket() {
	//how much is that dog in the window?
	{
		//test data: 36 characters total, 44 with spaces
		char* testData[] = {
			"the",
			"quick",
			"brown",
			"fox",
			"jumped", //longest word: 6 characters
			"over",
			"the",
			"lazy",
			"dog", //9 entries long
			NULL,
		};

		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(128);//deliberately too much data for one bucket

		//stress
		Toy_String* str = Toy_createString(&bucket, testData[0]);
		Toy_String* ptr = str;
		for (int i = 1; testData[i]; i++) {
			str = Toy_concatStrings(&bucket, str, Toy_createString(&bucket, testData[i]));
		}

		//check
		if (ptr->refCount != 9 ||
			str->length != 36)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected state of the string after stress test\n" TOY_CC_RESET);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//grab the buffer
		char* buffer = Toy_getStringRawBuffer(str);

		if (strcmp(buffer, "thequickbrownfoxjumpedoverthelazydog") != 0 ||
			strlen(buffer) != 36)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected state of the raw buffer after string stress test: '%s'\n" TOY_CC_RESET, buffer);
			free(buffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		if (bucket->next == NULL) //just to make sure
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Unexpected state of the bucket after string stress test\n" TOY_CC_RESET);
			free(buffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//clean up
		free(buffer);
		Toy_freeBucket(&bucket);
	}

	//
	return 0;
}

int test_string_equality() {
	//simple string equality (no concats)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_createString(&bucket, "Hello world");
		Toy_String* helloWorldTwo = Toy_createString(&bucket, "Hello world");
		Toy_String* helloEveryone = Toy_createString(&bucket, "Hello everyone");

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch
		if ((result = Toy_compareStrings(helloWorldOne, helloEveryone)) == 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check match (same object)
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldOne)) != 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloWorldOne);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality '%s' == '%s' is incorrect, found %s (these are the same object)\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string equality (with concat left)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_concatStrings(&bucket, Toy_createString(&bucket, "Hello "), Toy_createString(&bucket, "world"));
		Toy_String* helloWorldTwo = Toy_createString(&bucket, "Hello world");
		Toy_String* helloEveryone = Toy_createString(&bucket, "Hello everyone");

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat left equality '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch
		if ((result = Toy_compareStrings(helloWorldOne, helloEveryone)) == 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat left equality '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check match (same object)
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldOne)) != 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloWorldOne);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat left equality '%s' == '%s' is incorrect, found %s (these are the same object)\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string equality (with concat right)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_createString(&bucket, "Hello world");
		Toy_String* helloWorldTwo = Toy_concatStrings(&bucket, Toy_createString(&bucket, "Hello "), Toy_createString(&bucket, "world"));
		Toy_String* helloEveryone = Toy_createString(&bucket, "Hello everyone");

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat right equality '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string equality (with concat both)
	{
		//setup - these concat points are deliberately different
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_concatStrings(&bucket, Toy_createString(&bucket, "Hello "), Toy_createString(&bucket, "world"));
		Toy_String* helloWorldTwo = Toy_concatStrings(&bucket, Toy_createString(&bucket, "Hello"), Toy_createString(&bucket, " world"));
		Toy_String* helloEveryone = Toy_concatStrings(&bucket, Toy_createString(&bucket, "Hell"), Toy_createString(&bucket, "world"));

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat both equality '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch
		if ((result = Toy_compareStrings(helloWorldOne, helloEveryone)) == 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat both equality '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string equality (with concat arbitrary)
	{
		//setup - The quick brown fox jumps over the lazy dog.
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_concatStrings(&bucket,
			Toy_createString(&bucket, "The quick brown "),
			Toy_concatStrings(&bucket,
				Toy_createString(&bucket, "fox jumps o"),
				Toy_createString(&bucket, "ver the lazy dog.")
			)
		);

		Toy_String* helloWorldTwo = Toy_concatStrings(&bucket,
			Toy_concatStrings(&bucket,
				Toy_createString(&bucket, "The quick brown fox"),
				Toy_concatStrings(&bucket,
					Toy_createString(&bucket, " jumps ove"),
					Toy_createString(&bucket, "r th")
				)
			),
			Toy_concatStrings(&bucket,
				Toy_createString(&bucket, "e lazy dog"),
				Toy_createString(&bucket, ".")
			)
		);

		Toy_String* helloEveryone = Toy_concatStrings(&bucket,
			Toy_createString(&bucket, "The quick brown fox jumps over "),
			Toy_concatStrings(&bucket,
				Toy_createString(&bucket, "the lazy "),
				Toy_createString(&bucket, "reddit mod.")
			)
		);

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat arbitrary equality '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch
		if ((result = Toy_compareStrings(helloWorldOne, helloEveryone)) == 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String concat arbitrary equality '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string equality (empty strings, no concats)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_createString(&bucket, "");
		Toy_String* helloWorldTwo = Toy_createString(&bucket, "");
		Toy_String* helloEveryone = Toy_createString(&bucket, "Hello everyone");

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality empty '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch
		if ((result = Toy_compareStrings(helloWorldOne, helloEveryone)) == 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality empty '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch (same object)
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldOne)) != 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloWorldOne);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality empty '%s' == '%s' is incorrect, found %s (these are the same object)\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string equality (empty strings, deep concats)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloWorldOne = Toy_concatStrings(&bucket,
			Toy_createString(&bucket, ""),
			Toy_concatStrings(&bucket,
				Toy_createString(&bucket, ""),
				Toy_createString(&bucket, "")
			)
		);

		Toy_String* helloWorldTwo = Toy_concatStrings(&bucket,
			Toy_concatStrings(&bucket,
				Toy_createString(&bucket, ""),
				Toy_concatStrings(&bucket,
					Toy_createString(&bucket, ""),
					Toy_createString(&bucket, "")
				)
			),
			Toy_concatStrings(&bucket,
				Toy_createString(&bucket, ""),
				Toy_createString(&bucket, "")
			)
		);

		Toy_String* helloEveryone = Toy_concatStrings(&bucket,
			Toy_createString(&bucket, "The quick brown fox jumps over "),
			Toy_concatStrings(&bucket,
				Toy_createString(&bucket, "the lazy "),
				Toy_createString(&bucket, "reddit mod.")
			)
		);

		int result = 0; //for print the errors

		//check match
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldTwo)) != 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloWorldTwo);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality empty with concats '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check mismatch
		if ((result = Toy_compareStrings(helloWorldOne, helloEveryone)) == 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality empty with concats '%s' != '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check match (same object)
		if ((result = Toy_compareStrings(helloWorldOne, helloWorldOne)) != 0)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloWorldOne);
			char* rightBuffer = Toy_getStringRawBuffer(helloWorldOne);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String equality empty with concats '%s' == '%s' is incorrect, found %s (these are the same object)\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	return 0;
}

int test_string_diffs() {
	//simple string diffs (no concats)
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* helloEveryone = Toy_createString(&bucket, "Hello everyone");
		Toy_String* helloUniverse = Toy_createString(&bucket, "Hello universe");

		int result = 0; //for print the errors

		//check diff
		if (((result = Toy_compareStrings(helloEveryone, helloUniverse)) < 0) == false)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloEveryone);
			char* rightBuffer = Toy_getStringRawBuffer(helloUniverse);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String diff '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check diff (reversed)
		if (((result = Toy_compareStrings(helloUniverse, helloEveryone)) > 0) == false)
		{
			char* leftBuffer = Toy_getStringRawBuffer(helloUniverse);
			char* rightBuffer = Toy_getStringRawBuffer(helloEveryone);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String diff '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	//string diffs (with concat arbitrary)
	{
		//setup - The quick brown fox jumps over the lazy dog.
		Toy_Bucket* bucket = Toy_allocateBucket(1024);
		Toy_String* pangram = Toy_concatStrings(&bucket,
			Toy_createString(&bucket, "The quick brown "),
			Toy_concatStrings(&bucket,
				Toy_createString(&bucket, "fox jumps o"),
				Toy_createString(&bucket, "ver the lazy dog.")
			)
		);

		Toy_String* neckbeard = Toy_concatStrings(&bucket,
			Toy_createString(&bucket, "The quick brown fox jumps over "),
			Toy_concatStrings(&bucket,
				Toy_createString(&bucket, "the lazy "),
				Toy_createString(&bucket, "reddit mod.")
			)
		);

		int result = 0; //for print the errors

		//check diff
		if (((result = Toy_compareStrings(pangram, neckbeard)) < 0) == false)
		{
			char* leftBuffer = Toy_getStringRawBuffer(pangram);
			char* rightBuffer = Toy_getStringRawBuffer(neckbeard);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String diff '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//check diff (reversed)
		if (((result = Toy_compareStrings(neckbeard, pangram)) > 0) == false)
		{
			char* leftBuffer = Toy_getStringRawBuffer(neckbeard);
			char* rightBuffer = Toy_getStringRawBuffer(pangram);
			fprintf(stderr, TOY_CC_ERROR "ERROR: String diff '%s' == '%s' is incorrect, found %s\n" TOY_CC_RESET, leftBuffer, rightBuffer, result < 0 ? "<" : result == 0 ? "==" : ">");
			free(leftBuffer);
			free(rightBuffer);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_freeBucket(&bucket);
	}

	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
#if TOY_BITNESS == 64
		res = test_sizeof_string_64bit();
#else
		res = test_sizeof_string_32bit();
#endif
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_string_allocation();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_string_concatenation();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_string_with_stressed_bucket();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_string_equality();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_string_diffs();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
