#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include <sys/types.h>
#include <dirent.h>
#include "task_queue.h"

// Initialize condition variable
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t waiting = PTHREAD_COND_INITIALIZER;

// Globals
char *global_search_string;
int done = 0;

struct queue *taskqueue;

void *search(void *id) {
    uintptr_t worker_ID = (uintptr_t) id;
    char dir_path[250];

    /// maybe have a different while condition, while all threads havent exited?? (use global done variable)
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

    int n_workers = 1; // single-threaded version
    char *rootpath = argv[2];
    char *search_string = argv[3];
    global_search_string = search_string;

    /*
    Your main process/thread is expected to terminate only after all workers have
    terminated—it must block via pthread join or wait for the majority of the execution duration.
    */
    
    // Main thread enqueues rootpath
    taskqueue = initqueue();
    enqueue(taskqueue, rootpath);
    // printqueue(taskqueue); ///

    // Thread creation
    pthread_t workers[n_workers]; /// maybe use malloc instead?
    for (uintptr_t i = 0; i < n_workers; i++) {
        pthread_create(&workers[i], NULL, search, (void *)i);
    }

    for (uintptr_t i = 0; i < n_workers; i++) {
        pthread_join(workers[i], NULL);
    }

    // closedir

    return 0;
}

/*NOTES
All executed instances of `grep` must not print anything on screen. You may redirect
its standard output stream to /dev/null. Note that `system` executes its given as a shell
command, so I/O redirection via > and < is supported.

Note as well that the return value of `system` corresponds to the exit code of the
command—you may use this to determine whether a grep execution was able to find
the search string in the file assigned to it.
*/

