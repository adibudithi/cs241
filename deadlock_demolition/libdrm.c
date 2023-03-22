/**
 * Deadlock Demolition
 * CS 241 - Fall 2019
 * 
 * Advai Podduturi, advairp2
 * Mateusz Cikowski - mwc3
 * Robert Lou, robertl3
 * Rebecca Xun, rxun2
 */
#include "graph.h"
#include "libdrm.h"
#include "set.h"
#include <pthread.h>

struct drm_t {
    pthread_mutex_t mut;
};

graph* rag;
pthread_mutex_t global_mut = PTHREAD_MUTEX_INITIALIZER;
set* visited = NULL;

drm_t *drm_init() {
    drm_t* drm = malloc(sizeof(drm_t));
    pthread_mutex_init(&(drm->mut), NULL);
    pthread_mutex_lock(&global_mut);
    if (!rag) { 
        rag = shallow_graph_create(); 
    }
    graph_add_vertex(rag, drm);
    pthread_mutex_unlock(&global_mut);
    return drm;
}

int drm_post(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&global_mut);
    int drm_in_rag = graph_contains_vertex(rag, drm);
    int id_in_rag = graph_contains_vertex(rag, thread_id);
    
    if (!drm_in_rag || !id_in_rag || !graph_adjacent(rag, drm, thread_id)) {
        pthread_mutex_unlock(&global_mut);
        return 0;
    }

    graph_remove_edge(rag, drm, thread_id);
    pthread_mutex_unlock(&drm->mut);
    pthread_mutex_unlock(&global_mut);
    return 1;
}

int contains_cycle(void* node) {
    if (visited == NULL) {
        visited = shallow_set_create();
    }
    if (set_contains(visited, node)) {
        visited = NULL;
        return 1;
    } else {
        set_add(visited, node);
        vector* neighbors = graph_neighbors(rag, node);
        for (size_t i = 0; i < vector_size(neighbors); i++) {
            if (contains_cycle(vector_get(neighbors, i))) {
                return 1;
            }
        }
    }
    visited = NULL;
    return 0;
}

int drm_wait(drm_t *drm, pthread_t *thread_id) {
    pthread_mutex_lock(&global_mut);
    graph_add_vertex(rag, thread_id);

    graph_add_edge(rag, thread_id, drm);
    if (graph_adjacent(rag, drm, thread_id) || contains_cycle(thread_id)) {
        graph_remove_edge(rag, thread_id, drm);
        pthread_mutex_unlock(&global_mut);
        return 0;
    }

    pthread_mutex_unlock(&global_mut);
    pthread_mutex_lock(&drm->mut);
    
    pthread_mutex_lock(&global_mut);
    
    graph_remove_edge(rag, thread_id, drm);
    graph_add_edge(rag, drm, thread_id);
    
    pthread_mutex_unlock(&global_mut);
    return 1;
}

void drm_destroy(drm_t *drm) {
    pthread_mutex_destroy(&(drm->mut));
    pthread_mutex_destroy(&global_mut);
    graph_remove_vertex(rag, drm);
    free(drm);
}