// Linked list implementation of the task queue

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

pthread_mutex_t front_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t rear_lock = PTHREAD_MUTEX_INITIALIZER;

typedef struct queue_node {
    char path[250];
    struct queue_node *next;
} queue_node;

typedef struct queue {
    struct queue_node *front;
    struct queue_node *rear;
    pthread_mutex_t front_lock;
    pthread_mutex_t rear_lock;
} queue;

// Create an empty queue
struct queue* initqueue() {
    struct queue *tq = (struct queue*)malloc(sizeof(struct queue));
    tq->front = NULL;
    tq->rear = NULL;
    return tq;
}

void enqueue(queue *tq, char *path) {
    struct queue_node *temp = (struct queue_node *)malloc(sizeof(struct queue_node));
    assert (temp != NULL);

    strcpy(temp->path, path);
    temp->next = NULL;

    pthread_mutex_lock(&tq->rear_lock);
    tq->rear->next = temp;
    // tq->rear = temp;
    pthread_mutex_unlock(&tq->rear_lock);
}

void dequeue() {

}

void printqueue(queue *tq) {
    struct queue_node *current = tq->front;
    if (current == NULL) {
        printf("Task queue is empty.");
    }
    else {
        while(current != NULL) {
            printf("%s - ", current->path);
            current = current->next;
        }
    }
}