/**
 * Mini Memcheck
 * CS 241 - Fall 2019
 * Partners: Advai Podduturi, advairp2
 */
#include "mini_memcheck.h"
#include <stdio.h>
#include <string.h>

meta_data *head;
size_t total_memory_requested;
size_t total_memory_freed;
size_t invalid_addresses;

void *mini_malloc(size_t request_size, const char *filename,
                  void *instruction) {
    // your code here
    if (request_size == 0) {
        return NULL;
    }
    meta_data* ptr = (meta_data*) malloc(sizeof(meta_data) + request_size);
    if (!ptr) {
        return NULL;
    }
    ptr->request_size = request_size;
    ptr->filename = filename;
    ptr->instruction = instruction;
    if (!head) {
        ptr->next = NULL;    
    } else {
        ptr->next = head;
    }
    head = ptr;
    
    total_memory_requested += request_size;
    return (void*) (ptr + 1);
}
/*
 * Wrap a call to calloc.
 *
 * This works just like malloc, but zeros out the allocated memory.
 *
 * You may call calloc, malloc, or mini_malloc in this function,
 * but you should only do it once.
 *
 * If either the number of elements or the element size is 0, calloc's behavior
 * is undefined.
 *
 * @param num_elements
 *  Number of elements to allocate.
 * @param element_size
 *  Size of each element, in bytes.
 * @param filename
 *  Name of the file invoking this call to mini_calloc.
 * @param instruction
 *  Address of the instruction invoking this call to mini_calloc.
 *
 * @return
 *  On success, return a pointer to the memory block allocated by the function.
 *  This should be the start of the user's memory, and not the meta_data.
 *
 *  If the function fails to allocate the requested block of memory, return a
 *  NULL pointer.
 */
void *mini_calloc(size_t num_elements, size_t element_size,
                  const char *filename, void *instruction) {
    // your code here
    void * ptr = mini_malloc(num_elements * element_size, filename, instruction);
    if (!ptr) {
        return NULL;
    }
    memset(ptr, 0, element_size * num_elements);
    return ptr;
    // if (num_elements == 0 || element_size == 0) {
    //     void* ptr = mini_malloc(num_elements * element_size, filename, instruction);
    //     if (!ptr) {
    //         return NULL;
    //     }
    //     memset(ptr, 0, num_elements*element_size);
    //     return ptr;
    // }
    // meta_data* ptr = calloc(1, sizeof(meta_data) + (num_elements * element_size));
    // if (!ptr) {
    //     return NULL;
    // }
    // ptr->request_size = element_size * num_elements;
    // ptr->filename = filename;
    // ptr->instruction = instruction;
    // ptr->next = head;
    // head = ptr;
    // total_memory_requested += (element_size * num_elements);
    // return (void*) (ptr + sizeof(meta_data));
}

/*
 * Wrap a call to realloc.
 *
 * If the given pointer is NULL, you should treat this like a call to
 * mini_malloc. If the requested size is 0, you should treat this like a call
 * to mini_free and return NULL. If the pointer is NULL and the size is 0,
 * the behavior is undefined.
 *
 * In all other cases, you should use realloc to resize an existing allocation,
 * and then update the existing metadata struct with new values. If the size of
 * the allocation has increased, you must increase total_memory_requested;
 * if it has decreased, you must increase total_memory_freed. In other words,
 * these values should never decrease.
 *
 * If the user tries to realloc an invalid pointer, increment invalid_addresses
 * and return NULL.
 *
 * As with the other functions, you should only call malloc or realloc once.
 *
 * @param ptr
 *  The pointer to realloc.
 * @param request_size
 *  Size of the requested memory block, in bytes.
 * @param filename
 *  Name of the file invoking this call to mini_realloc.
 * @param instruction
 *  Address of the instruction invoking this call to mini_realloc.
 *
 * @return
 *  On success, return a pointer to the memory block allocated by the function.
 *  This should be the start of the user's memory, and not the meta_data.
 *
 *  If the function fails to allocate the requested block of memory, return a
 *  NULL pointer.
 */
void *mini_realloc(void *payload, size_t request_size, const char *filename,
                   void *instruction) {
    // your code here
    if (!payload && request_size == 0) {
        return NULL;
    }
    if (!payload) {
        void* ptr = mini_malloc(request_size, filename, instruction);
        return ptr;
    } else if (request_size == 0) {
        mini_free(payload);
        return NULL;
    }
    meta_data* curr = head;
    void * target = payload;
    // meta_data* target = (meta_data*) ((char*)payload - sizeof(meta_data));
    while (curr != NULL) {
        if (target == curr + 1) {
            if (request_size == curr->request_size) {
                curr->filename = filename;
                curr->instruction = instruction;
                return curr;
            }
            if (request_size > curr->request_size) {
                total_memory_requested += (request_size - curr->request_size);
            } else if (request_size < curr->request_size){
                total_memory_freed += (curr->request_size - request_size);
            }
            curr = realloc(curr, request_size * sizeof(meta_data));
            curr->request_size = request_size;
            curr->filename = filename;
            curr->instruction = instruction;
            return (void*) (curr + 1);
        }
        curr = curr->next;
    }
    invalid_addresses++;
    return NULL;
    // char* ptr = realloc(payload, request_size);
    // if (!ptr) {
    //     return NULL;
    // }
    // meta_data* realloc_data = (meta_data*) (ptr - sizeof(meta_data));
    // if (request_size > realloc_data->request_size) {
    //     total_memory_requested += (request_size - realloc_data->request_size);
    // } else if (request_size < realloc_data->request_size) {
    //     total_memory_freed += (realloc_data->request_size - request_size);
    // }
    // realloc_data->request_size = request_size;
    // realloc_data->filename = filename;
    // realloc_data->instruction = instruction;
    // return (void*) (ptr + sizeof(meta_data));
}
/*
 * Wrap a call to free.
 *
 * This free will also remove the metadata node from the list, assuming it is
 * a valid pointer.
 *
 * Unlike the regular free, you should not crash when given an invalid pointer.
 * Instead, increment invalid_addresses.
 *
 * Invalid pointers include pointers that you did not return from mini_malloc
 * (or mini_calloc, or mini_realloc), and double frees.
 *
 * @param ptr
 *  Pointer to a previously allocated memory block. If a NULL pointer is
 *  passed, no action occurs, and it does not count as a bad free.
 */
void mini_free(void *payload) {
    // your code here
    if (!payload) {
        return;
    }
    meta_data* target = (meta_data*) ((char*)payload - sizeof(meta_data));
    meta_data* curr = head;
    meta_data* prev = NULL;
    while (curr != NULL) {
        if (target == curr) {
            if (prev) {
                prev->next = curr->next;
            } else {
                head = curr->next;
            }
            total_memory_freed += curr->request_size;
            free(curr);
            return;
            
        }
        prev = curr;
        curr = curr->next;
    }
    invalid_addresses++;

    // if (!payload) {
    //     return;
    // }
    // meta_data* curr = head;
    // while (curr != NULL) {
    //     if ((curr + sizeof(meta_data)) == payload) {
    //         break;
    //     } else {
    //         invalid_addresses++;
    //     }
    // }
}
