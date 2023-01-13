// Linked list implementation of the task queue

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <string.h>

//

struct node {
    char path[250];
    struct node *next;
};

struct queue {
    struct node *front;
    struct node *rear;
    pthread_mutex_t queue_lock;
};

// Create an empty queue
struct queue* initqueue() {
    struct queue *tasks = (struct queue*)malloc(sizeof(struct queue));
    tasks->front = NULL;
    tasks->rear = NULL;
    pthread_mutex_t queue_lock = PTHREAD_MUTEX_INITIALIZER;
    return tasks;
}

void enqueue(struct queue *tasks, char *path) {
    struct node *newnode = (struct node*)malloc(sizeof(struct node));
    assert(newnode != NULL);

    strcpy(newnode->path, path);
    newnode->next = NULL;

    //lock this section//
    if (tasks->front == NULL) {
        tasks->front = newnode;
        tasks->rear = newnode;
    }
    else {
        tasks->rear->next = newnode;
        tasks->rear = newnode;
    }
    //unlock//
}

int dequeue(struct queue *tasks) {
    //lock this section//
    struct node *temp = tasks->front;
    if (temp == NULL) {
        //unlock here//
        return -1;
    }
    else {
        tasks->front = temp->next;
        //unlock here//
        free(temp);
        return 0;
    }
}

void printqueue(struct queue *tasks) {
    struct node *current = tasks->front;
    if (current == NULL)
        printf("No tasks queued.\n");
    printf("QUEUE: ");
    while(current != NULL) {
        printf("%s--", current->path);
        current = current->next;
    }
    printf("\n");
    free(current);
}