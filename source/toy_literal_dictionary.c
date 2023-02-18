#include "toy_literal_dictionary.h"

#include "toy_memory.h"

#include "toy_console_colors.h"

#include <stdio.h>

//util functions
static void setEntryValues(Toy_private_dictionary_entry* entry, Toy_Literal key, Toy_Literal value) {
	//much simpler now
	Toy_freeLiteral(entry->key);
	entry->key = Toy_copyLiteral(key);

	Toy_freeLiteral(entry->value);
	entry->value = Toy_copyLiteral(value);
}

static Toy_private_dictionary_entry* getEntryArray(Toy_private_dictionary_entry* array, int capacity, Toy_Literal key, unsigned int hash, bool mustExist) {
	//find "key", starting at index
	unsigned int index = hash % capacity;
	unsigned int start = index;

	//increment once, so it can't equal start
	index = (index + 1) % capacity;

	//literal probing and collision checking
	while (index != start) { //WARNING: this is the only function allowed to retrieve an entry from the array
		Toy_private_dictionary_entry* entry = &array[index];

		if (TOY_IS_NULL(entry->key)) { //if key is empty, it's either empty or tombstone
			if (TOY_IS_NULL(entry->value) && !mustExist) {
				//found a truly empty bucket
				return entry;
			}
			//else it's a tombstone - ignore
		} else {
			if (Toy_literalsAreEqual(key, entry->key)) {
				return entry;
			}
		}

		index = (index + 1) % capacity;
	}

	return NULL;
}

static void adjustEntryCapacity(Toy_private_dictionary_entry** dictionaryHandle, int oldCapacity, int capacity) {
	//new entry space
	Toy_private_dictionary_entry* newEntries = TOY_ALLOCATE(Toy_private_dictionary_entry, capacity);

	for (int i = 0; i < capacity; i++) {
		newEntries[i].key = TOY_TO_NULL_LITERAL;
		newEntries[i].value = TOY_TO_NULL_LITERAL;
	}

	//move the old array into the new one
	for (int i = 0; i < oldCapacity; i++) {
		if (TOY_IS_NULL((*dictionaryHandle)[i].key)) {
			continue;
		}

		//place the key and value in the new array (reusing string memory)
		Toy_private_dictionary_entry* entry = getEntryArray(newEntries, capacity, TOY_TO_NULL_LITERAL, Toy_hashLiteral((*dictionaryHandle)[i].key), false);

		entry->key = (*dictionaryHandle)[i].key;
		entry->value = (*dictionaryHandle)[i].value;
	}

	//clear the old array
	TOY_FREE_ARRAY(Toy_private_dictionary_entry, *dictionaryHandle, oldCapacity);

	*dictionaryHandle = newEntries;
}

static bool setEntryArray(Toy_private_dictionary_entry** dictionaryHandle, int* capacityPtr, int contains, Toy_Literal key, Toy_Literal value, int hash) {
	//expand array if needed
	if (contains + 1 > *capacityPtr * TOY_DICTIONARY_MAX_LOAD) {
		int oldCapacity = *capacityPtr;
		*capacityPtr = TOY_GROW_CAPACITY(*capacityPtr);
		adjustEntryCapacity(dictionaryHandle, oldCapacity, *capacityPtr); //custom rather than automatic reallocation
	}

	Toy_private_dictionary_entry* entry = getEntryArray(*dictionaryHandle, *capacityPtr, key, hash, false);

	//true = contains increase
	if (TOY_IS_NULL(entry->key)) {
		setEntryValues(entry, key, value);
		return true;
	}
	else {
		setEntryValues(entry, key, value);
		return false;
	}

	return false;
}

static void freeEntry(Toy_private_dictionary_entry* entry) {
	Toy_freeLiteral(entry->key);
	Toy_freeLiteral(entry->value);
	entry->key = TOY_TO_NULL_LITERAL;
	entry->value = TOY_TO_NULL_LITERAL;
}

static void freeEntryArray(Toy_private_dictionary_entry* array, int capacity) {
	if (array == NULL) {
		return;
	}

	for (int i = 0; i < capacity; i++) {
		if (!TOY_IS_NULL(array[i].key)) {
			freeEntry(&array[i]);
		}
	}

	TOY_FREE_ARRAY(Toy_private_dictionary_entry, array, capacity);
}

