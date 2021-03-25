#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int waiting_readers = 0;
int waiting_writers = 0;
int active_readers = 0;
int active_writers = 0;

pthread_cond_t can_write = PTHREAD_COND_INITIALIZER;
pthread_cond_t can_read = PTHREAD_COND_INITIALIZER;
pthread_mutex_t guard_mutex = PTHREAD_MUTEX_INITIALIZER;

void do_write() {
    sleep(1);
}

void do_read() {
    sleep(1);
}

void *writer(void *arg) {
    int thread_id = *((int *) arg);
    pthread_mutex_lock(&guard_mutex);
    printf("Writer thread with id %d is acquiring the lock!\n", thread_id);
    while(active_writers == 1 || active_readers > 0)
    {
            waiting_writers += 1;
            printf("Writer thread with id %d is releasing the lock!\n", thread_id);        
            pthread_cond_wait(&can_write, &guard_mutex);
            printf("Writer thread with id %d is acquiring the lock!\n", thread_id);
            waiting_writers -= 1;
    }
    active_writers = 1;
    
    // printf("Writer thread with id %d is releasing the lock!\n", thread_id);
    pthread_mutex_unlock(&guard_mutex); // release the lock after writing
    
    // In here Writer Could Do anything it desires. We sleep for 1 seconds
    do_write();

    //Now signalling others and changing shared vars
    pthread_mutex_lock(&guard_mutex);
    // printf("Writer thread with id %d is acquiring the lock!\n", thread_id);
    active_writers = 0;
    //Writer Priority.
    if(waiting_writers > 0)
    {
        pthread_cond_signal(&can_write); // signal a writer to wake up
    } else {
        pthread_cond_broadcast(&can_read); //All readers could read at the same time so we use broadcast here
    }

    printf("Writer thread with id %d is releasing the lock!\n", thread_id);
    pthread_mutex_unlock(&guard_mutex);

    return NULL;
}
                                                                               
void *reader(void *arg) {
    int thread_id = *((int *) arg);

    pthread_mutex_lock(&guard_mutex);
    printf("Reader thread with id %d is acquiring the lock!\n", thread_id);
    while(active_writers == 1 || waiting_writers > 0)
    {
            waiting_readers += 1;
            printf("Reader thread with id %d is releasing the lock!\n", thread_id);
            pthread_cond_wait(&can_read, &guard_mutex);
            printf("Reader thread with id %d is acquiring the lock!\n", thread_id);
            waiting_readers -= 1;
    }
    active_readers += 1;

    // printf("Reader thread with id %d is releasing the lock!\n", thread_id);
    pthread_mutex_unlock(&guard_mutex); // release the lock after reading
    
    if(waiting_readers > 0 && waiting_writers > 0) //readers can read simultaniously. so we broadcast all of them here. (if no writer is waiting of course)
    {
        pthread_cond_broadcast(&can_read);
    }
    
    // In here Writer Could Do anything it desires. We sleep for 1 seconds
    do_read();


    //Now signalling others and changing shared vars
    pthread_mutex_lock(&guard_mutex);
    // printf("Reader thread with id %d is acquiring the lock!\n", thread_id);
    active_readers -= 1;
    //Writer Priority.
    if(active_readers == 0)
    {
        if(waiting_writers > 0)
        {
            pthread_cond_signal(&can_write); // signal a writer to wake up
        } else {
            pthread_cond_broadcast(&can_read); //All readers could read at the same time so we use broadcast here
        }
    }

    printf("Reader thread with id %d is releasing the lock!\n", thread_id);
    pthread_mutex_unlock(&guard_mutex);
}

int main(int argc, char *argv[])
{
    int readers_count = 3;
    int writers_count = 2;
    pthread_t readers[3];
    pthread_t writers[2];

    int thread_ids[5] = {1, 2, 3, 4, 5};
    int i = 0;
    int j = 0;

    for (i = 0; i < readers_count; i++) {
        pthread_create(&readers[i], NULL, (void *)reader, (int *) &thread_ids[i]); 
    }
    for (j = 0; j < writers_count; j++) {
	    pthread_create(&writers[j], NULL, (void *)writer, (int *) &thread_ids[i]); 
        i++;
    }
    for (i = 0; i < readers_count; i++) {
        pthread_join(readers[i], NULL); 
    }
    for (i = 0; i < writers_count; i++) {
        pthread_join(writers[i], NULL); 
    }
    return 0;
}
