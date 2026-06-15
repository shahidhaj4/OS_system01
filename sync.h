#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>

#ifndef MAX_NODES
#define MAX_NODES 100
#endif

/* Pointer to the mutex array in shared memory */
extern pthread_mutex_t *node_mutexes;

/* Functions to initialize and clean up locks */
void init_node_locks(int node_count);
void destroy_node_locks(int node_count);

/* Functions to lock and unlock specific nodes */
void lock_node(int node);
void unlock_node(int node);

#endif // SYNC_H