#include "toy_table.h"
#include "toy_console_colors.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//'count' actually tracks the number of values
#define MIN_CAPACITY 16

//utils
static Toy_Table* adjustTableCapacity(Toy_Table* oldTable, unsigned int newCapacity) {
	//allocate and zero a new table in memory
	Toy_Table* newTable = malloc(newCapacity * sizeof(Toy_TableEntry) + sizeof(Toy_Table));

	newTable->capacity = newCapacity;
	newTable->count = 0;
	newTable->minPsl = 0;
	newTable->maxPsl = 0;

	//unlike other structures, the empty space in a table needs to be null
	memset(newTable + 1, 0, newTable->capacity * sizeof(Toy_TableEntry));

	if (oldTable == NULL) { //for initial allocations
		return newTable;
	}

	//for each entry in the old table, copy it into the new table
	for (int i = 0; i < oldTable->capacity; i++) {
		Toy_insertTable(&newTable, oldTable->data[i].key, oldTable->data[i].value);
	}

	//clean up and return
	free(oldTable);
	return newTable;
}

//exposed functions
Toy_Table* Toy_allocateTable() {
	return adjustTableCapacity(NULL, MIN_CAPACITY);
}

void Toy_freeTable(Toy_Table* table) {
	//TODO: slip in a call to free the complex values here

	free(table);
}

void Toy_insertTable(Toy_Table** table, Toy_Value key, Toy_Value value) {
	if (TOY_VALUE_IS_NULL(key) || TOY_VALUE_IS_BOOLEAN(key)) { //TODO: disallow functions and opaques
		fprintf(stderr, TOY_CC_ERROR "ERROR: Bad table key\n" TOY_CC_RESET);
		exit(-1); //TODO: #127
	}

	//expand the capacity
	if ((*table)->capacity < (*table)->count * (1 / 0.75f)) {
		(*table) = adjustTableCapacity(*table, (*table)->capacity * 2);
	}

	//insert
	unsigned int probe = Toy_hashValue(key) % (*table)->capacity;
	Toy_TableEntry entry = (Toy_TableEntry){ .key = key, .value = value, .psl = 0 };

	while (true) {
		//if this spot is free, insert and return
		if (TOY_VALUE_IS_NULL((*table)->data[probe].key)) {
			(*table)->data[probe] = entry;

			(*table)->count++;

			//TODO: benchmark the psl optimisation
			(*table)->minPsl = entry.psl < (*table)->minPsl ? entry.psl : (*table)->minPsl;
			(*table)->maxPsl = entry.psl > (*table)->maxPsl ? entry.psl : (*table)->maxPsl;

			return;
		}

		//if the new entry is "poorer", insert it and shift the old one
		if ((*table)->data[probe].psl < entry.psl) {
			Toy_TableEntry tmp = (*table)->data[probe];
			(*table)->data[probe] = entry;
			entry = tmp;
		}

		//adjust and continue
		probe = (probe + 1) % (*table)->capacity;
		entry.psl++;
	}
}

Toy_Value Toy_lookupTableValue(Toy_Table** table, Toy_Value key) {
	if (TOY_VALUE_IS_NULL(key) || TOY_VALUE_IS_BOOLEAN(key)) { //TODO: disallow functions and opaques
		fprintf(stderr, TOY_CC_ERROR "ERROR: Bad table key\n" TOY_CC_RESET);
		exit(-1); //TODO: #127
	}

	//lookup
	unsigned int probe = Toy_hashValue(key) % (*table)->capacity;
	unsigned int counter = 0;

	while (true) {
		//found the entry
		if (TOY_VALUE_IS_EQUAL((*table)->data[probe].key, key)) {
			return (*table)->data[probe].value;
		}

		//if the psl is too big, or empty slot
		if ((*table)->data[probe].psl > counter || TOY_VALUE_IS_NULL((*table)->data[probe].key)) {
			return TOY_VALUE_TO_NULL();
		}

		//adjust and continue
		probe = (probe + 1) % (*table)->capacity;
		counter++;
	}
}

void Toy_removeTableEntry(Toy_Table** table, Toy_Value key) {
	if (TOY_VALUE_IS_NULL(key) || TOY_VALUE_IS_BOOLEAN(key)) { //TODO: disallow functions and opaques
		fprintf(stderr, TOY_CC_ERROR "ERROR: Bad table key\n" TOY_CC_RESET);
		exit(-1); //TODO: #127
	}

	//lookup
	unsigned int probe = Toy_hashValue(key) % (*table)->capacity;
	unsigned int counter = 0;
	unsigned int wipe = probe; //wiped at the end

	while (true) {
		//found the entry
		if (TOY_VALUE_IS_EQUAL((*table)->data[probe].key, key)) {
			break;
		}

		//if the psl is too big, or empty slot
		if ((*table)->data[probe].psl > counter || TOY_VALUE_IS_NULL((*table)->data[probe].key)) {
			return;
		}

		//adjust and continue
		probe = (probe + 1) % (*table)->capacity;
		counter++;
	}

	//shift down the later entries (past the probing point)
	for (unsigned int i = (*table)->minPsl; i < (*table)->maxPsl; i++) {
		unsigned int p = (probe + i + 0) % (*table)->capacity; //prev
		unsigned int u = (probe + i + 1) % (*table)->capacity; //current

		//if the psl is too big, or an empty slot, stop
		if ((*table)->data[u].psl > (counter + i) || TOY_VALUE_IS_NULL((*table)->data[u].key)) {
			break;
		}

		(*table)->data[p] = (*table)->data[u];
		(*table)->data[p].psl--;
		wipe = wipe % (*table)->capacity;
	}

	//finally, wipe the removed entry
	(*table)->data[wipe] = (Toy_TableEntry){ .key = TOY_VALUE_TO_NULL(), .value = TOY_VALUE_TO_NULL(), .psl = 0 };
}