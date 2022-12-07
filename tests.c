#ifndef __SCHED__TESTS__
#define __SCHED__TESTS__
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <semaphore.h>
#include "scheduler.h"
#include <pthread.h>
#include "list.h"

int testValue;
int semValue;

// Function prototypes
void *test_wait_for_queue(void *arg);
void *process_function(void *arg);
void *short_term_scheduler(void *arg);
void *long_term_scheduler(void *arg);

// Test prototypes
void init_queue_test(sched_queue_t *queue, char *argv[]);
void signal_process_test(sched_queue_t *queue, char *argv[]);
void wait_for_process_test(sched_queue_t *queue, char *argv[]);
void next_process_test(sched_queue_t *queue, char *argv[]);
void wait_for_queue_test(sched_queue_t *queue, char *argv[]);

void tests(sched_queue_t *queue, char *argv[]){
    
    // Tests inital queue values
    init_queue_test(queue, argv); // 5 pts
    
    // Test for signal_process 
    signal_process_test(queue, argv); // 5 pts

    // Test for wait_for_process
    wait_for_process_test(queue, argv); // 5 pts

    // Test for wait_for_queue
    wait_for_queue_test(queue, argv); // 5 pts

    // Test for next_process
    next_process_test(queue, argv); // 20 pts

    // If all passes this will print
    fprintf(stdout, "Congratulations! you passed all my tests, this doesn't mean the program is 100 percent correct but it means you are doing good\nyou need to manually check if the long_term_scheduler is adding processes to the queue when they arrive, waiting for processes to arrive, and terminating properly at the end of the file\n also, the long_term_scheduler and short_term_scheduler functions are accessing the queue mutually excluded\n\n");

    // long_term_scheduler // 20 pts
    // short_term_scheduler // 20 pts
}

void *test_wait_for_queue(void *arg){
	sched_queue_t *queue = (sched_queue_t*)arg;
	sleep(5);
	testValue = 1;
	sem_post(&queue->ready_sem);	
	pthread_exit(0);
}


void init_queue_test(sched_queue_t *queue, char *argv[]){
    // list test
    assert(list_size(&queue->lst) == 0 && "Linked List cannot be NULL");

    // queue cpu_sem test
    sem_getvalue(&queue->cpu_sem, &semValue);
    assert(semValue == 1 && "Schedulers should have access to CPU");

    // ready_sem test
    sem_getvalue(&queue->ready_sem, &semValue);
    assert(semValue == 0 && "There are no processes in the queue initially");

    // sched_queue_sem test
    sem_getvalue(&queue->sched_queue_sem, &semValue);
    assert(semValue == QUEUE_SIZE && "There should be QUEUE_SIZE spaces available in the LL");

    // mutex test
    assert(pthread_mutex_lock(&queue->lock) == 0 && "No one is using the queue yet, it should be unlocked");
    pthread_mutex_unlock(&queue->lock);

    fprintf(stdout, "Queue initialized correctly\n");
}

void signal_process_test(sched_queue_t *queue, char *argv[]){
    process_t *testprocess = (process_t*) malloc(sizeof(process_t));
    sem_init(&testprocess->cpu_sem, 0, 0);
    queue->sched_ops.signal_process(testprocess);
    sem_getvalue(&testprocess->cpu_sem, &semValue);
    assert(semValue == 1 && "The scheduler should signal the process cpu semaphore");
    fprintf(stdout, "Signal process pass\n");
}


void wait_for_process_test(sched_queue_t *queue, char *argv[]){
    queue->sched_ops.wait_for_process(queue);
    sem_getvalue(&queue->cpu_sem, &semValue);
    assert(semValue == 0 && "The scheduler should wait on its own cpu semaphore");
    sem_post(&queue->cpu_sem);
    fprintf(stdout, "Wait for process pass\n");
}


