#include "sync.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/mman.h>

pthread_mutex_t *node_mutexes = NULL;

void init_node_locks(int node_count) {
    if (node_count > MAX_NODES) {
        node_count = MAX_NODES;
    }

    /* Allocate shared memory accessible by all forked processes */
    node_mutexes = mmap(NULL, sizeof(pthread_mutex_t) * node_count,
                        PROT_READ | PROT_WRITE,
                        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (node_mutexes == MAP_FAILED) {
        perror("Failed to allocate shared memory for mutexes");
        exit(EXIT_FAILURE);
    }

    /* Set mutex attributes to be process-shared */
    pthread_mutexattr_t attr;
    pthread_mutexattr_init(&attr);
    pthread_mutexattr_setpshared(&attr, PTHREAD_PROCESS_SHARED);

    /* Initialize each mutex */
    for (int i = 0; i < node_count; i++) {
        if (pthread_mutex_init(&node_mutexes[i], &attr) != 0) {
            perror("Failed to initialize mutex");
            exit(EXIT_FAILURE);
        }
    }

    pthread_mutexattr_destroy(&attr);
}

void lock_node(int node) {
    if (node_mutexes && node >= 0 && node < MAX_NODES) {
        pthread_mutex_lock(&node_mutexes[node]);
    }
}

void unlock_node(int node) {
    if (node_mutexes && node >= 0 && node < MAX_NODES) {
        pthread_mutex_unlock(&node_mutexes[node]);
    }
}

void destroy_node_locks(int node_count) {
    if (node_mutexes) {
        for (int i = 0; i < node_count; i++) {
            pthread_mutex_destroy(&node_mutexes[i]);
        }
        munmap(node_mutexes, sizeof(pthread_mutex_t) * node_count);
    }
}