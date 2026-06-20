#include "scheduler.h"
#include <stdio.h>
int get_next_traveler(NodeQueue* queue, ScheduleType sched_type) {
    if (queue == NULL || queue->front == NULL || queue->size == 0) {
        return -1;
    }

    QueueNode* current = queue->front;
    QueueNode* selected = queue->front;

    while (current != NULL) {
        if (sched_type == SCHED_FCFS) {
            if (current->order < selected->order) {
                selected = current;
            }
        }
        else if (sched_type == SCHED_SJF) {
            if (current->next_edge_weight < selected->next_edge_weight) {
                selected = current;
            }
        }

        current = current->next;
    }

    return selected->traveler_id;
}