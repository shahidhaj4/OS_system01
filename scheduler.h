#ifndef SCHEDULER_H
#define SCHEDULER_H

/**
 * @brief Structure representing a traveler waiting in a node's queue.
 * Direct implementation to match Ghada's queue properties.
 */
typedef struct {
    int traveler_id;
    int order;
    int next_edge_weight;
    int waiting_state;
} WaitingTraveler;

/**
 * @brief Queue structure managed by the parent process for each node.
 */
typedef struct {
    WaitingTraveler travelers[50]; /* MAX_WAITING_TRAVELERS */
    int count;
} NodeQueue;

/**
 * @brief Enum defining the supported scheduling algorithms.
 */
typedef enum {
    SCHED_FCFS, /* First Come First Served */
    SCHED_SJF   /* Shortest Job First */
} ScheduleType;

/**
 * @brief Selects the next traveler to proceed based on the active scheduling policy.
 * @param queue Pointer to the waiting queue of the specific intersection/node.
 * @param sched_type The scheduling algorithm to apply (SCHED_FCFS or SCHED_SJF).
 * @return int The traveler_id of the selected traveler, or -1 if the queue is empty.
 */
int get_next_traveler(NodeQueue* queue, ScheduleType sched_type);

#endif /* SCHEDULER_H */