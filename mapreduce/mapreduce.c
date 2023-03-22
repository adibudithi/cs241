/**
 * mapreduce
 * CS 241 - Fall 2019
 * 
 * Partners:
 * Robert Lou - robertl3
 * Mateusz Cikowski - mwc3
 * Advai Podduturi - advairp2
 * Rebecca Xun - rxun2
 */
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>

int *mapper_pipes;
int *reducer_pipes;
pid_t *mapper_pids;
pid_t reducer_pid;

void end(int exit_code) {

    descriptors_closeall();
    descriptors_destroy();

    if (mapper_pipes)
        free(mapper_pipes);
    if (mapper_pids)
        free(mapper_pids);
    if (reducer_pipes)
        free(reducer_pipes);

    exit(exit_code);
}

void exec_process(char *process, int new_stdin, int new_stdout, pid_t *child_pid) {
    *child_pid = fork();
    if (*child_pid == -1) {
        end(2);
    }
    if (*child_pid == 0) {
        if (dup2(new_stdin, 0) == -1) {
            end(2);
        }
        if (dup2(new_stdout, 1) == -1) {
            end(2);
        }
        descriptors_closeall();
        descriptors_destroy();

        execlp(process, process, NULL);
        end(1);
    }
}

void exec_splitter(char *file, int num_piece, int piece_num, int new_stdout, pid_t *child_pid) {
    *child_pid = fork();
    if (*child_pid == -1) {
        end(2);
    }
    if (*child_pid == 0) {
        if (dup2(new_stdout, 1) == -1) {
            end(2);
        }
        descriptors_closeall();
        descriptors_destroy();
    
        char arg1[10];
        snprintf(arg1, 10, "%d", num_piece);
        char arg2[10];
        snprintf(arg2, 10, "%d", piece_num);

        execlp("./splitter", "./splitter", file, arg1, arg2, NULL);
        end(1);
    }
}

int main(int argc, char **argv) {
    // Parse inputs
    char *input_file            = argv[1];
    char *output_file           = argv[2];
    char *mapper_executable     = argv[3];
    char *reducer_executable    = argv[4];
    int num_mappers             = atoi(argv[5]);

    int output_fd = open(output_file, O_WRONLY | O_CREAT, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
    if (output_fd == -1) {
        end(1);
    }
    descriptors_add(output_fd);

    // Create an input pipe for each mapper.
    mapper_pipes = malloc(2 * num_mappers * sizeof(int));
    for (int i = 0; i < num_mappers; i++) {
        int index = i * 2;
        if (pipe2(mapper_pipes + index, O_CLOEXEC) == -1) {
            end(1);
        }
        descriptors_add(mapper_pipes[index]);
        descriptors_add(mapper_pipes[index + 1]);
    }

    // Create one input pipe for the reducer.
    int reducer_pipe[2];
    if (pipe2(reducer_pipe, O_CLOEXEC) == -1) {
        end(1);
    }
    descriptors_add(reducer_pipe[0]);
    descriptors_add(reducer_pipe[1]);

    // Start a splitter process for each mapper.
    for (int i = 0; i < num_mappers; i++) {
        pid_t splitter_pid;
        exec_splitter(input_file, num_mappers, i, mapper_pipes[(i * 2) + 1], &splitter_pid);
    }

    // Start all the mapper processes.
    mapper_pids = malloc(num_mappers * sizeof(pid_t));
    for (int i = 0; i < num_mappers; i++) {
        exec_process(mapper_executable, mapper_pipes[i * 2], reducer_pipe[1], mapper_pids + i);
    }

    // Start the reducer process.
    exec_process(reducer_executable, reducer_pipe[0], output_fd, &reducer_pid);

    descriptors_closeall();
    descriptors_destroy();

    // Wait for the reducer to finish.
    int reducer_status;
    if (waitpid(reducer_pid, &reducer_status, 0) == -1) {
        end(3);
    }

    // Print nonzero subprocess exit codes.
    for (int i = 0; i < num_mappers; i++) {
        int mapper_status;
        if (waitpid(mapper_pids[i], &mapper_status, 0) == -1) {
            end(3);
        }
        if (WIFEXITED(mapper_status)) {
            int exit_code = WEXITSTATUS(mapper_status);
            if (exit_code != 0) {
                fprintf(stdout, "%s exited with status %d\n", mapper_executable, exit_code);
            }
        }
    }

    if (WIFEXITED(reducer_status)) {
        int exit_code = WEXITSTATUS(reducer_status);
        if (exit_code != 0) {
            fprintf(stdout, "%s exited with status %d\n", reducer_executable, exit_code);
        }
    }

    // Count the number of lines in the output file.
    print_num_lines(output_file);

    // Clean up
    if (mapper_pipes)
        free(mapper_pipes);
    if (mapper_pids)
        free(mapper_pids);
    if (reducer_pipes)
        free(reducer_pipes);

    return 0;
}
