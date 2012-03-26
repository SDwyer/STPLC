#ifdef WIN32
#include "thread.h"
#else
#include <pthread.h>
#include <semaphore.h>
#endif

#include "pool.h"

int valid = 1;

/*  */
int running = 1;

/*
   Function: pool_init
*/
pool_t* pool_init(int thread_count)
{
	int i;
	pool_t* thread_pool = NULL;

	/* ALLOCATE MEMORY FOR THREAD POOL */
	if((thread_pool = (pool_t*)malloc(sizeof(pool_t)))==NULL)
		return NULL;

	/* INITIALISE THE THREAD COUNT */
	(thread_pool->thread_count) = thread_count;

	/* INITIALIZE QUEUE MUTEX */
	if(pthread_mutex_init(&(thread_pool->mutex),NULL) != 0)
	{
		free(thread_pool);
		return NULL;
	}

	/* INITIALIZE QUEUE SEMAPHORE */
	if(sem_init(&(thread_pool->sem),0,0) != 0)
	{
		pthread_mutex_destroy(&thread_pool->mutex);
		free(thread_pool);
		return NULL;
	}

	/* ALLOCATE MEMORY FOR THREADS */
	if((thread_pool->threads =(pthread_t*)calloc(thread_count ,sizeof(pthread_t))) == NULL)
	{
		pthread_mutex_destroy(&thread_pool->mutex);
		sem_destroy (&thread_pool->sem);
		free(thread_pool->threads);
		free(thread_pool);
		return NULL;
	}
	
	/* CREATE THE THREADS */
	for(i = 0; i < thread_count; i++)
	{
		pthread_t* current_thread;
		current_thread = (&thread_pool->threads[i]);
		if(pthread_create(current_thread, NULL, &thread_routine, thread_pool) != 0)
		{
			pthread_mutex_destroy(&thread_pool->mutex);
			sem_destroy (&thread_pool->sem);
			free(thread_pool->threads);
			free(thread_pool);
			return NULL;
		}
	}
	
	/* CREATE TASK QUEUE */
	if((thread_pool->task_queue = get_task_queue())==NULL)
	{
		pthread_mutex_destroy(&thread_pool->mutex);
		sem_destroy (&thread_pool->sem);
		free(thread_pool->threads);
		free(thread_pool);
		return NULL;
	}

	return thread_pool;
}


/*
   Function: task_queue_init
*/
int	task_queue_init(pool_t * pool)
{
	task_queue_t* queue;

	if(!(queue = get_task_queue()))
		return 1;

	return 0;
}


/*
   Function: enqueue_task
*/
int  pool_enqueue_task(pool_t* pool, task_t* task)
{
	/* CREATE A NEW TASK */
	task_t* temp = get_task();
	task_queue_t* queue = (pool->task_queue);
	temp->func = task->func;
	temp->args = task->args;
	temp->next = NULL;

	/* ACQUIRE QUEUE LOCK */
	if(pthread_mutex_lock(&(pool->mutex)) != 0)
		return 1;

	/* IF QUEUE IS EMPTY */
	if((queue->tail) == NULL)
	{
		queue->head = temp;
		queue->tail = temp;
	}
	else
	{
		queue->tail->next = temp;
		queue->tail = temp;
	}

	/* UNLOCK SEMAPHORE */
	if(sem_post(&(pool->sem)) != 0)
		return 1;

	/* RELEASE QUEUE LOCK */
	if(pthread_mutex_unlock(&(pool->mutex)) != 0)
		return 1;

	return 0;
}

/*
   Function: dequeue_task
*/
task_t*  pool_dequeue_task(pool_t* pool)
{
	task_queue_t* queue = (pool->task_queue);
	task_t* task = get_task();
	task_t* temp;
	temp = queue->head;

	/* IF QUEUE CONTAINS TASKS */
	if(queue->head != NULL)
	{
		task->func = queue->tail->func;
		task->args = queue->tail->args;
		/* IF MORE THAN ONE TASK IN THE QUEUE */
		if(queue->head != queue->tail)
		{
			queue->head = temp->next;
		}
		else/* ONE TASK IN THE QUEUE */
		{
			queue->head = NULL;
			queue->tail = NULL;
		}
		free(temp);
		queue->task_count--;	
	}
	else/* EMPTY QUEUE */
	{
		free(task);
		task = NULL;
	}

	return task;
}

/*
   Function: pool_destroy
*/
int pool_destroy(pool_t* pool)
{
	int i;
	running = 0;

	/* UNLOCK THE SEMAPHORE FOR EACH THREAD IN THE POOL */
	//for(i=0;i<pool->thread_count;i++)
	///{
	//	sem_post(&pool->sem);
	//}

	/* FREE THE POOL */
	free(pool->threads);
	//sem_destroy(&pool->sem);
	pthread_mutex_destroy(&pool->mutex);
	free(pool->task_queue);
	free(pool);
	return 0;
}


/*
   Function: create_task
*/
int create_task(pool_t* pool, void *function_ptr, void* args)
{
	task_t* task = get_task();
	task->args = args;
	task->func = (void*(*)(void*))function_ptr;
	pool_enqueue_task(pool, task);
	return 0;
}

/*
   Function: thread_routine
*/
int thread_routine(pool_t* pool)
{
	void * (*function)(void*arg);
	void * args;
	task_t * task;

	/* WHILE POOL IS RUNNING */
	while(running)
	{
		/* LOCK QUEUE SEMAPHORE */
		if(sem_wait(&(pool->sem)))
			return 1;

		/* ACQUIRE QUEUE LOCK  */
		pthread_mutex_lock(&(pool->mutex));
		
		/* DEQUEUE TASK FROM TASK QUEUE IF AVAILABLE*/
		if((task = pool_dequeue_task(pool))!=NULL)
		{	
			function = task->func;
			args = task->args;

		}

		/* RELEASE QUEUE LOCK */
		pthread_mutex_unlock(&(pool->mutex));
	
		function(args);
		free(task);

	}
	return 0;
}

/*
	Function: get_task
*/
task_t* get_task()
{
	if(task_t* task = (task_t*)malloc(sizeof(task_t)))
	{
		task->args = NULL;
		task->func = NULL;
		task->next = NULL;
		return task;
	}
	return NULL;
}

/*
	Function: get_task_queue
*/
task_queue_t* get_task_queue()
{
	if(task_queue_t* queue = (task_queue_t*)malloc(sizeof(task_queue_t)))
	{
		queue->task_count = 0;
		queue->head = NULL;
		queue->tail = NULL;
		return queue;
	}
	return NULL;
}

/*
	Function: set_running
*/
void set_running(int r)
{
	running = r;
}