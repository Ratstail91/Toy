# toy_memory.h

This header defines all of the memory management utilities. Any and all heap-based memory management goes through these utilities.

A default memory allocator function is used internally, but it can be overwritten for diagnostic and platform related purposes.

## Defined Macros

### TOY_GROW_CAPACITY(capacity)

This macro calculates, in place, what size of memory should be allocated based on the previous size.

### TOY_GROW_CAPACITY_FAST(capacity)

This macro calculates, in place, what size of memory should be allocated based on the previous size. It grows faster than `TOY_GROW_CAPACITY`.

### TOY_ALLOCATE(type, count)

This macro wraps `Toy_reallocate`, which itself calls the allocator function. `type` is the type that will be allocated, and `count` is the number which will be needed (usually calculated with `TOY_GROW_CAPACITY`).

This returns a pointer of `type`.

### TOY_FREE(type, pointer)

This macro wraps `Toy_reallocate`, which itself calls the allocator function. `type` is the type that will be freed, and `pointer` is to what is being freed. This should only be used when a single element has been allocated, as opposed to an array.

### TOY_FREE_ARRAY(type, pointer, oldCount)

This macro wraps `Toy_reallocate`, which itself calls the allocator function. `type` is the type that will be freed, `pointer` is a reference to what is being freed, and `oldCount` is the size of the array being freed. This should only be used when an array has been allocated, as opposed to a single element.

### TOY_GROW_ARRAY(type, pointer, oldCount, count)

This macro wraps `Toy_reallocate`, which itself calls the allocator function. `type` is the type that is being operated on, `pointer` is what is being resized, `oldCount` is the previous size of the array and `count` is the new size of the array (usually calculated with `TOY_GROW_CAPACITY`).

This returns a pointer of `type`.

### TOY_SHRINK_ARRAY(type, pointer, oldCount, count)

This macro wraps `Toy_reallocate`, which itself calls the allocator function. `type` is the type that is being operated on, `pointer` is what is being resized, `oldCount` is the previous size of the array and `count` is the new size of the array.

This returns a pointer of `type`.

## Defined Interfaces

### typedef void* (*Toy_MemoryAllocatorFn)(void* pointer, size_t oldSize, size_t newSize)

This function interface is used for defining any memory allocator functions.

Any and all memory allocator functions should:

* Take a `pointer` to a previously allocated block of memory, or `NULL`
* Take the `oldSize`, which is the previous size of the `pointer` allocated, in bytes (`oldSize` can be 0)
* Take the `newSize`, which is the new size of the buffer to be allocaated, in bytes (`newSize` can be 0)
* Return the newly allocated buffer, or `NULL` if `newSize` is zero
* Return `NULL` on error

## Defined Functions

### TOY_API void* Toy_reallocate(void* pointer, size_t oldSize, size_t newSize)

This function shouldn't be called directly. Instead, use one of the given macros.

This function wraps a call to the internal assigned memory allocator.

### void Toy_setMemoryAllocator(Toy_MemoryAllocatorFn)

This function sets the memory allocator, replacing the default memory allocator.
