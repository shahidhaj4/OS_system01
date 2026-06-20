#ifndef SYNC_H
#define SYNC_H

#include <pthread.h>
#include "scheduler.h"   /* brings in ScheduleType + queue.h (NodeQueue) */

#ifndef MAX_NODES
#define MAX_NODES 100
#endif


extern ScheduleType g_schedule_type;
extern pthread_mutex_t *node_mutexes;
extern pthread_cond_t  *node_conds;
extern NodeQueue        *node_queues;
void init_node_locks(int node_count);
void destroy_node_locks(int node_count);
void lock_node(int node, int traveler_id, int next_edge_weight);
void unlock_node(int node, int traveler_id);

#endif /* SYNC_H */