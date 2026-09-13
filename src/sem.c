#include "../include/sem.h"

void my_sem_init(my_sem_t *sem, int count){

  int mutex_init_err = pthread_mutex_init(&sem->mutex, NULL);

  int cond_init_err = pthread_cond_init(&sem->cond, NULL);

  if(mutex_init_err){
    fprintf(stderr, "mutex init error %d\n", mutex_init_err); 
    exit(1);
  }

  if(cond_init_err){
    fprintf(stderr, "cond init error %d\n", cond_init_err); 
    exit(1);
  }

  sem->count = count;
}

void my_sem_post(my_sem_t *sem){
  int lock_err = pthread_mutex_lock(&sem->mutex);
  if(lock_err){
    fprintf(stderr, "lock error %d\n", lock_err); 
    exit(1);
  }
  sem->count++;
  int signal_err = pthread_cond_signal(&sem->cond);
  if(signal_err){
    fprintf(stderr, "signal error %d\n", signal_err); 
    exit(1);
  }

  int unlock_err = pthread_mutex_unlock(&sem->mutex);
  if(unlock_err){
    fprintf(stderr, "unlock error %d\n", unlock_err); 
    exit(1);
  }
}

void my_sem_wait(my_sem_t *sem){
  int lock_err = pthread_mutex_lock(&sem->mutex);

  if(lock_err){
    fprintf(stderr, "lock error %d\n", lock_err); 
    exit(1);
  }

  while(sem->count == 0){
    int wait_err = pthread_cond_wait(&sem->cond, &sem->mutex);
    if(wait_err){
      fprintf(stderr, "wait error %d\n", wait_err); 
      exit(1);
    }
  }
  sem->count--;
  
  int unlock_err = pthread_mutex_unlock(&sem->mutex);
  if(unlock_err){
    fprintf(stderr, "unlock error %d\n", unlock_err); 
    exit(1);
  }
}
