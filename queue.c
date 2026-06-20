#include "queue.h"

void init_node_queue(NodeQueue* q) {
    if (q == NULL) return;
    q->count = 0;
}

int enqueue_traveler(NodeQueue* q, int traveler_id, int order, int next_edge_weight, int waiting_state) {
    if (q == NULL) return 0;

    if (q->count >= MAX_WAITING_TRAVELERS) {
        return 0;
    }

    q->travelers[q->count].traveler_id = traveler_id;
    q->travelers[q->count].order = order;
    q->travelers[q->count].next_edge_weight = next_edge_weight;
    q->travelers[q->count].waiting_state = waiting_state;

    q->count++;
    return 1;
}

int remove_traveler(NodeQueue* q, int traveler_id) {
    if (q == NULL || q->count == 0) return 0;

    for (int i = 0; i < q->count; i++) {
        if (q->travelers[i].traveler_id == traveler_id) {
            for (int j = i; j < q->count - 1; j++) {
                q->travelers[j] = q->travelers[j + 1];
            }

            q->count--;
            return 1;
        }
    }

    return 0;
}