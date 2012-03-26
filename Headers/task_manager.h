#ifndef _TASK_MANAGER_

#define _TASK_MANAGER_

#ifdef _WIN32
#include "thread.h"
#include <windows.h>
#define WINTHREADS
#elif MACOS
	#include <sys/param.h>
	#include <sys/sysctl.h>
	#include <pthread.h>
#else
	#include <unistd.h>
	#include <pthread.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>
#include <errno.h>
#include "pool.h"

#define MIN_POOL_SIZE 4
#define MAX_POOL_SIZE 32

typedef struct task_manger_t
{
	int number_of_cores;
	pool_t* threadpool;
	pthread_mutex_t mutex;

}task_manager_t;


typedef struct pfor_t
{
	int start;
	int end;
	void*(*func)(int arg);
	void* args;

}pfor_t;
/*
   Function: task_manager_init

   Parameters:

		none

   Returns:
		0 if sucessfull

   See Also:

*/
int task_manager_init();
/*
   Function: task_manager_destroy

   Parameters:

		none

   Returns:
		0 if sucessfull

   See Also:

*/
int task_manager_destroy();
/*
   Function: get_number_of_cpu_cores

		gets a count of physical CPU cores available

   Parameters:

		none

   Returns:

      number of physical cores  if successfull

   See Also:

*/
int get_number_of_cpu_cores(); 

/*
   Function: spawn_task

		Allocates the memory for a new task struct
		Initialises the task with the parameters passed
		Calls the enqueue_task function 

   Parameters:

		function_ptr - a pointer to the function
		args - a pointer to the arguments for the supplied function

   Returns:

      0 - if successfull

   See Also:

*/
int spawn_task(void *function_ptr, void* args);

/*
   Function: par_for

       splits the input domain into the equal chunks (one for each physical processor core) 
	   carries out the provided function in parallel on each chunk of the function data as 
	   demarked by the input domain and offset 
   Parameters:

		input_start- the start point for the chunk of function data
		input_size - the size of the function data array
		function_ptr - a pointer to the function
		func_data - a pointer to the function data

   Returns:

      0 - if successfull

   See Also:

*/
int par_for(int input_size, void *function_ptr, void* args);



/*
   Function: pfor

   Parameters:

		struct pfor - 

   Returns:


   See Also:

*/
void pfor(void* args);

#endif