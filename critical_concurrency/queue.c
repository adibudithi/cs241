/**
 * Critical Concurrency
 * CS 241 - Fall 2019
 */
#include "queue.h"
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * This queue is implemented with a linked list of queue_nodes.
 */
typedef struct queue_node {
    void *data;
    struct queue_node *next;
} queue_node;

struct queue {
    /* queue_node pointers to the head and tail of the queue */
    queue_node *head, *tail;

    /* The number of elements in the queue */
    ssize_t size;

    /**
     * The maximum number of elements the queue can hold.
     * max_size is non-positive if the queue does not have a max size.
     */
    ssize_t max_size;

    /* Mutex and Condition Variable for thread-safety */
    pthread_cond_t cv;
    pthread_mutex_t m;
};

queue *queue_create(ssize_t max_size) {
    /* Your code here */
    queue *q = malloc(sizeof(queue));
    if (!q) return NULL;

    q->head = NULL;
    q->tail = NULL;
    q->size = 0;
    q->max_size = max_size;

    pthread_mutex_init(&(q->m), NULL);
    pthread_cond_init(&(q->cv), NULL);

    return q;
}

void queue_destroy(queue *this) {
    /* Your code here */
    if (!this) return;

    queue_node *iter = this->head;
    queue_node *curr;

    while (iter) {
        curr = iter;
        iter = iter->next;
        free(curr);
    }

    pthread_cond_destroy(&(this->cv));
    pthread_mutex_destroy(&(this->m));

    free(this);
}

void queue_push(queue *this, void *data) {
    /* Your code here */
    pthread_mutex_lock(&(this->m));

    if (this->max_size > 0) {
        while (this->size >= this->max_size) {
            pthread_cond_wait(&(this->cv), &(this->m));
        }
    }

    queue_node *node = malloc(sizeof(queue_node));
    node->data = data;
    node->next = NULL;

    if (this->size == 0) {
        this->tail = node;
        this->head = this->tail;
    } else {
        this->tail->next = node;
        this->tail = node;
    }

    this->size += 1;

    pthread_cond_broadcast(&(this->cv));
    pthread_mutex_unlock(&(this->m));
}

void *queue_pull(queue *this) {
    /* Your code here */
    pthread_mutex_lock(&(this->m));

    while (this->size <= 0) {
        pthread_cond_wait(&(this->cv), &(this->m));
    }

    queue_node *node = this->head;

    if (this->size == 1) {
        this->tail = NULL;
        this->head = this->tail;
    } else {
        this->head = node->next;
    }

    void *data = node->data;
    free(node);
    
    this->size -= 1;

    pthread_cond_broadcast(&(this->cv));
    pthread_mutex_unlock(&(this->m));

    return data;
}
