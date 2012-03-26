
#include "task_manger.h"
#include "pool.h"
#include <stdlib.h>
#include <math.h>

#define TYPE int

#include <stdio.h>

int array1[] = {1,2,3,4};
int array2[2];


/* GLOBAL VARIABLES */
task_manager_t* task_man;


/* FUNCTION TO GET NUMBER OF CPU CORES */
int get_number_of_cpu_cores() 
{
    #ifdef WIN32
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return sysinfo.dwNumberOfProcessors;
    #elif MACOS
        int nm[2];
        size_t len = 4;
        uint32_t count;
 
        nm[0] = CTL_HW; nm[1] = HW_AVAILCPU;
        sysctl(nm, 2, &count, &len, NULL, 0);
 
        if(count < 1) 
		{
            nm[1] = HW_NCPU;
            sysctl(nm, 2, &count, &len, NULL, 0);
            if(count < 1) { count = 1; }
        }
        return count;
    #else
        return sysconf(_SC_NPROCESSORS_ONLN);
    #endif
}

/* TASK MANAGER INITIALIZER FUNCTION */
int task_manager_init()
{
	int core_count;
	task_man = NULL;

	/* ALLOCATE MEMORY FOR TASK MANAGER */
	if((task_man = (task_manager_t*)malloc(sizeof(task_manager_t)))==NULL)
		return 1;

	/* INITIALIZE CORE COUNT */
	if((core_count = get_number_of_cpu_cores()) > 0)
		task_man->number_of_cores = core_count;
	else
		task_man->number_of_cores = 1;

	/* INITIALIZE THREAD POOL */
	if(core_count < MIN_POOL_SIZE)
	{
		/* INITIALIZE THREAD POOL TO MIN_POOL_SIZE */
		if((task_man->threadpool = pool_init(MIN_POOL_SIZE)) == NULL)
		{
			free(task_man);
			return 1;
		}
	}
	else
	{   /* INITIALIZE THREAD POOL TO DOUBLE THE CORE COUNT */
	    if((task_man->threadpool = pool_init(core_count * 2)) == NULL)
		{
			free(task_man);
			return 1;
		}
	}

	/* INITIALIZE QUEUE MUTEX */
	if(pthread_mutex_init(&(task_man->mutex),NULL) != 0)
	{
		free(task_man);
		return NULL;
	}

	return 0;
}

int task_manager_destroy()
{
	if(task_man)
	{
		pool_destroy(task_man->threadpool);
		pthread_mutex_destroy(&task_man->mutex);
		free(task_man);
	}
	return 0;
}


/* SPAWN TASK */
int spawn_task(void *function_ptr, void* args)
{
	create_task(task_man->threadpool, function_ptr,args);
	return 0;
}

/* PARALLEL FOR LOOP */
int par_for(int input_size, void *function_ptr, void* args)
{
	int i_count;
	/* GET THE QUOTIENT AND REMAINDER OF THE ARRAY SIZE DIVIDED BY THE CORE COUNT */
	div_t  temp;
	temp = div(input_size, task_man->number_of_cores);
	int quotient = temp.quot;
	int remainder = temp.rem;
	/* IF THE INPUT SIZE IS SMALLER THAN THE NUMBER OF CORES CREATE A TASK FOR EACH ARRAY ELEMENT */
	if(quotient *task_man->number_of_cores < task_man->number_of_cores)
	{	
	    for(i_count = 0; i_count < remainder; i_count++)
			create_task(task_man->threadpool, function_ptr,(void*)i_count);
	}
	else/* DIVIDE ARRAY INTO n CHUNKS */
	{
	    for(i_count = 0; i_count < task_man->number_of_cores ; i_count++)
	    {
		    int args[2];
			args[0] = i_count * quotient;
			/* IF LAST CHUNK */
			if(i_count ==  (task_man->number_of_cores)-1)
			{
			    if(remainder == 0)
					args[1] = input_size;
				else/* TAG ON THE REMAINDER */
					args[1] = quotient* task_man->number_of_cores + remainder;
			}
			else/*  */
			{
			    args[1] = quotient* (i_count +1);
			}
			int z = (temp.quot * i_count);

			pfor_t *temp;
			if((temp = (pfor_t*)malloc(sizeof(pfor_t)))!=NULL)
			{	
				temp->args = args;
				temp->func = (void*(*)(int arg))function_ptr;
				temp->start = args[0];
				temp->end = args[1];
				spawn_task(pfor, temp);
			}
	    }
	}
	return 0;
}



void pfor(void* args)
{
	int i;
	pfor_t* pfor;
	pfor = (pfor_t*)args;
	void * (*pfor_func)(int arg);
	void * pfor_args;

	pfor_func = pfor->func;
	pfor_args = pfor->args;

	for(i = pfor->start; i < pfor->end; i++)
		pfor_func(i);

}




