/**
 * Utilities Unleashed
 * CS 241 - Fall 2019
 * 
 * Partners: 
 * Robert Lou, robertl3
 * Advai Podduturi, advairp2
 * Rebecca Xun, rxun2
 * Mateusz Cikowski, mwc3
 */
#include <unistd.h>
#include "format.h"
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_time_usage();
        return 0;
    }

    char** exec_inputs = (char**) malloc((argc) * sizeof(char*));
    memcpy(exec_inputs, &argv[1], (argc - 1)*sizeof(char*));
    exec_inputs[argc - 1] = NULL;

    struct timespec * start_time = (struct timespec *) malloc(sizeof(struct timespec));
    struct timespec * end_time = (struct timespec *) malloc(sizeof(struct timespec));
    clock_gettime(CLOCK_MONOTONIC, start_time);

    int status;

    pid_t child = fork();

    if (child < 0) {
        free(exec_inputs);
        free(start_time);
        free(end_time);
        print_fork_failed();
    }
    
    if (child > 0) {
        pid_t pid = waitpid(child, &status, 0);
        if (pid != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            clock_gettime(CLOCK_MONOTONIC, end_time);
            double second_diff = end_time->tv_sec - start_time->tv_sec;
            double nano_diff = ((double) end_time->tv_nsec - (double) start_time->tv_nsec) / 1000000000;
            double time_diff = second_diff + nano_diff;
            display_results(argv, time_diff);
        }
    } else {
        execvp(exec_inputs[0], exec_inputs);
        print_exec_failed();
    }

    free(exec_inputs);
    free(start_time);
    free(end_time);
    
    return 0;
} 
