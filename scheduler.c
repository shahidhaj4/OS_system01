#include "scheduler.h"

int get_next_traveler(NodeQueue* queue, ScheduleType sched_type) {
    /* If the queue is invalid or empty, return -1 indicating no traveler is waiting */
    if (!queue || queue->count == 0) {
        return -1;
    }

    /* Start by assuming the first traveler in the queue is the best choice */
    int selected_index = 0;

    /* Iterate through the queue to find the optimal traveler based on the scheduler */
    for (int i = 1; i < queue->count; i++) {
        if (sched_type == SCHED_FCFS) {
            /* FCFS Logic: Select the traveler with the lowest arrival order number */
            if (queue->travelers[i].order < queue->travelers[selected_index].order) {
                selected_index = i;
            }
        } 
        else if (sched_type == SCHED_SJF) {
            /* SJF Logic: Select the traveler with the shortest next edge weight */
            if (queue->travelers[i].next_edge_weight < queue->travelers[selected_index].next_edge_weight) {
                selected_index = i;
            }
        }
    }

    /* Store the traveler ID of the selected winner */
    int next_traveler_id = queue->travelers[selected_index].traveler_id;

    return next_traveler_id;
}