//exposed functions
void Toy_initLiteralDictionary(Toy_LiteralDictionary* dictionary) {
	//HACK: because modulo by 0 is undefined, set the capacity to a non-zero value (and allocate the arrays)
	dictionary->entries = NULL;
	dictionary->capacity = TOY_GROW_CAPACITY(0);
	dictionary->contains = 0;
	dictionary->count = 0;
	adjustEntryCapacity(&dictionary->entries, 0, dictionary->capacity);
}

void Toy_freeLiteralDictionary(Toy_LiteralDictionary* dictionary) {
	freeEntryArray(dictionary->entries, dictionary->capacity);
	dictionary->capacity = 0;
	dictionary->contains = 0;
}

void Toy_setLiteralDictionary(Toy_LiteralDictionary* dictionary, Toy_Literal key, Toy_Literal value) {
	if (TOY_IS_NULL(key)) {
		fprintf(stderr, TOY_CC_ERROR "Dictionaries can't have null keys (set)\n" TOY_CC_RESET);
		return;
	}

	//BUGFIX: Can't hash a function
	if (TOY_IS_FUNCTION(key) || TOY_IS_FUNCTION_NATIVE(key)) {
		fprintf(stderr, TOY_CC_ERROR "Dictionaries can't have function keys (set)\n" TOY_CC_RESET);
		return;
	}

	if (TOY_IS_OPAQUE(key)) {
		fprintf(stderr, TOY_CC_ERROR "Dictionaries can't have opaque keys (set)\n" TOY_CC_RESET);
		return;
	}

	const int increment = setEntryArray(&dictionary->entries, &dictionary->capacity, dictionary->contains, key, value, Toy_hashLiteral(key));

	if (increment) {
		dictionary->contains++;
		dictionary->count++;
	}
}

Toy_Literal Toy_getLiteralDictionary(Toy_LiteralDictionary* dictionary, Toy_Literal key) {
	if (TOY_IS_NULL(key)) {
		fprintf(stderr, TOY_CC_ERROR "Dictionaries can't have null keys (get)\n" TOY_CC_RESET);
		return TOY_TO_NULL_LITERAL;
	}

	//BUGFIX: Can't hash a function
	if (TOY_IS_FUNCTION(key) || TOY_IS_FUNCTION_NATIVE(key)) {
		fprintf(stderr, TOY_CC_ERROR "Dictionaries can't have function keys (get)\n" TOY_CC_RESET);
		return TOY_TO_NULL_LITERAL;
	}

	if (TOY_IS_OPAQUE(key)) {
		fprintf(stderr, TOY_CC_ERROR "Dictionaries can't have opaque keys (get)\n" TOY_CC_RESET);
		return TOY_TO_NULL_LITERAL;
	}

	Toy_private_dictionary_entry* entry = getEntryArray(dictionary->entries, dictionary->capacity, key, Toy_hashLiteral(key), true);

	if (entry != NULL) {
		return Toy_copyLiteral(entry->value);
	}
	else {
		return TOY_TO_NULL_LITERAL;
	}
}

void Toy_removeLiteralDictionary(Toy_LiteralDictionary* dictionary, Toy_Literal key) {
	if (TOY_IS_NULL(key)) {
		fprintf(stderr, TOY_CC_ERROR "Dictionaries can't have null keys (remove)\n" TOY_CC_RESET);
		return;
	}

	//BUGFIX: Can't hash a function
	if (TOY_IS_FUNCTION(key) || TOY_IS_FUNCTION_NATIVE(key)) {
		fprintf(stderr, TOY_CC_ERROR "Dictionaries can't have function keys (remove)\n" TOY_CC_RESET);
		return;
	}

	if (TOY_IS_OPAQUE(key)) {
		fprintf(stderr, TOY_CC_ERROR "Dictionaries can't have opaque keys (remove)\n" TOY_CC_RESET);
		return;
	}

	Toy_private_dictionary_entry* entry = getEntryArray(dictionary->entries, dictionary->capacity, key, Toy_hashLiteral(key), true);

	if (entry != NULL) {
		freeEntry(entry);
		entry->value = TOY_TO_BOOLEAN_LITERAL(true); //tombstone
		dictionary->count--;
	}
}

bool Toy_existsLiteralDictionary(Toy_LiteralDictionary* dictionary, Toy_Literal key) {
	//null & not tombstoned
	Toy_private_dictionary_entry* entry = getEntryArray(dictionary->entries, dictionary->capacity, key, Toy_hashLiteral(key), false);
	return !(TOY_IS_NULL(entry->key) && TOY_IS_NULL(entry->value));
}
