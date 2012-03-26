
#ifdef _WIN32
#include "thread.h"
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

#include "task_manger.h"

static int count;
pthread_barrier_t b;

void task1(int arg)
{
	int c = ++count;
	printf("Task %d running..\n",c);
	pthread_barrier_wait(&b);
	printf("Task %d finished \n",c);
	
}

int arr[] = {1,2,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,26,27,28,29,30,31,32,33};

void  par_task1(int i)
{
	arr[i] = arr[i] * arr[i];
	//pthread_barrier_wait(&b);
}





int main()
{
	//int x = pthread_barrier_init(&b,  34);
         
	task_manager_init();      
	int y;
	y = par_for(16, par_task1, arr);
	getchar();
	//pthread_barrier_wait(&b);

	int z;
	for(z=0;z<16;z++)
	{
	    printf("%d == %d\n",z+1, arr[z]);
	}

	getchar();
	task_manager_destroy();
	getchar();
	return 0;
}