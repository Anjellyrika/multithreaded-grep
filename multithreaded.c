#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <assert.h>
#include <stdint.h>
#include <string.h>
#include "task_queue.h"

// Initialize condition variable
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t waiting = PTHREAD_COND_INITIALIZER;

char *global_search_string;

void *search(void *id) {
    uintptr_t worker_ID = (uintptr_t) id;
    char *search_string = global_search_string;

    /// REMOVE THIS SECTION LATER
    printf("Thread %ld searching for %s\n", worker_ID, search_string);
    ///

    // what the threads will do
    /*
    if taskqueue is empty:
        thread goes to sleep.
    else:
        dequeue the task
        printf("[%d] DIR %s\n", worker_ID, dir_path);
        opendir
        for (each child object accessed by readdir): // NOTE: ignore . and ..
            if child is a directory:
                enqueue new search to task queue containing the path to the said directory
                printf("[%d] ENQUEUE %s\n", worker_ID, dir_path);
            else if child is a file:
                grep with arguments search_string and path via the system function where search_string is the search_string supplied as a program argument and path is the path (relative or absolute) of the said file;
                if match found:
                    printf("[%d] PRESENT %d\n", worker_ID, file_path)
                else:
                    printf("[%d] ABSENT %d\n", worker_ID, file_path)
    */
    return NULL;
}

int main(int argc, char *argv[]) {
    assert(argc == 4);

    int n_workers = atoi(argv[1]);
    char *rootpath = argv[2];
    char *search_string = argv[3];
    global_search_string = search_string;

    /// REMOVE THIS SECTION LATER
    // printf("you entered: %s, %s, %s\n", argv[1], argv[2], argv[3]);
    // printf("%d threads will be searching %s for substring \"%s\".\n", n_workers, rootpath, search_string);
    ///

    /*
    Your main process/thread is expected to terminate only after all workers have
    terminated—it must block via pthread join or wait for the majority of the execution duration.
    It must also enqueue `rootpath` before launching any of the workers.
    */
    
    // Initialize task queue
    struct queue *taskqueue = initqueue();
    printqueue(taskqueue);
    enqueue(taskqueue, "PLEASE!!!!!WORK!!!");
    printqueue(taskqueue);
    
    // Thread creation
    pthread_t workers[n_workers]; // maybe use malloc instead?
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

