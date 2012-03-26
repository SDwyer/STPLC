
#include "thread.h"

//threads
int pthread_create(pthread_t *thread, const pthread_attr_t *attr, void* start_routine, void *arg)
{
	*thread=CreateThread(0,0,(LPTHREAD_START_ROUTINE)start_routine,arg,0,0);
	return 0;
}

int   pthread_join(pthread_t thread, void **data)
{
	WaitForSingleObject(thread,INFINITE);
	CloseHandle(thread);
	return 0;
}

int   pthread_attr_init(pthread_attr_t *)
{
	return 0;
}

//mutex variables
int   pthread_mutex_destroy(pthread_mutex_t *mutex)
{
	DeleteCriticalSection(mutex);
	return 0;
}

int   pthread_mutex_init(pthread_mutex_t * mutex, pthread_mutexattr_t *attr)
{
	(void) attr;
	InitializeCriticalSection(mutex);
	return 0;
}

int   pthread_mutex_lock(pthread_mutex_t *mutex)
{
	EnterCriticalSection(mutex);
	return 0;
}
int   pthread_mutex_trylock(pthread_mutex_t *mutex)
{
	return TryEnterCriticalSection(mutex);
}

int   pthread_mutex_unlock(pthread_mutex_t * mutex)
{
    LeaveCriticalSection(mutex);
	return 0;
}

//counting semaphore
int sem_init (sema_t *s,int shared,  u_int initial_count)
{
	pthread_mutex_init (&s->lock_, NULL);
	pthread_cond_init (&s->count_nonzero_, NULL);
	s->count_ = initial_count;
	s->waiters_count_ = 0;
	 
	return 0;
}

int sem_destroy (sema_t *s)
{
	pthread_mutex_destroy (&s->lock_);
	pthread_cond_destroy (&s->count_nonzero_);
	free(s); 
	return 0;
}


int sem_wait (sema_t *s)
{
	  // Acquire mutex to enter critical section.
	  pthread_mutex_lock (&s->lock_);

	  // Keep track of the number of waiters so that <sema_post> works correctly.
	  s->waiters_count_++;

	  // Wait until the semaphore count is > 0, then atomically release
	  // <lock_> and wait for <count_nonzero_> to be signaled. 
	  while (s->count_ == 0)
		pthread_cond_wait (&s->count_nonzero_, &s->lock_);

	  // <s->lock_> is now held.

	  // Decrement the waiters count.
	  s->waiters_count_--;

	  // Decrement the semaphore's count.
	  s->count_--;

	  // Release mutex to leave critical section.
	  pthread_mutex_unlock (&s->lock_);

	  return 0;
}


int sem_post (sema_t *s)
{
	  pthread_mutex_lock (&s->lock_);

	  // Always allow one thread to continue if it is waiting.
	  if (s->waiters_count_ > 0)
		pthread_cond_signal (&s->count_nonzero_);

	  // Increment the semaphore's count.
	  s->count_++;

	  pthread_mutex_unlock (&s->lock_);

	   return 0;
}




//conditional variables
int   pthread_cond_broadcast(pthread_cond_t *cond)
{
	WakeAllConditionVariable(cond);
	return 0;
}
int  pthread_cond_destroy(pthread_cond_t *cond)
{
	return 0;
}

int   pthread_cond_init(pthread_cond_t *cond, const pthread_condattr_t *)
{
	InitializeConditionVariable(cond);
	return 0;
}

int   pthread_cond_signal(pthread_cond_t *cond)
{
	WakeConditionVariable(cond);
	return 0;
}

int   pthread_cond_timedwait(pthread_cond_t *cond, pthread_mutex_t *mutex, const struct timespec * time)
{
	DWORD wait = (DWORD)(time->tv_sec*1000000 + (time->tv_nsec>>10));
	SleepConditionVariableCS (cond, mutex, wait);
	return 0;
}

int   pthread_cond_wait(pthread_cond_t *cond, pthread_mutex_t *mutex)
{
    SleepConditionVariableCS (cond, mutex, INFINITE);
    return 0;
}


//barrier
int pthread_barrier_destroy(pthread_barrier_t *b)
{
	EnterCriticalSection(&b->m);

	while (b->total > b->count)
	{
		/* Wait until everyone exits the barrier */
		SleepConditionVariableCS(&b->cv, &b->m, INFINITE);
	}

	LeaveCriticalSection(&b->m);

	DeleteCriticalSection(&b->m);

	return 0;
}

int pthread_barrier_init(pthread_barrier_t *b, int count)
{
	b->count = count;
	b->total = 0;

	InitializeCriticalSection(&b->m);
	InitializeConditionVariable(&b->cv);

	return 0;
}

int pthread_barrier_wait(pthread_barrier_t *b)
{
	EnterCriticalSection(&b->m);

	while (b->total >b->count)
	{
		/* Wait until everyone exits the barrier */
		SleepConditionVariableCS(&b->cv, &b->m, INFINITE);
	}

	/* Are we the first to enter? */
	if (b->total == b->count) b->total = 0;

	b->total++;

	if (b->total == b->count)
	{
		b->total += b->count - 1;
		WakeAllConditionVariable(&b->cv);

		LeaveCriticalSection(&b->m);

		return 1;
	}
	else
	{
		while (b->total < b->count)
		{
			/* Wait until enough threads enter the barrier */
			SleepConditionVariableCS(&b->cv, &b->m, INFINITE);
		}

		b->total--;

		/* Get entering threads to wake up */
		if (b->total == b->count) WakeAllConditionVariable(&b->cv);

		LeaveCriticalSection(&b->m);

		return 0;
	}
}

