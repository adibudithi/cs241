/**
 * Parallel Make Lab
 * CS 241 - Fall 2019
 */
 
#include "format.h"
#include "graph.h"
#include "parmake.h"
#include "parser.h"
#include "vector.h"
#include "queue.h"
#include "set.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h> 
#include <pthread.h>

static int DEFAULT = 0;
static int UNPROCESSED = 1;
static int FAILED = 2;
static int SUCCEEDED = 3;
static int IGNORED = 4;
static int PROCESSING = 5;

graph* nodes;
queue* tasks;
set* visited = NULL;
set* valid_goals;
int goal_total;
int goal_count;

pthread_mutex_t lock; 

// printf("hello\n");
// printf("target: %s\n", rule->target);
// printf("goal_total: %d\n", goal_total);
// printf("goal_count: %d\n", goal_count);

int get_state(rule_t* rule) {
    pthread_mutex_lock(&lock);
    int state = rule->state;
    pthread_mutex_unlock(&lock);
    return state;
}

void set_state(rule_t* rule, int state) {
    pthread_mutex_lock(&lock);
    rule->state = state;
    pthread_mutex_unlock(&lock);
}

int cycles(char* node) {
    rule_t* rule = (rule_t*) graph_get_vertex_value(nodes, node);
    int state = get_state(rule);
    if (state == IGNORED) {
        return IGNORED;
    }
    vector* neighbors = graph_neighbors(nodes, node);
    set_state(rule, UNPROCESSED);
    VECTOR_FOR_EACH(neighbors, neighbor, {
        if (set_contains(visited, neighbor)) {
            state = IGNORED;
            vector_destroy(neighbors);
            return IGNORED;
        }
        set_add(visited, neighbor);
        if (cycles(neighbor) == IGNORED) {
            set_state(rule, IGNORED);
            vector_destroy(neighbors);
            return IGNORED;
        }
        set_remove(visited, neighbor);
    });
    vector_destroy(neighbors);
    return UNPROCESSED;
}

void check_failed_dependencies(rule_t* rule) {
    vector* dependencies = graph_neighbors(nodes, rule->target);
    VECTOR_FOR_EACH(dependencies, dependency, {
        rule_t* dep = (rule_t *) graph_get_vertex_value(nodes, dependency);
        int dep_state = get_state(dep);
        if (dep_state == FAILED) {
            set_state(rule, FAILED);
            break;
        }
    });
    vector_destroy(dependencies);
}

int changed(rule_t* rule) {
    vector* dependencies = graph_neighbors(nodes, rule->target);
    struct stat old;
    if (stat(rule->target, &old) == 0) {
        VECTOR_FOR_EACH(dependencies, dependency, {
            struct stat new;
            if (!stat(dependency, &new)) {
                if (difftime(old.st_mtime, new.st_mtime) >= 0) {
                    vector_destroy(dependencies);
                    return 0;
                }
            }
        });
    }
    vector_destroy(dependencies);
    return 1;
}

void execute(rule_t* rule) {
    VECTOR_FOR_EACH(rule->commands, command, {
        if (system(command)) {
            set_state(rule, FAILED);
            break;
        }
    });
    int state = get_state(rule);
    if (state != FAILED) {
        set_state(rule, SUCCEEDED);
    }
}

void push_failed_antineighbors(rule_t* rule) {
    vector* antineighbors = graph_antineighbors(nodes, rule->target);
    VECTOR_FOR_EACH(antineighbors, antineighbor, {
        rule_t* anti = (rule_t *) graph_get_vertex_value(nodes, antineighbor);
        queue_push(tasks, anti);
    });
    vector_destroy(antineighbors);
}

void push_successful_antineighbors(rule_t* rule) {
    vector* antineighbors = graph_antineighbors(nodes, rule->target);
    VECTOR_FOR_EACH(antineighbors, antineighbor, {
        rule_t* anti = (rule_t *) graph_get_vertex_value(nodes, antineighbor);
        int anti_state = get_state(anti);
        if (anti_state == DEFAULT) {
            continue;
        }
        vector* antidependencies = graph_neighbors(nodes, anti->target);
        int good = 1;
        VECTOR_FOR_EACH(antidependencies, antidependency, {
            rule_t* antidep = (rule_t *) graph_get_vertex_value(nodes, antidependency);
            int antidep_state = get_state(antidep);
            if (antidep_state != SUCCEEDED) {
                good = 0;
            }
        });
        if (good) {
            queue_push(tasks, anti);
        }
        vector_destroy(antidependencies);
    });
    vector_destroy(antineighbors);
}

void* rope() {
    rule_t* rule;
    while ((rule = queue_pull(tasks))) {
        int state = get_state(rule);
        if (state == PROCESSING) {
            continue;
        } else {
            set_state(rule, PROCESSING);
        }
        check_failed_dependencies(rule);
        if (changed(rule)) {
            execute(rule);
        } else {
            set_state(rule, SUCCEEDED);
            goal_count++;
        }
        state = get_state(rule);
        if (state == FAILED) {
            push_failed_antineighbors(rule);
        } else if (state == SUCCEEDED) {
            push_successful_antineighbors(rule);
        }
        if (set_contains(valid_goals, rule->target)) {
            pthread_mutex_lock(&lock);
            goal_count++;
            pthread_mutex_unlock(&lock);
        }
        if (goal_count == goal_total) {
            queue_push(tasks, NULL);;
        }
    }
    queue_push(tasks, NULL);
    return NULL;
}

void load_rules() {
    vector* all_vertices = graph_vertices(nodes);
    vector* goals = graph_neighbors(nodes, "");
    goal_total = vector_size(goals);
    goal_count = 0;

    VECTOR_FOR_EACH(goals, target, {
        // printf("target: %s\n", (char*) target);
        rule_t* rule = (rule_t *) graph_get_vertex_value(nodes, target);
        int status = cycles(target);
        if (status == IGNORED) {
            goal_total--;
            print_cycle_failure(rule->target);
        } else if (status == UNPROCESSED) {
            set_add(valid_goals, target);
        }
    });

    if (!goal_total) {
        queue_push(tasks, NULL);
    }

    VECTOR_FOR_EACH(all_vertices, target, {
        vector* neighbors = graph_neighbors(nodes, target);
        rule_t* rule = (rule_t *) graph_get_vertex_value(nodes, target);
        int state = get_state(rule);
        if (vector_empty(neighbors) && state == UNPROCESSED) {
            queue_push(tasks, rule);
        }
        vector_destroy(neighbors);
    });
}

int parmake(char *makefile, size_t num_threads, char **targets) {
    // good luck!
    nodes = parser_parse_makefile(makefile, targets);
    tasks = queue_create(-1);
    valid_goals = string_set_create();
    visited = string_set_create();

    load_rules();

    pthread_t threads[num_threads];
    for (size_t i = 0; i < num_threads; i++) {
        pthread_create(&threads[i], NULL, rope, NULL); 
    }

    for (size_t i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }

    queue_destroy(tasks);
    graph_destroy(nodes);
    set_destroy(valid_goals);
    set_destroy(visited);
    return 0;
}
