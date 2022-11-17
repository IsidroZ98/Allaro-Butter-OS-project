/*DO NOT MODIFY THIS FILE*/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "scheduler.h"

// Modify this value to change the maximum number of ready processes in queue at the same time.
const int QUEUE_SIZE = 10;

int main(int argc, char *argv[]){

    // the short_term_scheduler thread schedules the execution of processes on the CPU
    pthread_t short_term_scheduler_thread;
    // the long_term_scheduler thread reads the file and create processes to add into the scheduler queue
    pthread_t long_term_scheduler_thread;

    // initialize the scheduler_queue structure
    sched_queue_t *queue = (sched_queue_t*) malloc(sizeof(sched_queue_t));

    // add operations and initialize queue's variables
    if(!strcmp(argv[1], "-fifo")){
	queue->sched_ops = dispatch_fifo;
    }
    else if(!strcmp(argv[1], "-rr")){
        queue->sched_ops = dispatch_rr;
    }
    else{
    	fprintf(stderr, "Must provide scheduling algorithm -fifo or -rr");
        return EXIT_FAILURE;
    }
    queue->sched_ops.init_sched_queue(queue, QUEUE_SIZE);

    // create threads and assign their correponding function
    if(pthread_create(&long_term_scheduler_thread, NULL, long_term_scheduler, (void*)queue)){
        return EXIT_FAILURE;
    }
    if(pthread_create(&short_term_scheduler_thread, NULL, short_term_scheduler, (void*)queue)){
        return EXIT_FAILURE;
    }

    // detach threads so they can execute freely
    pthread_detach(long_term_scheduler_thread);

    // wait until all processes finish their execution
    pthread_join(short_term_scheduler_thread, NULL);

    // destroy queue
    queue->sched_ops.destroy_sched_queue(queue);

    return EXIT_SUCCESS;
}
