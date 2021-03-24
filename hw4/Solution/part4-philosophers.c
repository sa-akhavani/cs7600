// Naive dining philosophers solution, which leads to deadlock.

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 3

#define PICKED_UP 1
#define UNUSED 0

// state of each fork. if it is PICKED_UP it is equal to zero.
int state[NUM_THREADS];
// Condition variable
pthread_cond_t cond  = PTHREAD_COND_INITIALIZER;
// Guard Mutex for condition variable used for changing state[] array
pthread_mutex_t guard_mutex = PTHREAD_MUTEX_INITIALIZER;

struct forks {
  int philosopher;
  pthread_mutex_t *left_fork;
  pthread_mutex_t *right_fork;
} forks[NUM_THREADS];

void * philosopher_doit(void *forks_arg) {
  struct forks *forks = forks_arg;

  // if 0 or 1 fork is available for a philosopher, wait till 2 are available.
  while (state[(forks->philosopher) % NUM_THREADS] == PICKED_UP || state[(forks->philosopher + 1) % NUM_THREADS] == PICKED_UP)
  {
    pthread_cond_wait(&cond, &guard_mutex);
  }

  pthread_mutex_lock(forks->left_fork);
  //change state of the fork to PICKED_UP
  state[forks->philosopher] = PICKED_UP;

  pthread_mutex_lock(forks->right_fork);
  //change state of the fork to PICKED_UP
  state[forks->philosopher + 1] = PICKED_UP;

  sleep(1);
  printf("Philosopher %d just ate.\n", forks->philosopher);

  pthread_mutex_unlock(forks->left_fork);
  //change state of the fork to UNUSED after eating
  state[(forks->philosopher) % NUM_THREADS] = UNUSED;
  
  pthread_mutex_unlock(forks->right_fork);
  //change state of the fork to UNUSED after eating
  state[(forks->philosopher + 1) % NUM_THREADS] = UNUSED;

  // brodcast condition variable and unlock guard mutex
  pthread_cond_broadcast(&cond);
  pthread_mutex_unlock(&guard_mutex);
  return NULL;
}

int main(int argc, char* argv[])
{
  pthread_t thread[NUM_THREADS];
  pthread_mutex_t mutex_resource[NUM_THREADS];

  int i;
  for (i = 0; i < NUM_THREADS; i++) {
    mutex_resource[i] = (pthread_mutex_t)PTHREAD_MUTEX_INITIALIZER;
    forks[i] = (struct forks){i,
                     &mutex_resource[i], &mutex_resource[(i+1) % NUM_THREADS]};
  }

  for (i = 0; i < NUM_THREADS; i++) {
    pthread_create(&thread[i], NULL, &philosopher_doit, &forks[i]);
  }
  for (i = 0; i < NUM_THREADS; i++) {
    pthread_join(thread[i], NULL);
  }

  return 0;
}
