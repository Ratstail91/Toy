#pragma once

#include "toy_common.h"

typedef enum Toy_ValueType {
	TOY_VALUE_NULL,
	TOY_VALUE_BOOLEAN,
	TOY_VALUE_INTEGER,
	TOY_VALUE_FLOAT,
	TOY_VALUE_STRING,
	TOY_VALUE_ARRAY,
	TOY_VALUE_DICTIONARY,
	TOY_VALUE_FUNCTION,
	TOY_VALUE_OPAQUE,
} Toy_ValueType;

//8 bytes in size
typedef struct Toy_Value {      //32 | 64 BITNESS
	union {
		bool boolean;           //1  | 1
		int integer;            //4  | 4
		float number;           //4  | 4
		//TODO: strings
		//TODO: arrays
		//TODO: dictonaries
		//TODO: functions
		//TODO: opaque
	} as;                       //4  | 4

	Toy_ValueType type;         //4  | 4
} Toy_Value;                    //8  | 8

#define TOY_VALUE_IS_NULL(value)				((value).type == TOY_VALUE_NULL)
#define TOY_VALUE_IS_BOOLEAN(value)				((value).type == TOY_VALUE_BOOLEAN)
#define TOY_VALUE_IS_INTEGER(value)				((value).type == TOY_VALUE_INTEGER)
#define TOY_VALUE_IS_FLOAT(value)				((value).type == TOY_VALUE_FLOAT)
#define TOY_VALUE_IS_STRING(value)				((value).type == TOY_VALUE_STRING)
#define TOY_VALUE_IS_ARRAY(value)				((value).type == TOY_VALUE_ARRAY)
#define TOY_VALUE_IS_DICTIONARY(value)			((value).type == TOY_VALUE_DICTIONARY)
#define TOY_VALUE_IS_FUNCTION(value)			((value).type == TOY_VALUE_FUNCTION)
#define TOY_VALUE_IS_OPAQUE(value)				((value).type == TOY_VALUE_OPAQUE)

#define TOY_VALUE_AS_BOOLEAN(value)				((value).as.boolean)
#define TOY_VALUE_AS_INTEGER(value)				((value).as.integer)
#define TOY_VALUE_AS_FLOAT(value)				((value).as.number)
//TODO: more

#define TOY_VALUE_TO_NULL()						((Toy_Value){{ .integer = 0 }, TOY_VALUE_NULL})
#define TOY_VALUE_TO_BOOLEAN(value)				((Toy_Value){{ .boolean = value }, TOY_VALUE_BOOLEAN})
#define TOY_VALUE_TO_INTEGER(value)				((Toy_Value){{ .integer = value }, TOY_VALUE_INTEGER})
#define TOY_VALUE_TO_FLOAT(value)				((Toy_Value){{ .number = value }, TOY_VALUE_FLOAT})
//TODO: more

#define TOY_VALUE_IS_TRUTHY(value) Toy_private_isTruthy(value)
TOY_API bool Toy_private_isTruthy(Toy_Value value);

#define TOY_VALUE_IS_EQUAL(left, right) Toy_private_isEqual(left, right)
TOY_API bool Toy_private_isEqual(Toy_Value left, Toy_Value right);
