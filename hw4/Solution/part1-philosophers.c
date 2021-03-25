#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 6

char *thread_name[6] = {
    "Thread 1", "Thread 2", "Thread 3", "Thread 4",
    "Thread 5", "Thread 6"
};
char *resource_name[6] = {
    "Resource A", "Resource B", "Resource C",
    "Resource D", "Resource E", "Resource F"
};
pthread_mutex_t resource[6] = {
    PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER
};
pthread_mutex_t resource_try[6] = {
    PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER,PTHREAD_MUTEX_INITIALIZER,
    PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER
};

int have_resource[6] = {0};
int eat_retry_time[] = {2, 3, 5, 7, 11, 13};


struct philosopher {
  int id;
  int left_fork_id;
  int right_fork_id;
} philosophers[NUM_THREADS];


void think()
{
    sleep(2);
}

void prepare_to_eat(int philosopher_id, int resource_id1, int resource_id2)
{
    int i_own_resource_A = 0, i_own_resource_B = 0;
    pthread_mutex_lock(&resource_try[resource_id1]);
    if (have_resource[resource_id1] == 0) {
        have_resource[resource_id1] = 1;
        i_own_resource_A = 1; // acquired
    }
    pthread_mutex_unlock(&resource_try[resource_id1]);

    pthread_mutex_lock(&resource_try[resource_id2]);
    if (have_resource[resource_id2] == 0) {
        have_resource[resource_id2] = 1;
        i_own_resource_B = 1; // acquired
    }
    pthread_mutex_unlock(&resource_try[resource_id2]);

    if (i_own_resource_A && i_own_resource_B) {
        // Deadlock would be possible here, but no other thread
        // should believe they own both A and B.
        pthread_mutex_lock(&resource[resource_id1]);
        pthread_mutex_lock(&resource[resource_id2]);
        // Success: resources acquired
    } else {
        sleep(eat_retry_time[philosopher_id]);
    }
}

void finish_eating(int resource_id1, int resource_id2)
{
    pthread_mutex_unlock(&resource[resource_id1]);
    pthread_mutex_unlock(&resource[resource_id2]);
}

void * philosopher_doit(void *philosopher_args)
{
  struct philosopher *phil = philosopher_args;
  int i = 5;
  time_t time_start, time_end;
  time(&time_start);
  for (i = 0; i < 5; i++) {
    think();
    printf("%s thought!\n", thread_name[phil->id]);
    prepare_to_eat(phil->id, phil->left_fork_id, phil->right_fork_id);
    
    // This call to pthread_mutex_trylock is my code for testing that
    // your program is working correctly.  -- Prof. Cooperman
    if (pthread_mutex_trylock(&resource[0]) == 0) {
      printf("*** ERROR:  %s should be locked; but we locked it again.\n",
             resource_name[0]); // resource_name[0] is "Resource_A"
      exit(1);
    }
    if (pthread_mutex_trylock(&resource[1]) == 0) {
      printf("*** ERROR:  %s should be locked; but we locked it again.\n",
             resource_name[1]); // resource_name[0] is "Resource_B"
      exit(1);
    }

    sleep(2);
    printf("%s ate (using %s and %s)!\n",
      thread_name[0], resource_name[0], resource_name[1]);

    finish_eating(phil->left_fork_id, phil->right_fork_id); // release mutex locks
  }
  time(&time_end);
  printf("*** %s finished in %d seconds.\n",
    thread_name[0], time_end - time_start);
}


int main(int argc, char* argv[])
{
  pthread_t thread[NUM_THREADS];

  int i;
  for (i = 0; i < NUM_THREADS; i++) {
    philosophers[i] = (struct philosopher){i, i, (i+1) % NUM_THREADS};
  }

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_create(&thread[i], NULL, &philosopher_doit, &philosophers[i]);
  }

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(thread[i], NULL);
  }

  return 0;
}
