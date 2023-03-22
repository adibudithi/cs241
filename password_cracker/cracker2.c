/**
 * Password Cracker
 * CS 241 - Fall 2019
 */
#include "cracker2.h"
#include "format.h"
#include "utils.h"
#include "string.h"
#include "crypt.h"
#include <stdio.h>
#include <stdlib.h>

static size_t n_size = 9;
static size_t h_size = 14;
static size_t p_size = 9;
static int found;
static char* username;
static char* solution;
static int hashcount;
static double startTime;

struct job {
    int* id;
    long* count;
    long* offset;
    char* name;
    char* hash;
    char* password;
};

void murk(struct job* this) {
    free(this->id);
    free(this->offset);
    free(this->name);
    free(this->hash);
    free(this->password);
    free(this);
}

void* crack(void* job) {
    int yes = 0;
    struct job* task = (struct job*) job;
    int len = getPrefixLength(task->password);
    setStringPosition(task->password + len, *(task->offset));
    v2_print_thread_start(*(task->id), task->name, *(task->offset), task->password);
    struct crypt_data cdata;
    cdata.initialized = 0;

    for (long i = 0; i < *(task->count); i++) {
        if (found) {
            v2_print_thread_result(*(task->id), i, 1);
            yes = 1;
            break;
        }
        if (!strcmp(crypt_r(task->password, "xx", &cdata), task->hash)) {
            v2_print_thread_result(*(task->id), i, 0);
            solution = task->password;
            found = 1;
            yes = 1;
            break;
        }
        incrementString(task->password + len);
        hashcount++;
    }
    if (!yes) v2_print_thread_result(*(task->id), *(task->count), 0);
    murk(task);
    return (void*) task->password;
}

int start(size_t thread_count) {
    // TODO your code here, make sure to use thread_count!
    // Remember to ONLY crack passwords in other threads
    pthread_t ids[thread_count];
    char name[n_size];
    char hash[h_size];
    char password[p_size];

    while (scanf("%s %s %s", name, hash, password) != -1) {
        v2_print_start_user(name);
        found = 0;
        username = NULL;
        solution = NULL;
        hashcount = 0;
        startTime = getCPUTime();

        for (size_t i = 0; i < thread_count; i++) {
            struct job* task = malloc(sizeof(struct job));
            task->name = strdup(name);
            task->hash = strdup(hash);
            task->password = strdup(password);
            task->id = malloc(sizeof(int));
            *(task->id) = i + 1;
            task->count = malloc(sizeof(long));
            task->offset = malloc(sizeof(long));

            getSubrange(strlen(password) - getPrefixLength(password), thread_count, *(task->id), task->offset, task->count);
            
            pthread_create(&ids[i], NULL, &crack, task);
        }

        for (size_t i = 0; i < thread_count; i++) {
            pthread_join(ids[i], NULL);
        }  
        if (found) {
            v2_print_summary(name, solution, hashcount, getCPUTime() - startTime, getCPUTime(), 0);
        } else {
            v2_print_summary(name, solution, hashcount, getCPUTime() - startTime, getCPUTime(), 1);
        }
    }

    return 0; // DO NOT change the return code since AG uses it to check if your
              // program exited normally
}
