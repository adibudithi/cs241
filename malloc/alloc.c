/**
 * Malloc
 * CS 241 - Fall 2019
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static struct node* head = NULL;
static struct node* tail = NULL;

struct node {
    size_t* size;
    struct node* next;
    struct node* prev;
};

size_t get_size(void* ptr) {
    return *(size_t*) (ptr - sizeof(size_t));
}

size_t* get_size_ptr(void* ptr) {
    return (size_t*) (ptr - sizeof(size_t));
}

void* get_ptr(size_t* ptr) {
    return (void*) ((void*) ptr + sizeof(size_t));
}

void remove_node(struct node* ptr) {
    if (!ptr) return;
    if (ptr->prev && ptr->next) {
        ptr->prev->next = ptr->next;
        ptr->next->prev = ptr->prev;
    } else if (ptr->prev) {
        ptr->prev->next = NULL;
    } else if (ptr->next) {
        head = ptr->next;
        ptr->next->prev = NULL;
        ptr->next = NULL;
    } else {
        head = NULL;
    }
    *(ptr->size)--;
}

void coalesce(void* ptr) {
    if (!ptr) return;
    void* next = (ptr + (*(size_t*) ptr) - 1);
    size_t len = *(size_t*) next;
    *(size_t*) ptr = len;
}

/**
 * Allocate space for array in memory
 *
 * Allocates a block of memory for an array of num elements, each of them size
 * bytes long, and initializes all its bits to zero. The effective result is
 * the allocation of an zero-initialized memory block of (num * size) bytes.
 *
 * @param num
 *    Number of elements to be allocated.
 * @param size
 *    Size of elements.
 *
 * @return
 *    A pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory, a
 *    NULL pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/calloc/
 */
void *calloc(size_t num, size_t size) {
    // implement calloc!
    void* alloc = malloc(num * size);
    return memset(alloc, 0, num * size);
}

/**
 * Allocate memory block
 *
 * Allocates a block of size bytes of memory, returning a pointer to the
 * beginning of the block.  The content of the newly allocated block of
 * memory is not initialized, remaining with indeterminate values.
 *
 * @param size
 *    Size of the memory block, in bytes.
 *
 * @return
 *    On success, a pointer to the memory block allocated by the function.
 *
 *    The type of this pointer is always void*, which can be cast to the
 *    desired type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a null pointer is returned.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/malloc/
 */
void *malloc(size_t size) {
    struct node* curr = head;
    size_t space = size;
    if (size < sizeof(struct node)) {
        space = sizeof(struct node);
    }
    space += sizeof(size_t);      
    while (space % 8 != 0) {
        space++;
    }
    while (curr) {
        if (*(curr->size) >= space) {
            remove_node(curr);
            return get_ptr(curr->size);
        }
        curr = curr->next;
    }
    void* block = sbrk(space);
    *(size_t*) block = space;
    return get_ptr(block);
}

/**
 * Deallocate space in memory
 *
 * A block of memory previously allocated using a call to malloc(),
 * calloc() or realloc() is deallocated, making it available again for
 * further allocations.
 *
 * Notice that this function leaves the value of ptr unchanged, hence
 * it still points to the same (now invalid) location, and not to the
 * null pointer.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(),
 *    calloc() or realloc() to be deallocated.  If a null pointer is
 *    passed as argument, no action occurs.
 */
void free(void *ptr) {
    // implement free!
    if (!ptr) return;
    struct node* this = ((ptr - sizeof(size_t)) + get_size(ptr)) - sizeof(struct node);
    this->size = get_size_ptr(ptr);
    *(this->size)++;
    if (!head) {
        this->prev = NULL;
        this->next = NULL;
        head = this;
        tail = head;
        return;
    }
    this->prev = NULL;
    this->next = head;
    head->prev = this;
    head = this;
}

/**
 * Reallocate memory block
 *
 * The size of the memory block pointed to by the ptr parameter is changed
 * to the size bytes, expanding or reducing the amount of memory available
 * in the block.
 *
 * The function may move the memory block to a new location, in which case
 * the new location is returned. The content of the memory block is preserved
 * up to the lesser of the new and old sizes, even if the block is moved. If
 * the new size is larger, the value of the newly allocated portion is
 * indeterminate.
 *
 * In case that ptr is NULL, the function behaves exactly as malloc, assigning
 * a new block of size bytes and returning a pointer to the beginning of it.
 *
 * In case that the size is 0, the memory previously allocated in ptr is
 * deallocated as if a call to free was made, and a NULL pointer is returned.
 *
 * @param ptr
 *    Pointer to a memory block previously allocated with malloc(), calloc()
 *    or realloc() to be reallocated.
 *
 *    If this is NULL, a new block is allocated and a pointer to it is
 *    returned by the function.
 *
 * @param size
 *    New size for the memory block, in bytes.
 *
 *    If it is 0 and ptr points to an existing block of memory, the memory
 *    block pointed by ptr is deallocated and a NULL pointer is returned.
 *
 * @return
 *    A pointer to the reallocated memory block, which may be either the
 *    same as the ptr argument or a new location.
 *
 *    The type of this pointer is void*, which can be cast to the desired
 *    type of data pointer in order to be dereferenceable.
 *
 *    If the function failed to allocate the requested block of memory,
 *    a NULL pointer is returned, and the memory block pointed to by
 *    argument ptr is left unchanged.
 *
 * @see http://www.cplusplus.com/reference/clibrary/cstdlib/realloc/
 */
void *realloc(void *ptr, size_t size) {
    // implement realloc!
    if (!ptr) return NULL;
    if (size == 0) free(ptr);
    if (size <= get_size(ptr)) return ptr;
    void* alloc = malloc(size);
    memcpy(alloc, ptr, get_size(ptr));
    free(ptr);
    return alloc;
}
