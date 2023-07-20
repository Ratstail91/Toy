#pragma once

/*!
# literal_array.h

This header defines the array structure, which manages a series of `Toy_Literal` instances in sequential memory. The array does not take ownership of given literals, instead it makes an internal copy.

The array type is one of two fundemental data structures used throughout Toy - the other is the dictionary.
!*/

#include "toy_common.h"

#include "toy_literal.h"

typedef struct Toy_LiteralArray {
	Toy_Literal* literals;
	int capacity;
	int count;
} Toy_LiteralArray;

/*!
## Defined Functions
!*/

/*
### void Toy_initLiteralArray(Toy_LiteralArray* array)

This function initializes a `Toy_LiteralArray` pointed to by `array`.
*/
TOY_API void Toy_initLiteralArray(Toy_LiteralArray* array);

/*!
### void Toy_freeLiteralArray(Toy_LiteralArray* array)

This function frees a `Toy_LiteralArray` pointed to by `array`. Every literal within is passed to `Toy_freeLiteral()` before its memory is released.
!*/
TOY_API void Toy_freeLiteralArray(Toy_LiteralArray* array);

/*!
### int Toy_pushLiteralArray(Toy_LiteralArray* array, Toy_Literal literal)

This function adds a new `literal` to the end of the `array`, growing the array's internal buffer if needed.

This function returns the index of the inserted value.
!*/
TOY_API int Toy_pushLiteralArray(Toy_LiteralArray* array, Toy_Literal literal);

/*!
### Toy_Literal Toy_popLiteralArray(Toy_LiteralArray* array)

This function removes the literal at the end of the `array`, and returns it.
!*/
TOY_API Toy_Literal Toy_popLiteralArray(Toy_LiteralArray* array);

/*!
### bool Toy_setLiteralArray(Toy_LiteralArray* array, Toy_Literal index, Toy_Literal value)

This function frees the literal at the position represented by the integer literal `index`, and stores `value` in its place.

This function returns true on success, otherwise it returns false.
!*/
TOY_API bool Toy_setLiteralArray(Toy_LiteralArray* array, Toy_Literal index, Toy_Literal value);

/*!
### Toy_Literal Toy_getLiteralArray(Toy_LiteralArray* array, Toy_Literal index)

This function returns the literal at the position represented by the integer literal `index`, or returns a null literal if none is found.

If `index` is not an integer literal or is out of bounds, this function returns a null literal.
!*/
TOY_API Toy_Literal Toy_getLiteralArray(Toy_LiteralArray* array, Toy_Literal index);

/*!
### int Toy_private_findLiteralIndex(Toy_LiteralArray* array, Toy_Literal literal)

This function scans through the array, and returns the index of the first element that matches the given `literal`, otherwise it returns -1.

Private functions are not intended for general use.
!*/
int Toy_private_findLiteralIndex(Toy_LiteralArray* array, Toy_Literal literal);

//TODO: add a function to get the capacity & count
