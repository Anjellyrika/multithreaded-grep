#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

// Initialize locks
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t increment_done_lock = PTHREAD_MUTEX_INITIALIZER;

// Globals
char *global_search_string;
int open_dirs = 1;

struct queue *taskqueue;
struct node {
    char path[250];
    struct node *next;
};

struct queue {
    struct node *front;
    struct node *rear;
    pthread_mutex_t queue_lock;
};

// Task queue function signatures
struct queue* initqueue();
void enqueue(struct queue*, char*);
void dequeue(struct queue*);
int isEmpty(struct queue*);
void printqueue(struct queue*);

//

void *search(void *id) {
    uintptr_t worker_ID = (uintptr_t) id;
    char dir_path[250];

    while (open_dirs > 0) {
        int empty = 0;
        pthread_mutex_lock(&lock);
            empty = isEmpty(taskqueue);
        pthread_mutex_unlock(&lock);
        
        if(!empty){
            
            // Take a new job
            pthread_mutex_lock(&lock);  // Only one thread can take a single job at a time
            // printf("%ld acquired lock.\n", worker_ID); ///
            // printqueue(taskqueue); ///
            if(isEmpty(taskqueue)) {
                pthread_mutex_unlock(&lock);
                continue;    // Continue loop if task queue has become empty.
            }
            strcpy(dir_path, taskqueue->front->path);
            dequeue(taskqueue);
            pthread_mutex_unlock(&lock);
            // printf("%ld now searching. Took job %s\n", worker_ID, dir_path); ///

            char abs_path[250] = {0};
            realpath(dir_path, abs_path);
            printf("[%ld] DIR %s\n", worker_ID, abs_path);

            DIR *dir = opendir(dir_path);
            if (dir == NULL) continue;
            struct dirent *content;

            while ((content = readdir(dir)) != NULL) {
                if (!strcmp(content->d_name, ".") || !strcmp(content->d_name, "..")) continue;

                int type = content->d_type;
                if (type == DT_DIR) {            // Child is a directory
                    pthread_mutex_lock(&lock);  // Only one thread can modify task queue at a time
                    char new_path[250] = {0};
                    strcat(new_path, dir_path);
                    strcat(new_path, "/");
                    strcat(new_path, content->d_name);
                    enqueue(taskqueue, new_path);

                    realpath(new_path, abs_path);
                    printf("[%ld] ENQUEUE %s\n", worker_ID, abs_path);
                    pthread_mutex_lock(&increment_done_lock);
                    open_dirs++;
                    pthread_mutex_unlock(&increment_done_lock);
                    pthread_mutex_unlock(&lock);
                }

                else if (type == DT_REG) {       // Child is a file
                    char grep_command[600];
                    char file_path[250] = {0};
                    strcat(file_path, dir_path);
                    strcat(file_path, "/");
                    strcat(file_path, content->d_name);
                    sprintf(grep_command, "grep \"%s\" \"%s\" 1> /dev/null", global_search_string, file_path);

                    int grep_retval = system(grep_command);
                        realpath(file_path, abs_path);
                        if (grep_retval == 0)
                            printf("[%ld] PRESENT %s\n", worker_ID, abs_path);
                        else
                            printf("[%ld] ABSENT %s\n", worker_ID, abs_path);
                }
            }
            closedir(dir);
            pthread_mutex_lock(&increment_done_lock);
            open_dirs--;
            pthread_mutex_unlock(&increment_done_lock);
        }
        else {
            continue;
        }
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    assert(argc == 4);

    int n_workers = atoi(argv[1]);
    char *rootpath = argv[2];
    char *search_string = argv[3];
    global_search_string = search_string;
    
    // Main thread enqueues rootpath
    taskqueue = initqueue();
    enqueue(taskqueue, rootpath);

    // Thread creation
    pthread_t workers[n_workers];
    for (uintptr_t i = 0; i < n_workers; i++) {
        pthread_create(&workers[i], NULL, search, (void *)i);
    }

    for (uintptr_t i = 0; i < n_workers; i++) {
        pthread_join(workers[i], NULL);
    }
    // closedir

    return 0;
}

// TASK QUEUE METHODS

struct queue* initqueue() {
    struct queue *tasks = (struct queue*)malloc(sizeof(struct queue));
    tasks->front = NULL;
    tasks->rear = NULL;
    pthread_mutex_init(&tasks->queue_lock, NULL);
    return tasks;
}

void enqueue(struct queue *tasks, char *path) {
    struct node *newnode = (struct node*)malloc(sizeof(struct node));
    assert(newnode != NULL);

    strcpy(newnode->path, path);
    newnode->next = NULL;

    pthread_mutex_lock(&tasks->queue_lock);
    if (tasks->front == NULL) {
        tasks->front = newnode;
        tasks->rear = newnode;
    }
    else {
        tasks->rear->next = newnode;
        tasks->rear = newnode;
    }
    pthread_mutex_unlock(&tasks->queue_lock);
}

void dequeue(struct queue *tasks) {
    pthread_mutex_lock(&tasks->queue_lock);
    struct node *temp = tasks->front;
    if (temp == NULL) {
        pthread_mutex_unlock(&tasks->queue_lock);
        return;
    }
    else {
        tasks->front = temp->next;
        pthread_mutex_unlock(&tasks->queue_lock);
        free(temp);
        return;
    }
}

int isEmpty(struct queue *tasks) {
    pthread_mutex_lock(&tasks->queue_lock);
    if (tasks->front == NULL) {
        pthread_mutex_unlock(&tasks->queue_lock);
        return 1;
    }
    else {
        pthread_mutex_unlock(&tasks->queue_lock);
        return 0;
    }
}

void printqueue(struct queue *tasks) {
    struct node *current = tasks->front;
    if (current == NULL) {
        printf("No tasks queued.\n");
        return;    
    }
    printf("\nQUEUE: ");
    while(current != NULL) {
        printf("%s--", current->path);
        current = current->next;
    }
    printf("\n");
    free(current);
}