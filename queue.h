#ifndef QUEUE_H
#define QUEUE_H

#define MAX_WAITING_TRAVELERS 50

typedef struct {
    int traveler_id;
    int order;
    int next_edge_weight;
    int waiting_state;
} WaitingTraveler;

typedef struct {
    WaitingTraveler travelers[MAX_WAITING_TRAVELERS];
    int count;
} NodeQueue;

void init_node_queue(NodeQueue* q);
int enqueue_traveler(NodeQueue* q, int traveler_id, int order, int next_edge_weight, int waiting_state);
int remove_traveler(NodeQueue* q, int traveler_id);

#endif