#include "toy_common.h"

#include "toy_memory.h"

//rope pattern
typedef struct Toy_String {             //32 | 64 BITNESS
	enum Toy_StringType {
		TOY_STRING_NODE,
		TOY_STRING_LEAF,
	} type;                             //4  | 4

	int length;                         //4  | 4
	int refCount;                       //4  | 4

	union {
		struct {
			struct Toy_String* left;    //4  | 8
			struct Toy_String* right;   //4  | 8
		} node;                         //8  | 16

		struct {
			int dummy;                  //4  | 4
			char data[];                //-  | -
		} leaf;                         //4  | 4
	} as;                               //8  | 16
} Toy_String;                           //20 | 28

TOY_API Toy_String* Toy_createString(Toy_Bucket** bucket, const char* cstring);
TOY_API Toy_String* Toy_createStringLength(Toy_Bucket** bucket, const char* cstring, int length);

TOY_API Toy_String* Toy_copyString(Toy_Bucket** bucket, Toy_String* str);
TOY_API Toy_String* Toy_deepCopyString(Toy_Bucket** bucket, Toy_String* str);

TOY_API Toy_String* Toy_concatString(Toy_Bucket** bucket, Toy_String* left, Toy_String* right);

TOY_API void Toy_freeString(Toy_String* str);

TOY_API int Toy_getStringLength(Toy_String* str);
TOY_API int Toy_getStringRefCount(Toy_String* str);

TOY_API char* Toy_getStringRawBuffer(Toy_String* str); //allocates the buffer on the heap, needs to be freed

//TODO: compare