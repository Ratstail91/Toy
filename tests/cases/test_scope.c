#include "toy_scope.h"
#include "toy_console_colors.h"

#include "toy_bucket.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>



int test_scope_allocation() {
	//allocate and free a scope
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		Toy_Scope* scope = Toy_pushScope(&bucket, NULL);

		//check
		if (scope == NULL ||
			scope->next != NULL ||
			scope->table == NULL ||
			scope->table->capacity != 16 ||
			scope->refCount != 1 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a Toy_Scope\n" TOY_CC_RESET);
			Toy_popScope(scope);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_popScope(scope);
		Toy_freeBucket(&bucket);
	}

	//allocate and free a list of scopes
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		//run
		Toy_Scope* scope = NULL;

		for (int i = 0; i < 5; i++) {
			scope = Toy_pushScope(&bucket, scope);
		}

		//check
		if (
			scope == NULL ||
			scope->next == NULL ||
			scope->table == NULL ||
			scope->table->capacity != 16 ||
			scope->refCount != 1 ||

			scope->next->next == NULL ||
			scope->next->table == NULL ||
			scope->next->table->capacity != 16 ||
			scope->next->refCount != 2 ||

			scope->next->next->next == NULL ||
			scope->next->next->table == NULL ||
			scope->next->next->table->capacity != 16 ||
			scope->next->next->refCount != 3 ||

			scope->next->next->next->next == NULL ||
			scope->next->next->next->table == NULL ||
			scope->next->next->next->table->capacity != 16 ||
			scope->next->next->next->refCount != 4 ||

			scope->next->next->next->next->next != NULL ||
			scope->next->next->next->next->table == NULL ||
			scope->next->next->next->next->table->capacity != 16 ||
			scope->next->next->next->next->refCount != 5 || //refCount includes all ancestors

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate a list of Toy_Scope\n" TOY_CC_RESET);
			while (scope) {
				scope = Toy_popScope(scope);
			}
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		while (scope) {
			scope = Toy_popScope(scope);
		}
		Toy_freeBucket(&bucket);
	}

	//ensure pops work correctly
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		//run
		Toy_Scope* scope = NULL;

		for (int i = 0; i < 5; i++) {
			scope = Toy_pushScope(&bucket, scope);
		}

		for (int i = 0; i < 2; i++) {
			scope = Toy_popScope(scope);
		}

		//check
		if (
			scope == NULL ||
			scope->next == NULL ||
			scope->table == NULL ||
			scope->table->capacity != 16 ||
			scope->refCount != 1 ||

			scope->next->next == NULL ||
			scope->next->table == NULL ||
			scope->next->table->capacity != 16 ||
			scope->next->refCount != 2 ||

			scope->next->next->next != NULL ||
			scope->next->next->table == NULL ||
			scope->next->next->table->capacity != 16 ||
			scope->next->next->refCount != 3 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate and free a list of Toy_Scope\n" TOY_CC_RESET);
			while (scope) {
				scope = Toy_popScope(scope);
			}
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		while (scope) {
			scope = Toy_popScope(scope);
		}
		Toy_freeBucket(&bucket);
	}

	//branched scope references
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		//run
		Toy_Scope* scopeBase = Toy_pushScope(&bucket, NULL);
		Toy_Scope* scopeA = Toy_pushScope(&bucket, scopeBase);
		Toy_Scope* scopeB = Toy_pushScope(&bucket, scopeBase);

		//check
		if (
			scopeBase == NULL ||
			scopeBase->next != NULL ||
			scopeBase->table == NULL ||
			scopeBase->table->capacity != 16 ||
			scopeBase->refCount != 3 ||

			scopeA == NULL ||
			scopeA->next != scopeBase ||
			scopeA->table == NULL ||
			scopeA->table->capacity != 16 ||
			scopeA->refCount != 1 ||

			scopeB == NULL ||
			scopeB->next != scopeBase ||
			scopeB->table == NULL ||
			scopeB->table->capacity != 16 ||
			scopeB->refCount != 1 ||

			scopeA->next != scopeB->next || //double check

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to allocate branched scopes\n" TOY_CC_RESET);
			Toy_popScope(scopeB);
			Toy_popScope(scopeA);
			Toy_popScope(scopeBase);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_popScope(scopeB);
		Toy_popScope(scopeA);
		Toy_popScope(scopeBase);

		Toy_freeBucket(&bucket);
	}

	//deallocate ancestors correctly
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		//run
		Toy_Scope* scopeA = Toy_pushScope(&bucket, NULL);
		Toy_Scope* scopeB = Toy_pushScope(&bucket, scopeA);
		Toy_Scope* scopeC = Toy_pushScope(&bucket, scopeB);

		Toy_popScope(scopeB);

		//check
		if (
			scopeA == NULL ||
			scopeA->next != NULL ||
			scopeA->table == NULL ||
			scopeA->table->capacity != 16 ||
			scopeA->refCount != 2 ||

			//scopeB still exists in memory until scopeC is popped
			scopeB == NULL ||
			scopeB->next != scopeA ||
			scopeB->table == NULL ||
			scopeB->table->capacity != 16 ||
			scopeB->refCount != 1 ||

			scopeC == NULL ||
			scopeC->next != scopeB ||
			scopeC->table == NULL ||
			scopeC->table->capacity != 16 ||
			scopeC->refCount != 1 ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to deallocate ancestors scopes correctly\n" TOY_CC_RESET);
			Toy_popScope(scopeC);
			Toy_popScope(scopeA);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_popScope(scopeC);
		Toy_popScope(scopeA);

		Toy_freeBucket(&bucket);
	}

	//deep copy
	{
		//setup
		Toy_Bucket* bucket = Toy_allocateBucket(TOY_BUCKET_IDEAL);

		//run
		Toy_Scope* scopeA = Toy_pushScope(&bucket, NULL);
		Toy_Scope* scopeB = Toy_pushScope(&bucket, scopeA);
		Toy_Scope* scopeCopy = Toy_deepCopyScope(&bucket, scopeB);

		//check
		if (
			scopeA == NULL ||
			scopeA->next != NULL ||
			scopeA->table == NULL ||
			scopeA->table->capacity != 16 ||
			scopeA->refCount != 3 ||

			scopeB == NULL ||
			scopeB->next != scopeA ||
			scopeB->table == NULL ||
			scopeB->table->capacity != 16 ||
			scopeB->refCount != 1 ||

			scopeB == NULL ||
			scopeB->next != scopeA ||
			scopeB->table == NULL ||
			scopeB->table->capacity != 16 ||
			scopeB->refCount != 1 ||

			scopeB == scopeCopy ||

			false)
		{
			fprintf(stderr, TOY_CC_ERROR "ERROR: Failed to deep copy a scope\n" TOY_CC_RESET);
			Toy_popScope(scopeCopy);
			Toy_popScope(scopeB);
			Toy_popScope(scopeA);
			Toy_freeBucket(&bucket);
			return -1;
		}

		//cleanup
		Toy_popScope(scopeCopy);
		Toy_popScope(scopeB);
		Toy_popScope(scopeA);

		Toy_freeBucket(&bucket);
	}

	return 0;
}

int test_scope_elements() {
	//TODO: Ensure the scope's primary function of handling key-value pairs works correctly.
	printf(TOY_CC_WARN "'test_scope_elements()' not yet implemented\n" TOY_CC_RESET);
	return 0;
}

int main() {
	//run each test set, returning the total errors given
	int total = 0, res = 0;

	{
		res = test_scope_allocation();
		if (res == 0) {
			printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	{
		res = test_scope_elements();
		if (res == 0) {
			// printf(TOY_CC_NOTICE "All good\n" TOY_CC_RESET);
		}
		total += res;
	}

	return total;
}
