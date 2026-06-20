#include "sync.h"
#include "queue.h"
#include "scheduler.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>


ScheduleType    g_schedule_type = SCHED_FCFS;
pthread_mutex_t *node_mutexes   = NULL;
pthread_cond_t  *node_conds     = NULL;
NodeQueue       *node_queues    = NULL;


static int *node_occupied   = NULL;
static int *fcfs_order_ctr  = NULL;


static pthread_mutex_t *order_mutex = NULL;


void init_node_locks(int node_count) {
    if (node_count > MAX_NODES) {
        node_count = MAX_NODES;
    }

    size_t mutexes_sz = sizeof(pthread_mutex_t) * node_count;
    size_t conds_sz    = sizeof(pthread_cond_t)  * node_count;
    size_t queues_sz   = sizeof(NodeQueue)        * node_count;
    size_t occupied_sz = sizeof(int)              * node_count;

    node_mutexes = mmap(NULL, mutexes_sz, PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    node_conds = mmap(NULL, conds_sz, PROT_READ | PROT_WRITE,
                      MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    node_queues = mmap(NULL, queues_sz, PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    node_occupied = mmap(NULL, occupied_sz, PROT_READ | PROT_WRITE,
                         MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    fcfs_order_ctr = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                          MAP_SHARED | MAP_ANONYMOUS, -1, 0);
    order_mutex = mmap(NULL, sizeof(pthread_mutex_t), PROT_READ | PROT_WRITE,
                       MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (node_mutexes == MAP_FAILED || node_conds == MAP_FAILED ||
        node_queues == MAP_FAILED || node_occupied == MAP_FAILED ||
        fcfs_order_ctr == MAP_FAILED || order_mutex == MAP_FAILED) {
        perror("Failed to allocate shared memory for sync structures");
        exit(EXIT_FAILURE);
    }


    pthread_mutexattr_t mattr;
    pthread_mutexattr_init(&mattr);
    pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED);

    pthread_condattr_t cattr;
    pthread_condattr_init(&cattr);
    pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED);

    for (int i = 0; i < node_count; i++) {
        if (pthread_mutex_init(&node_mutexes[i], &mattr) != 0) {
            perror("Failed to initialize node mutex");
            exit(EXIT_FAILURE);
        }
        if (pthread_cond_init(&node_conds[i], &cattr) != 0) {
            perror("Failed to initialize node condvar");
            exit(EXIT_FAILURE);
        }
        init_node_queue(&node_queues[i]);
        node_occupied[i] = 0;
    }

    if (pthread_mutex_init(order_mutex, &mattr) != 0) {
        perror("Failed to initialize order mutex");
        exit(EXIT_FAILURE);
    }
    *fcfs_order_ctr = 0;

    pthread_mutexattr_destroy(&mattr);
    pthread_condattr_destroy(&cattr);
}


static int next_fcfs_ticket(void) {
    pthread_mutex_lock(order_mutex);
    int ticket = (*fcfs_order_ctr)++;
    pthread_mutex_unlock(order_mutex);
    return ticket;
}


void lock_node(int node, int traveler_id, int next_edge_weight) {
    if (!node_mutexes || node < 0 || node >= MAX_NODES) return;

    int order = next_fcfs_ticket();

    pthread_mutex_lock(&node_mutexes[node]);

    enqueue_traveler(&node_queues[node], traveler_id, order,
                     next_edge_weight, 1);

    while (1) {
        int next_id = get_next_traveler(&node_queues[node], g_schedule_type);

        if (!node_occupied[node] && next_id == traveler_id) {

            remove_traveler(&node_queues[node], traveler_id);
            node_occupied[node] = 1;
            break;
        }


        pthread_cond_wait(&node_conds[node], &node_mutexes[node]);
    }

    pthread_mutex_unlock(&node_mutexes[node]);
}


void unlock_node(int node, int traveler_id) {
    if (!node_mutexes || node < 0 || node >= MAX_NODES) return;
    (void)traveler_id; /* not strictly needed to free the node, kept for symmetry/logging */

    pthread_mutex_lock(&node_mutexes[node]);
    node_occupied[node] = 0;
    pthread_cond_broadcast(&node_conds[node]);
    pthread_mutex_unlock(&node_mutexes[node]);
}


void destroy_node_locks(int node_count) {
    if (node_count > MAX_NODES) {
        node_count = MAX_NODES;
    }

    if (node_mutexes) {
        for (int i = 0; i < node_count; i++) {
            pthread_mutex_destroy(&node_mutexes[i]);
        }
        munmap(node_mutexes, sizeof(pthread_mutex_t) * node_count);
        node_mutexes = NULL;
    }

    if (node_conds) {
        for (int i = 0; i < node_count; i++) {
            pthread_cond_destroy(&node_conds[i]);
        }
        munmap(node_conds, sizeof(pthread_cond_t) * node_count);
        node_conds = NULL;
    }

    if (node_queues) {
        munmap(node_queues, sizeof(NodeQueue) * node_count);
        node_queues = NULL;
    }

    if (node_occupied) {
        munmap(node_occupied, sizeof(int) * node_count);
        node_occupied = NULL;
    }

    if (order_mutex) {
        pthread_mutex_destroy(order_mutex);
        munmap(order_mutex, sizeof(pthread_mutex_t));
        order_mutex = NULL;
    }

    if (fcfs_order_ctr) {
        munmap(fcfs_order_ctr, sizeof(int));
        fcfs_order_ctr = NULL;
    }
}