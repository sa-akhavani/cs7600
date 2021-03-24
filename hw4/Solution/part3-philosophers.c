// Naive dining philosophers solution, which leads to deadlock.

#include <stdio.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_THREADS 3

// define sem_dining semaphore. It is global to make things easier
sem_t sem_dining;

struct forks {
  int philosopher;
  pthread_mutex_t *left_fork;
  pthread_mutex_t *right_fork;
} forks[NUM_THREADS];

void * philosopher_doit(void *forks_arg) {
  //sem_wait before grabbing forks
  sem_wait(&sem_dining);
  struct forks *forks = forks_arg;
  pthread_mutex_lock(forks->left_fork);
  pthread_mutex_lock(forks->right_fork);
  sleep(1);
  printf("Philosopher %d just ate.\n", forks->philosopher);
  pthread_mutex_unlock(forks->left_fork);
  pthread_mutex_unlock(forks->right_fork);
  // sem_post after eating and releasing forks
  sem_post(&sem_dining);
  return NULL;
}

int main(int argc, char* argv[])
{
  pthread_t thread[NUM_THREADS];
  pthread_mutex_t mutex_resource[NUM_THREADS];

//   Initialize semaphore to value 2
  sem_init(&sem_dining, 0, 2);

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
