 /* Teaching Threads
 * CS 241 - Fall 2019
 * Partners: Advai Podduturi (advairp2)
 */
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "reduce.h"
#include "reducers.h"
typedef struct thread_data {
    reducer funct;
    int start;
    int end;
    int* list;
    int reduced_output;
} thread_data;

/* You might need a struct for each task ... */

/* You should create a start routine for your threads. */
void par_reduce_start_routine(void* data) {
    thread_data* thread = (thread_data*) data;
    for (int i = thread->start; i < thread->end; i++) {
        // printf("start: %d   end: %d\n", thread->start, thread->end);
        // printf("val1: %d    val2: %d\n", thread->reduced_output, thread->list[i]);
        thread->reduced_output = thread->funct(thread->reduced_output, thread->list[i]);
    }

    // printf("ret = %d\n", thread->reduced_output);
    // printf("\n");
    // pthread_exit((void*) &thread->reduced_output);

}

int par_reduce(int *list, size_t list_len, reducer reduce_func, int base_case,
               size_t num_threads) {
    /* Your implementation goes here */
    if (!list) {
        return 0;
    }
    // for (size_t i = 0; i < list_len; i++) {
        // printf("list[%zu] = %d\n", i, list[i]);
    // }
    if (num_threads > list_len) {
        num_threads = list_len / 2;
    }
    int offset = list_len / num_threads;
    int start = 0;
    int end = 0;
    int* list_copy = malloc(sizeof(int) * list_len);
    memcpy(list_copy, list, list_len);
    thread_data* threads = malloc(sizeof(thread_data) * num_threads);
    pthread_t * ids = malloc(sizeof(pthread_t) * num_threads);
    for (size_t i = 0; i < num_threads; i++) {
        if (i == num_threads - 1) {
            end = list_len;
        } else {
            end = start + offset;
        }
        threads[i].funct = reduce_func;
        threads[i].start = start;
        threads[i].end = end;
        threads[i].list = list;
        threads[i].reduced_output = base_case;
        
        pthread_create(ids + i, NULL, (void*) &par_reduce_start_routine, (void*)(threads + i));
        
        start = end;
    }
    // for (size_t i = 0; i < num_threads; i++) {
        
    // }
    int sum = base_case; 
    for (size_t i = 0; i < num_threads; i++) {
        pthread_join(ids[i], NULL);
        sum = reduce_func(sum, threads[i].reduced_output);
        // printf("sum = %d    tid = %lu\n", threads[i].reduced_output, ids[i]);
    }
    // printf("FINAL SUM = %d\n", sum);
    free(threads);
    free(ids);
    free(list_copy);
    return sum;
}
