/**
 * Password Cracker
 * CS 241 - Fall 2019
 */
#include "cracker1.h"
#include "format.h"
#include "utils.h"
#include "string.h"
#include "crypt.h"
#include "includes/queue.h"
#include <stdio.h>
#include <stdlib.h>

static size_t n_size = 9;
static size_t h_size = 14;
static size_t p_size = 9;
static queue* jobs;

struct job {
    char* name;
    char* hash;
    char* password;
};

void* crack(void* id) {
    int num_found = 0;
    double start_time = getThreadCPUTime();
    struct job* task;

    while ((task = queue_pull(jobs)) != NULL) {
        if (!task) return 0;
        
        int found = 0;

        v1_print_thread_start(*(int*) id, task->name);
        char* key = strdup(task->password);
        struct crypt_data cdata;
        cdata.initialized = 0;
        int hash_count = 1;
        int length = getPrefixLength(key);
        setStringPosition(key + length, 0);
        
        while (incrementString(key + length)) {
            hash_count++;
            if (!strcmp(crypt_r(key, "xx", &cdata), task->hash)) {
                found = 1;
                break;
            }
        }

        if (found) {
            v1_print_thread_result(*(int*) id, task->name, key, hash_count, getThreadCPUTime() - start_time, 0);
            num_found++;
        } else {
            v1_print_thread_result(*(int*) id, task->name, key, hash_count, getThreadCPUTime() - start_time, 1);
        }

        free(key);
        free(task->name);
        free(task->hash);
        free(task->password);
        free(task);
    }

    queue_push(jobs, NULL);
    int* count = malloc(sizeof(int));
    *count = num_found;

    free(id);

    return (void*) count;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    pthread_t ids[thread_count];
    jobs = queue_create(-1);
    char n_buf[n_size];
    char h_buf[h_size];
    char p_buf[p_size];
    int total = 0;
    int cracked = 0;

    while (scanf("%s %s %s", n_buf, h_buf, p_buf) != -1) {
        struct job* task = malloc(sizeof(struct job));
        task->name = strdup(n_buf);
        task->hash = strdup(h_buf);
        task->password = strdup(p_buf);
        queue_push(jobs, task);
        total++;
    }

    queue_push(jobs, NULL);

    for (size_t i = 0; i < thread_count; i++) {
        int* id = malloc(sizeof(int));
        *id = i + 1;
        pthread_create(&ids[i], NULL, &crack, id);
    }

    void* num;
    for (size_t i = 0; i < thread_count; i++) {
        pthread_join(ids[i], &num);
        cracked += *(int*) num;
        free(num);
    }

    v1_print_summary(cracked, total - cracked);

    queue_destroy(jobs);

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