void wait_for_queue_test(sched_queue_t *queue, char *argv[]){
    testValue = 0;
    pthread_t *wait_for_queue_thread;
    pthread_create(&wait_for_queue_thread, 0, test_wait_for_queue, (void*)queue);
    queue->sched_ops.wait_for_queue(queue);
    assert(testValue == 1 && "The scheduler needs to wait for a single process to enter the queue if this is failing your scheduler does not actually stop for a process");
    sem_getvalue(&queue->ready_sem, &semValue);
    assert(semValue == 1 && "The scheduler should wait, but not modify permanently the value of the number of processes in the queue");
    sem_wait(&queue->ready_sem);
    fprintf(stdout, "Wait for queue pass\n");
}

void next_process_test(sched_queue_t *queue, char *argv[]){
    pthread_t test_threads[QUEUE_SIZE];

    // Populate the queue with dummy processes
    for (int i = 0; i < QUEUE_SIZE; i++){
           process_t *new_process = (process_t*) malloc(sizeof(process_t));
           new_process->serviceTime = 10;
           new_process->pid = i;
           sem_init(&new_process->cpu_sem, 0, 0);
           new_process->queue = queue;
           new_process->context = malloc(sizeof(list_elem_t));
           new_process->context->datum = new_process;
           sem_wait(&queue->sched_queue_sem);
           pthread_mutex_lock(&queue->lock);
                list_insert_tail(&queue->lst, new_process->context);
                pthread_create(&test_threads[i], NULL, process_function, (void *)new_process->context);
           pthread_mutex_unlock(&queue->lock);
           sem_post(&queue->ready_sem);
    }

    // The tests below will check the process_function and the next_process functions, and whether they are able to run and terminate correctly in either fifo or rr.
    if(!strcmp(argv[1], "-fifo")){
        process_t *info;
        pthread_t long_term_sched; // So the files are initialized
        sched_queue_t *otherqueue = (sched_queue_t*)malloc(sizeof(sched_queue_t));
        queue->sched_ops.init_sched_queue(otherqueue, 0);
        sem_wait(&otherqueue->cpu_sem); // prevent long_term from running not what is being tested here
        pthread_create(&long_term_sched, NULL, long_term_scheduler, (void *) otherqueue);
        sleep(5);
        for (int i = 0; i < QUEUE_SIZE; i++){
                sem_wait(&queue->cpu_sem);
                info = queue->sched_ops.next_process(queue);
                fprintf(stdout, "Start execution of process %d\n", info->pid);
                assert(info->pid == i && "Are you getting the front of the queue? Are you removing the node when you do?");
                queue->sched_ops.signal_process(info);
                queue->sched_ops.wait_for_process(queue);
                sem_getvalue(&queue->ready_sem, &semValue);
                assert(semValue == QUEUE_SIZE-1-i && "Processes exiting should signal they are exiting the same way they do when they enter");
                sem_getvalue(&queue->sched_queue_sem, &semValue);
                assert(semValue == 1+i && "Processes exiting should signal they are exiting the same way they do when they enter");
                sem_post(&queue->cpu_sem);
        }
	pthread_cancel(long_term_sched);
	fprintf(stdout, "Process_function and next_process_fifo pass\n");
    }
    else if(!strcmp(argv[1], "-rr")){
        process_t *info;
        pthread_t long_term_sched; // So the files are initialized
        sched_queue_t *otherqueue = (sched_queue_t*)malloc(sizeof(sched_queue_t));
        queue->sched_ops.init_sched_queue(otherqueue, 0);
        sem_wait(&otherqueue->cpu_sem); // prevent long_term from running not what is being tested here
        pthread_create(&long_term_sched, NULL, long_term_scheduler, (void *) otherqueue);
        sleep(5);
        for(int j = 0; j < 3; j++){
                for (int i = 0; i < QUEUE_SIZE; i++){
                        sem_wait(&queue->cpu_sem);
                        info = queue->sched_ops.next_process(queue);
                        fprintf(stdout, "Start execution of process %d\n", info->pid);
                        assert(info->pid == i && "Are you getting the front of the queue? Is the process adding itself at the end of the queue?");
                        queue->sched_ops.signal_process(info);
                        queue->sched_ops.wait_for_process(queue);
                        sem_post(&queue->cpu_sem);
                }
        }
	pthread_cancel(long_term_sched);
	fprintf(stdout, "Process_function and next_process_rr pass\n");
    }
}


#endif
