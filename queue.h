#ifndef QUEUE_H
#define QUEUE_H

typedef struct QueueNode {
    int traveler_id;          
    int order;                
    int next_edge_weight;     
    int waiting_state;        
    struct QueueNode* next;   
} QueueNode;

typedef struct {
    QueueNode* front;         
    QueueNode* rear;          
    int size;                 
} NodeQueue;

/* Queue Functions */
void init_node_queue(NodeQueue* q);
void enqueue_traveler(NodeQueue* q, int traveler_id, int order, int next_edge_weight, int waiting_state);
int remove_traveler(NodeQueue* q, int traveler_id);
QueueNode* get_node_queue(NodeQueue* q);
void free_node_queue(NodeQueue* q);

#endif /* QUEUE_H */