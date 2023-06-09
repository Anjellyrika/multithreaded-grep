#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>

// Globals
char *global_search_string;

struct queue *taskqueue;
struct node {
    char path[250];
    struct node *next;
};

struct queue {
    struct node *front;
    struct node *rear;
};

// Task queue function signatures
struct queue* initqueue();
void enqueue(struct queue*, char*);
int dequeue(struct queue*);
void printqueue(struct queue*);

//

void *search(void *id) {
    uintptr_t worker_ID = (uintptr_t) id;
    char dir_path[250];

    while (1) {
        if(taskqueue->front != NULL){

            // Take a new job
            strcpy(dir_path, taskqueue->front->path);
            dequeue(taskqueue);

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
                    char new_path[250] = {0};
                    strcat(new_path, dir_path);
                    strcat(new_path, "/");
                    strcat(new_path, content->d_name);
                    enqueue(taskqueue, new_path);

                    realpath(new_path, abs_path);
                    printf("[%ld] ENQUEUE %s\n", worker_ID, abs_path);
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
        }
        else
            break;
    }
    return NULL;
}

int main(int argc, char *argv[]) {
    assert(argc == 4);

    // int n_workers = argv[1]; // single-threaded version only uses the main thread
    char *rootpath = argv[2];
    char *search_string = argv[3];
    global_search_string = search_string;
    
    // Main thread enqueues rootpath and executes search
    taskqueue = initqueue();
    enqueue(taskqueue, rootpath);
    search(0);

    free(taskqueue);

    return 0;
}

// TASK QUEUE METHODS

struct queue* initqueue() {
    struct queue *tasks = (struct queue*)malloc(sizeof(struct queue));
    tasks->front = NULL;
    tasks->rear = NULL;
    return tasks;
}

void enqueue(struct queue *tasks, char *path) {
    struct node *newnode = (struct node*)malloc(sizeof(struct node));
    assert(newnode != NULL);

    strcpy(newnode->path, path);
    newnode->next = NULL;

    if (tasks->front == NULL) {
        tasks->front = newnode;
        tasks->rear = newnode;
    }
    else {
        tasks->rear->next = newnode;
        tasks->rear = newnode;
    }
}

int dequeue(struct queue *tasks) {
    struct node *temp = tasks->front;
    if (temp == NULL) {
        return -1;
    }
    else {
        tasks->front = temp->next;
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