
#ifndef _THREAD_

#define _THREAD_


#include <windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>


typedef HANDLE pthread_t;
typedef int pthread_attr_t;
typedef int pthread_mutexattr_t;
typedef int pthread_condattr_t;
typedef void *pthread_barrierattr_t;


typedef CRITICAL_SECTION pthread_mutex_t;
typedef CONDITION_VARIABLE pthread_cond_t;


typedef void(*thread_routine_t)(void*);

#define PTHREAD_MUTEX_INITIALIZER {(PRTL_CRITICAL_SECTION_DEBUG)(void*)-1,-1,0,0,0,0}
#define PTHREAD_COND_INITIALIZER {0}
#define PTHREAD_BARRIER_INITIALIZER {0,0,PTHREAD_MUTEX_INITIALIZER,PTHREAD_COND_INITIALIZER} 


int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void * start_routine, void *arg);
int pthread_join(pthread_t, void **);
int pthread_init(pthread_t *thread);
int pthread_attr_init(pthread_attr_t *);


//mutex
int pthread_mutex_init(pthread_mutex_t *m, pthread_mutexattr_t *a);
int pthread_mutex_lock(pthread_mutex_t *m);
int pthread_mutex_unlock(pthread_mutex_t *m);
int pthread_mutex_trylock(pthread_mutex_t *m);
int pthread_mutex_destroy(pthread_mutex_t *m);

//counting semaphore
struct sema_t
{
	int count_; // Current count of the semaphore.
	long waiters_count_; // Number of threads that have called <sema_wait>.  
	pthread_mutex_t lock_; // Serialize access to <count_> and <waiters_count_>.
	pthread_cond_t count_nonzero_;	// Condition variable that blocks the <count_> 0.
};
typedef struct sema_t sem_t;
int sem_init (sema_t *s,int shared, unsigned int initial_count);
int sem_wait (sema_t *s);
int sem_post (sema_t *s);
int sem_destroy(sema_t *s);


//condition variable

typedef struct timespec
{
	LONGLONG tv_sec;
	LONGLONG tv_nsec;
};

int   pthread_cond_init(pthread_cond_t *, const pthread_condattr_t *);
int   pthread_cond_broadcast(pthread_cond_t *);
int   pthread_cond_destroy(pthread_cond_t *);
int   pthread_cond_signal(pthread_cond_t *);
int   pthread_cond_timedwait(pthread_cond_t *, pthread_mutex_t *, const struct timespec *);
int   pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);


//barrier

typedef struct barrier_t pthread_barrier_t;
struct barrier_t
{
	int count;
	int total;
	CRITICAL_SECTION m;
	CONDITION_VARIABLE cv;
};

int pthread_barrier_init(pthread_barrier_t *b, int count);
int pthread_barrier_wait(pthread_barrier_t *b);
int pthread_barrier_destroy(pthread_barrier_t *b);


#endif
