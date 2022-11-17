/*DO NOT MODIFY THIS FILE*/

#ifndef	__SCHEDULER__H__
#define	__SCHEDULER__H__

#include <pthread.h>
#include <semaphore.h>

#include "list.h"

// Structure prototypes
struct process; 
// typedef defines a new variable type process_t and you can use it to create a variable just like you would use any other variable type (i.e. int, float, etc.)
typedef struct process process_t;

struct sched_queue; 
typedef struct sched_queue sched_queue_t;

struct dispatcher;
typedef struct dispatcher dispatcher_t;

// scheduler thread function prototypes
void *long_term_scheduler(void *arg);
void *short_term_scheduler(void *arg);

/* Processes will be created with some basic info (in the order defined below):
    - The process ID
    - The process expected service time
    - The process arrival time
    - The time the process completes its execution
    - The process context (a node in a linked list)
    - Access to the CPU which should be initiality unavailable until the dispatcher schedules the process
    After creaction the process will be moved to a 
    queue where the process will wait for execution.
*/
struct process{
    int pid;
    float serviceTime;
    int arrivalTime;
    float completionTime;
    list_elem_t *context;
    sched_queue_t *queue;
    sem_t cpu_sem; 
};
/* This is a simple simulation of a dispatcher,
    it contains functions to control the execution
    of processes on the CPU, so it must be able to (in the order defined below):
    - Initialize all necessary semaphores and mutex on the scheduler queue
    - Destroy the queue and everything in it once the system is done executing processes
    - Send a signal to a process and wake it so it can control the CPU
    - Make the dispatcher wait until the process time on the CPU expires or the process terminates
*/
struct dispatcher {
    void            (*init_sched_queue)    (sched_queue_t *queue, int queue_size);
    void            (*destroy_sched_queue) (sched_queue_t *queue);
    void            (*signal_process)      (process_t *info);
    void            (*wait_for_process)    (sched_queue_t *queue);
    process_t *     (*next_process)        (sched_queue_t *queue);
    void            (*wait_for_queue)      (sched_queue_t *queue);
};

/* The queue must be able to control (in the order defined below):
    - A linked list to act as our process queue
    - A dispatcher program (this structure will control everything about the execution of processess from initializing the queue, choosing ready processes for execution and eventually deleting the queue)
    - Access to the CPU (the semaphore should allow only 1 process for execution)
    - Access to the QUEUE (this mutex should protect the integrity of the queue)
    - The max size of the queue (this sempahore should keep track of the maximum amount of processes allowed at the same time in the queue)
    - The current amount of processes in the queue (this semaphore should keep track of the current number of ready processes in the queue)
*/
struct sched_queue{
    list_t lst;
    pthread_mutex_t lock;
    dispatcher_t sched_ops;
    sem_t cpu_sem, ready_sem, sched_queue_sem;
};

// to be initialized in scheduler.c
extern dispatcher_t dispatch_rr;
extern dispatcher_t dispatch_fifo;


#endif /* __SCHEDULER__H__ */
