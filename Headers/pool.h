
#ifndef _POOL_
#define _POOL_

/* TASK */
typedef struct task_t
{
	task_t* next;
	void*(*func)(void* arg);
	void* args;
	
}task_t;

/* TASK QUEUE */
typedef struct task_queue_t
{
	task_t* head;
	task_t* tail;
	int task_count;
	
}task_queue_t;

/* THREAD POOL */
typedef struct pool_t
{
	int thread_count;
	pthread_t* threads;
	task_queue_t* task_queue;
	pthread_mutex_t mutex;
	sem_t sem;	
}pool_t; 

typedef enum 
{
    threadpool_invalid        = -1,
    threadpool_lock_failure   = -2,
    threadpool_queue_full     = -3,
    threadpool_shutdown       = -4,
    threadpool_thread_failure = -5
} threadpool_error_t;

/*
   Function: pool_init

   Allocates the memory for and initializes a new threadpool.
   Calls the task_queue_init function to initilze the task_queue .
   Allocates the memory for a thread array and creates the threads. 
   Allocates memory for and initializes the task_queue semaphore.

   Parameters:

      n - The number of threads in the pool 
	  this number is derived form the cpu core check done earlier .

   Returns:

      A pointer to the initilized threadpool.
	  Or NULL if the initialization fails.

   See Also:

*/
pool_t* pool_init(int n);


/*
   Function: task_queue_init

   Allocates the memory for and initializes the task queue.

   Parameters:

      queue - The task_queue to be intialised

   Returns:

      0 if successfull

   See Also:

*/
int	task_queue_init(pool_t* pool);


/*
   Function: create_task

		Allocates the memory for a new task struct
		Initialises the task with the parameters passed
		Calls the enqueue_task function 

   Parameters:

		pool- the threadpool to add the task to
		function_ptr - a pointer to the function
		args - a pointer to the arguments for the supplied function

   Returns:

      0 - if successfull

   See Also:

*/

int create_task(pool_t* pool, void *function_ptr, void* args);


/*
   Function: enqueue_task

		Allocates the memory for a new task
		Initialises the task with the parameters passed
		Adds the task to the task_queue

   Parameters:

		pool- the threadpool to add the task to
		function_ptr - a pointer to the function
		args - a pointer to the arguments for the supplied function

   Returns:

      0 - if successfull

   See Also:

*/
int  pool_enqueue_task(pool_t* pool, task_t* task);

/*
   Function: dequeue_task

		Attempts to dequeue tasks from the task queue in FIFO order

   Parameters:

		pool- pointer to the threadpool from which to dequeue the task

   Returns: 

      task* - if sucessfull
	  NULL - if fails

   See Also:

*/
task_t*  pool_dequeue_task(pool_t* pool);

/*
   Function: pool_destroy

		Stops the threadpool and deallocates memory 
		Lets the remaining tasks complete

   Parameters:

		pool- pointer to the threadpool 

   Returns: 

      0 - if sucessfull

   See Also:

*/

int pool_destroy(pool_t* pool);

/*
   Function: thread_routine

		Contains the code each thread in the pool will carry out.

   Parameters:

		pool- pointer to the threadpool 

   Returns: 

      void

   See Also:

*/

int thread_routine(pool_t* pool);

/*
   Function: get_task

		gets a new task structure

   Parameters:

		none 

   Returns: 

      task_t*

   See Also:

*/
task_t* get_task();


/*
   Function: get_task_queue

		gets a new task_queue structure

   Parameters:

		none 

   Returns: 

      task_queue_t*

   See Also:

*/

task_queue_t* get_task_queue();

/*
   Function: set_running

		gets a new task_queue structure

   Parameters:

		none 

   Returns: 

      task_queue_t*

   See Also:

*/
void set_running();

#endif