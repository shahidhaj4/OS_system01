#include "queue.h"
#include <stdio.h>
#include <stdlib.h>

void init_node_queue(NodeQueue* q) {
    if (q == NULL) return;
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}

void enqueue_traveler(NodeQueue* q, int traveler_id, int order, int next_edge_weight, int waiting_state) {
    if (q == NULL) return;

    QueueNode* newNode = (QueueNode*)malloc(sizeof(QueueNode));
    if (newNode == NULL) {
        perror("Memory allocation failed");
        return;
    }

    newNode->traveler_id = traveler_id;
    newNode->order = order;
    newNode->next_edge_weight = next_edge_weight;
    newNode->waiting_state = waiting_state;
    newNode->next = NULL;

    if (q->rear == NULL) {
        q->front = newNode;
        q->rear = newNode;
    } else {
        q->rear->next = newNode;
        q->rear = newNode;
    }
    q->size++;
}

int remove_traveler(NodeQueue* q, int traveler_id) {
    if (q == NULL || q->front == NULL) return 0;

    QueueNode* temp = q->front;
    QueueNode* prev = NULL;

    while (temp != NULL && temp->traveler_id != traveler_id) {
        prev = temp;
        temp = temp->next;
    }

    if (temp == NULL) return 0;

    if (prev == NULL) {
        q->front = temp->next;
    } else {
        prev->next = temp->next;
    }

    if (temp == q->rear) {
        q->rear = prev;
    }

    free(temp);
    q->size--;
    return 1;
}

QueueNode* get_node_queue(NodeQueue* q) {
    if (q == NULL) return NULL;
    return q->front;
}

void free_node_queue(NodeQueue* q) {
    if (q == NULL) return;
    QueueNode* current = q->front;
    QueueNode* nextNode;

    while (current != NULL) {
        nextNode = current->next;
        free(current);
        current = nextNode;
    }
    q->front = NULL;
    q->rear = NULL;
    q->size = 0;
}