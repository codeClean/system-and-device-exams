
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h> 
#include <fcntl.h>
#include <math.h>
/*
	-The threads reads the initial value of Xval
		-Calculate the ajacent values based on a gap
		-store the results into a queue (protecting with mutex)
		-then each of them release and signal (using thire own semaphores)
	- the main thread waits on the shared queue
		- reads the id of structure when it become available
		- then prints each x values from the structure
*/

typedef struct preSum {
	int steps;
	int *Xval;
	int *Result;
	sem_t *s1;			//a barrier for each steps
	
}preSum_t ;
preSum_t *data;
int thCount;			

//protect shared queue when synchronizing . . .
pthread_mutex_t lock= PTHREAD_MUTEX_INITIALIZER; 


int range;		// the number of elements generated with a range of [1-9];
int *thId;		// a shared queue storing thread ids(...storing struvt X-val values)
 

 //int test[8]  =  {2,4,6,1,3,5,8,7};


int init(int rang)
{ 
	int res; 


	data[0].Xval = (int *)malloc(sizeof(int)*rang);
	for (int i = 0; i < rang; ++i)
	{
		
		data[0].Xval[i] = rand()%10;		//init  data[0] with initial content of xval;
		//data[0].Xval[i] = test[i];

	}
}
int init_sem(int n ){
 
	for (int i = 0; i <= n; ++i)
	{
		data[i].s1 = (sem_t *)malloc (sizeof(sem_t));
		sem_init(data[i].s1,0,0);	//initially in blocking till value is added	

		data[i].Xval = (int *)malloc(sizeof(int)*range);
		data[i].Result = (int *)malloc(sizeof(int)*range);
	}
}
int doCalsulation(int index, int steps)
{
	int res =0 ;
	//printf("%d steps \n", steps );
	for (int i = 0; i < steps; ++i)
	{
		
		res = res + data[0].Xval[index] ; index--;
		if(index<0) return res;
		res = res + data[0].Xval[index] ; index--;
	}
	return res;
}

static void * Threads(void *argc){
	int id = *(int *)argc ;
	int Res;
	//id++;

	//sleep(8);
	//printf("thread %d \n",id );
	for (int i = 0; i < range; ++i)
	{
		int steps = pow(2,id);


		Res = doCalsulation(i,steps);  //the gap for each elements of Xval,

		//printf("check res %d  \n",Res );

		data[id].Result[i] = Res;

	}

	//printf(" locking \n");
	pthread_mutex_lock(&lock);

	
	thId[thCount] =id;
	thCount++;

	pthread_mutex_unlock(&lock);

	//printf("posting  threads %d  id at  %d \n",id,thId[thCount-1]);
	sem_post(data[id].s1);

	

 }
int main(int argc, char const *argv[])
{
	int n,*rc,*pi,r;
	pthread_t *th;
	void *retval;

	thCount=0;
	if (argc!=2) { printf("usage: n \n"); return 1; }
	n = atoi(argv[1]);
	
	range = pow(2,n);			//numeber of elements
	n++;
	thId= (int *)malloc(sizeof(int )*n);
	data = (preSum_t *)malloc(sizeof(preSum_t)*n);  
	rc = (int *)malloc(sizeof(int));

	init_sem(n);
	init(range);//this initializes data[0].Xval;

	th= (pthread_t *)malloc(sizeof(pthread_t)*n);
	n--;
	for (int i = 0; i < n ; ++i)
	{
		thId[i] = -1;
		pi = (int *)malloc(sizeof(int));
		*pi=i;
	 rc[i] = pthread_create(&th[i],NULL,Threads,pi);
	 sleep(1);
	}

	//printf("number elements = %d \n",range);


	for (int j = 0; j < range; ++j) 
		printf(" %d   ",data[0].Xval[j] ); printf("\n");

	//sleep(8);


	for (int k = 0; k < n; ++k)
	{
		//printf("waiting . . .   %d \n",k);
		while(1){


		 pthread_mutex_lock(&lock);		 
		 r = thId[k];
		pthread_mutex_unlock(&lock); 
		if(r !=-1)  break; }


		sem_wait(data[r].s1);
		//printf("got semaphore %d \n",thId[k]);
		 
		 
		 for (int z = 0; z < range; ++z)
		 {
		 	 printf(" %d   ",data[k].Result[z] );
		 }
		 printf("\n");
	}



	return 0;
}

