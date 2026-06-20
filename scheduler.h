#ifndef SCHEDULER_H
#define SCHEDULER_H

#include "queue.h"

typedef enum {
    SCHED_FCFS,
    SCHED_SJF
} ScheduleType;

int get_next_traveler(NodeQueue* queue, ScheduleType sched_type);

#endif