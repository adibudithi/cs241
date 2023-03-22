/**
 * Shell
 * CS 241 - Fall 2019
 */
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include "format.h"
#include "shell.h"
#include "vector.h"
#include "sstring.h"

static vector* history;
static vector* args;
static char* script_file_name;
static char* history_file_name;
static char* history_file_path;
static char* line;
static size_t buffer;
static FILE* stream;
static char buf[PATH_MAX];
static int op;
static vector* args1;
static vector* args2;

typedef struct process {
    char *command;
    pid_t pid;
} process;

void yeet(char* target) {
    char* temp = strchr(target, '\n');
    if (temp) *temp = '\0';
}

char* skeet(char* target) {
    int i = 0;
    while(target[i] != '\0') i++;
    target[i - 1] = '\0';
    return target;
}

void get_args(char* line) {
    yeet(line);
    sstring* s = cstr_to_sstring(line);
    args = sstring_split(s, ' ');
    args1 = string_vector_create();
    args2 = string_vector_create();
    sstring_destroy(s);
    op = 0;
    for (size_t i = 0; i < vector_size(args); i++) {
        yeet(vector_get(args, i));
        if (!strcmp(vector_get(args, i), "&&")) {
            op = 1;
            size_t j = 0;
            while (j < i) {
                vector_push_back(args1, vector_get(args, j));
                j++;
            }
            j++;
            while (j < vector_size(args)) {
                vector_push_back(args2, vector_get(args, j));
                j++;
            }
        } else if (!strcmp(vector_get(args, i), "||")) {
            op = 2;
            size_t j = 0;
            while (j < i) {
                vector_push_back(args1, vector_get(args, j));
                j++;
            }
            j++;
            while (j < vector_size(args)) {
                vector_push_back(args2, vector_get(args, j));
                j++;
            }
        } else if (* (char*) (vector_get(args, i) + strlen(vector_get(args, i)) - 1) == ';') {
            op = 3;
            skeet(vector_get(args, i));
            size_t j = 0;
            while (j < i) {
                vector_push_back(args1, vector_get(args, j));
                j++;
            }
            while (j < vector_size(args)) {
                vector_push_back(args2, vector_get(args, j));
                j++;
            }
        }
    }
}      

void read_history() {
    FILE* history_file = fopen(history_file_name, "r");
    if (history_file) {
        while (getline(&line, &buffer, history_file) != -1) {
            yeet(line);
            vector_push_back(history, line);
        }
        fclose(history_file);
    } else {
        print_history_file_error();
    }
}

void write_history() {
    FILE* history_file = fopen(history_file_path, "w");
    for (size_t i = 0; i < vector_size(history); i++) {
        fwrite(strcat(vector_get(history, i), "\n"), 1, strlen(vector_get(history, i)), history_file);
    }
    fclose(history_file);
}

void print_history() {
    for (size_t i = 0; i < vector_size(history); i++) {
        print_history_line(i, vector_get(history, i));
    }
}

int change_directory(vector* args) {
    if (vector_size(args) == 1) return 0;
    if (chdir(vector_get(args, 1)) == -1) {
        print_no_directory(vector_get(args, 1));
        return 0;
    }
    return 1;
}

int nth_cmd() {
    size_t num;
    sscanf(vector_get(args, 0) + 1, "%zd", &num);
    if (num < vector_size(history)) {
        char* past = vector_get(history, num);
        get_args(past);
        vector_push_back(history, past);
        return 1;
    } else {
        print_invalid_index();
        return 0;
    }
}

int prefix() {
    if (strlen((char*) vector_get(args, 0)) == 1) {
        if (vector_size(history) > 0) {
            get_args(*vector_back(history));
            vector_push_back(history, *vector_back(history));
            return 1;
        } else {
            print_no_history_match();
            return 0;
        }
    } else {
        for (int i = (int) vector_size(history) - 1; i >= 0; i--) {
            char* query = line + 1;
            char temp[strlen(query) + 1];
            memset(temp, '\0', sizeof(temp));
            temp[strlen(query)] = '\0';
            strncpy(temp, vector_get(history, i), strlen(query));
            if (!strcmp(query, temp)) {
                get_args(vector_get(history, i));
                vector_push_back(history, vector_get(history, i));
                return 1;
            }
        }
        print_no_history_match();
    }
    return 0;
}

int spoon(vector* args) {
    if (!strcmp(vector_get(args, 0), "cd")) {
        vector_push_back(history, line);
        return change_directory(args);
    } 
    int status;
    pid_t pid;
    pid = fork();
    if (pid == 0) {
        // child
        char** argt = calloc(vector_size(args) + 1, sizeof(char*));
        for (size_t i = 0; i < vector_size(args); i++) {
            argt[i] = vector_get(args, i);
        }
        argt[vector_size(args)] = NULL;
        print_command_executed(getpid());
        execvp(argt[0], argt);
        print_exec_failed(argt[0]);
        free(argt);
        exit(1);
    } else if (pid > 0) {
        // parent
        pid = waitpid(pid, &status, 0);
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            return 1;
        } else {
            return 0;
        }
    } else {
        print_fork_failed();
        return 0;
    }
}

void end_shell() {
    if (history_file_path) write_history();
    vector_destroy(history);
    free(line);
}

void end_loop() {
    vector_destroy(args);
    vector_destroy(args1);
    vector_destroy(args2);
}

void loop() {
    if (script_file_name) {
        stream = fopen(script_file_name, "r");
    } else {
        stream = stdin;
    }
    while (1) {
        print_prompt(getcwd(buf, PATH_MAX), getpid());
        if (getline(&line, &buffer, stream) == -1) return;
        if (!strcmp(line, "\n")) continue;
        if (script_file_name) printf("%s", line);
        get_args(line);
        if (!strcmp(vector_get(args, 0), "exit")) {
            end_loop();
            break;
        } else if (!strcmp(vector_get(args, 0), "!history")) {
            print_history();
        } else if (* (char*) vector_get(args, 0) == '#') {
            if (nth_cmd()) {
                spoon(args);
            }
        } else if (* (char*) vector_get(args, 0) == '!') {
            if (prefix()) {
                spoon(args);
            }
        } else {
            vector_push_back(history, line);
            if (op == 1) {
                if (spoon(args1)) spoon(args2);
            } else if (op == 2) {
                if (!spoon(args1)) spoon(args2);
            } else if (op == 3) {
                spoon(args1);
                spoon(args2);
            } else {
                spoon(args);
            }
        }
        end_loop();
    }
}

int shell(int argc, char *argv[]) {
    // TODO: This is the entry point for your shell.
    history = string_vector_create();
    int flag = getopt(argc, argv, "f:h:");

    while (flag != -1) {
        switch (flag) {
            case 'f':
                script_file_name = optarg;
                break;
            case 'h':
                history_file_name = optarg;
                history_file_path = get_full_path(history_file_name);
                read_history();
                break;
            default:
                print_usage();
                return 0;
        }
        flag = getopt(argc, argv, "f:h:");
    }
    if (script_file_name == NULL && history_file_name == NULL && argc > 1) {;
        print_usage();
    } else {
        loop();
        end_shell();
    }
    return 0;
}