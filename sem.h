#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>

typedef struct my_sem {
  pthread_mutex_t mutex;
  pthread_cond_t cond;
  pthread_cond_t is_full;
  int count;
} my_sem_t;

void my_sem_init(my_sem_t *sem, int count);

void my_sem_post(my_sem_t *sem);

void my_sem_wait(my_sem_t *sem);
