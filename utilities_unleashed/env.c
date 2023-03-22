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
    if (argc <= 2) {
        print_env_usage();
        return 0;
    }

    int env_count = 0;
    int exec_count = 0;
    int exec_args = 0;
    for (int i = 1; i < argc; i++) {
        if (!strcmp(argv[i], "--")) {
            exec_args = 1;
        } else if (exec_args) {
            exec_count++;
        } else {
            env_count++;
        }
    }

    if ((env_count == 0 && exec_count == 0) || exec_args == 0) {
        print_env_usage();
        return 0;
    }

    char** env_variables = (char**) malloc(env_count * sizeof(char*));
    char** exec_inputs = (char**) malloc((exec_count + 1) * sizeof(char*));

    memcpy(env_variables, &argv[1], env_count * sizeof(char*));
    memcpy(exec_inputs, &argv[env_count + 2], exec_count * sizeof(char*));

    exec_inputs[exec_count] = NULL;

    int status;
    pid_t child = fork();
    if (child < 0) {
        free(exec_inputs);
        free(env_variables);
        print_fork_failed();
    }
    
    if (child > 0) {
        pid_t pid = waitpid(child, &status, 0);
        if (pid != -1 && WIFEXITED(status) && WEXITSTATUS(status) == 0) {

        }
    } else {
        for (int i = 0; i < env_count; i++) {
            char* curr_pair = env_variables[i];
            if (strstr(curr_pair, "=") == NULL) {
                free(exec_inputs);
                free(env_variables);   
                print_env_usage();
            }
            char* curr_key = strtok(curr_pair, "=");
            char* curr_val = strtok(NULL, "=");

            if (curr_val[0] == '%') {
                curr_val = getenv(curr_val+1);
                if (curr_val == NULL) {
                    free(exec_inputs);
                    free(env_variables);   
                    print_environment_change_failed();
                }
            }

           if(setenv(curr_key, curr_val, 1) == -1) {
                free(exec_inputs);
                free(env_variables);   
                print_environment_change_failed();
           }

        }

        execvp(exec_inputs[0], exec_inputs);
        print_exec_failed();
    }
    
    free(exec_inputs);
    free(env_variables);
    return 0;
}
