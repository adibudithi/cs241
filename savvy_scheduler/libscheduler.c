/**
 * Savvy Scheduler
 * CS 241 - Fall 2019
 * Partners:
 * Advai Podduturi - advairp2
 */

#include "libpriqueue/libpriqueue.h"
#include "libscheduler.h"

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "print_functions.h"

/**
 * The struct to hold the information about a given job
 */
typedef struct _job_info {
    /* TODO: Add any other information and bookkeeping you need into this
     * struct. */
    int id;
    int priority;
    double rt;
    double rr_time;
    double xxtime;
    double et;
    double ut;
    double st;
} job_info;

int job_count;
double twt;
double ttat;
double trt;

priqueue_t pqueue;
comparer_t comparision_func;
scheme_t pqueue_scheme;

void scheduler_start_up(scheme_t s) {
    switch (s) {
    case FCFS:
        comparision_func = comparer_fcfs;
        break;
    case PRI:
        comparision_func = comparer_pri;
        break;
    case PPRI:
        comparision_func = comparer_ppri;
        break;
    case PSRTF:
        comparision_func = comparer_psrtf;
        break;
    case RR:
        comparision_func = comparer_rr;
        break;
    case SJF:
        comparision_func = comparer_sjf;
        break;
    default:
        printf("Did not recognize scheme\n");
        exit(1);
    }
    priqueue_init(&pqueue, comparision_func);
    pqueue_scheme = s;
    // Put any additional set up code you may need here
}

static int break_tie(const void *a, const void *b) {
    return comparer_fcfs(a, b);
}

int comparer_fcfs(const void *a, const void *b) {
    // TODO: Implement me!
    job_info* aa = ((job*)a)->metadata;
    job_info* bb = ((job*)b)->metadata;
    // not sure if this is working
    if ( aa->xxtime > bb->xxtime ) 
        return 1;
    if ( bb->xxtime > aa->xxtime )
        return -1;

    return 0;
}

int comparer_ppri(const void *a, const void *b) {
    // Complete as is
    return comparer_pri(a, b);
}

int comparer_pri(const void *a, const void *b) {
    // TODO: Implement me!
    job_info* aa = ((job*)a)->metadata;
    job_info* bb = ((job*)b)->metadata;

    if ( aa->priority > bb->priority ) 
        return 1;
    if ( bb->priority > aa->priority )
        return -1;
    return break_tie(a, b);
}

int comparer_psrtf(const void *a, const void *b) {
    // TODO: Implement me!
    job_info* aa = ((job*)a)->metadata;
    job_info* bb = ((job*)b)->metadata;

    if ( (aa->rt - aa->ut) > (bb->rt - bb->ut) ) 
        return 1;
    if ( (bb->rt - bb->ut) > (aa->rt - aa->ut) )
        return -1;
    return break_tie(a, b);
}

int comparer_rr(const void *a, const void *b) {
    // TODO: Implement me!
    job_info* aa = ((job*)a)->metadata;
    job_info* bb = ((job*)b)->metadata;

    if ( aa->rr_time > bb->rr_time ) 
        return 1;
    if ( bb->rr_time > aa->rr_time )
        return -1;
    return 0;
}

int comparer_sjf(const void *a, const void *b) {
    // TODO: Implement me!
    job_info* aa = ((job*)a)->metadata;
    job_info* bb = ((job*)b)->metadata;

    if ( aa->rt > bb->rt ) 
        return 1;
    if ( bb->rt > aa->rt )
        return -1;
    
    return break_tie(a, b);
}

// Do not allocate stack space or initialize ctx. These will be overwritten by
// gtgo
void scheduler_new_job(job *newjob, int job_number, double time,
                       scheduler_info *sched_data) {
    // TODO: Implement me!
    job_info* tempjob = malloc(sizeof(job_info));
    newjob->metadata = tempjob;

    tempjob->xxtime = time;
    tempjob->priority = sched_data->priority;
    tempjob->id = job_number;
    tempjob->rt = sched_data->running_time;

    tempjob->st = 0;
    tempjob->rr_time = 0;
    tempjob->ut = 0;
    tempjob->et = 0;

    priqueue_offer(&pqueue, newjob);
}

job *scheduler_quantum_expired(job *job_evicted, double time) {
    // TODO: Implement me!
    job_info *temp;
    job_info *next_temp;
    switch (pqueue_scheme) {
        case PSRTF:
            // do nothing
        case PPRI:
            // do nothing
        case RR:
            if (job_evicted) {
                temp = job_evicted->metadata;
                temp->ut += time - temp->rr_time;
                priqueue_offer(&pqueue, job_evicted);
            }

            job *next_job = priqueue_poll(&pqueue);
            if (next_job) {
                next_temp = next_job->metadata;
                next_temp->rr_time = time;
                if (next_temp->ut == 0)
                    next_temp->st = time;
            }
            return next_job;
            // is this working??
        default:
            if (!job_evicted) {
                job *x = priqueue_poll(&pqueue);
                if (x) {
                    next_temp = x->metadata;
                    next_temp->rr_time = time;
                    if (next_temp->ut == 0) {
                        next_temp->st = time;
                    }
                }
                return x;
            } else
                return job_evicted;
    }
}

void scheduler_job_finished(job *job_done, double time) {
    // TODO: Implement me!
    job_info* x = (job_info*)(job_done->metadata);
    x->ut += time - x->rr_time;    
    x->et = time;
    ttat += (x->et - x->xxtime);
    twt += (x->et - x->xxtime - x->rt);
    trt += (x->st - x->xxtime);
    job_count++;
    free(job_done->metadata);
}

static void print_stats() {
    fprintf(stderr, "turnaround     %f\n", scheduler_average_turnaround_time());
    fprintf(stderr, "total_waiting  %f\n", scheduler_average_waiting_time());
    fprintf(stderr, "total_response %f\n", scheduler_average_response_time());
}

double scheduler_average_waiting_time() {
    // TODO: Implement me!
    return (double) twt/job_count;
}

double scheduler_average_turnaround_time() {
    // TODO: Implement me!
    return (double) ttat/job_count;
}

double scheduler_average_response_time() {
    // TODO: Implement me!
    return (double) trt/job_count;
}

void scheduler_show_queue() {
    // OPTIONAL: Implement this if you need it!
    // dont need this
}

void scheduler_clean_up() {
    priqueue_destroy(&pqueue);
    print_stats();
}