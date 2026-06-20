#include "scheduler.h"
#include <stdio.h>


int get_next_traveler(NodeQueue* queue, ScheduleType sched_type) {
    if (queue == NULL || queue->count == 0) {
        return -1;
    }

    int selected_index = 0;

    for (int i = 1; i < queue->count; i++) {
        if (sched_type == SCHED_FCFS) {
            if (queue->travelers[i].order < queue->travelers[selected_index].order) {
                selected_index = i;
            }
        }
        else if (sched_type == SCHED_SJF) {
            if (queue->travelers[i].next_edge_weight < queue->travelers[selected_index].next_edge_weight) {
                selected_index = i;
            }
        }
    }

    return queue->travelers[selected_index].traveler_id;
}