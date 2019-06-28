#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include "pthread.h"

typedef struct  
{
	pthread_mutex_t mutex;
	pthread_cond_t cond;
	int count;
	int *checked;
	int workingFlag; 
}mycond_t ;

mycond_t mycond;
int n;

void init()
{
	pthread_mutex_init(&mycond.mutex,NULL);
	pthread_cond_init(&mycond.cond,NULL);
	mycond.count=0;
	mycond.checked= (int *)malloc(sizeof(int)*n);
	mycond.workingFlag =0;
}
 bool isprime(int n)
{
	int c;
	if (n <2) return false;

	for(c= 2; c<= n/2; c++)
		if((n % c) == 0) return false;

	return true;
}


static void * searchprime(void *argc)
{
	int id = *(int *)argc; 
	int randval,i;
	while(1){
		sleep(1);
		randval = rand();  //srand(time(0)); //you can use this to make the random not redundant ;
		if(isprime(randval)){ 	//do nothing   }
		else { continue; } 
		 
		pthread_mutex_lock(&mycond.mutex); 
		while(mycond.workingFlag==1){
			pthread_cond_wait(&mycond.cond,&mycond.mutex);
		}
		mycond.workingFlag=1; 
		int flagcheck=0;
		// search if the prime number is already checked 
		for (i = 0; i < mycond.count; i++)
		{
			if (randval == mycond.checked[i])
			{
				flagcheck=1;
				printf("the number is  already checked: dont increase count \n");
				continue;
			}
		}
		if(flagcheck==0){
			mycond.checked[mycond.count]=randval;
			mycond.count++; 
			printf("thread %d adding new prime %d count become %d \n",
				id,mycond.checked[mycond.count]=randval,mycond.count);
			if(mycond.count >=n){printf("got all primes: exiting  \n"); exit(0);}
		} 
		//printf("inside while: pid  %d  count become %d  checked number %d \n\n",
			//id ,mycond.count,mycond.checked[mycond.count-1]); 
		mycond.workingFlag=0;
		pthread_cond_signal(&mycond.cond);
		pthread_mutex_unlock(&mycond.mutex);
	}
	
}

int main(int argc, char const *argv[])
{
	pthread_t *thsearch;
	int t;

	if (argc!=3)
	{
		printf("usage: number of thread, maximum prime number \n");
		return 0;
	}
	t = atoi(argv[1]);
	n =	atoi(argv[2]);

	init();
	
	printf("%d numeber of prime numeber will be searched with %d threads \n",n,t );
for (int i = 0; i < t; i++)
{	

	//printf("the %d th thread \n",i);
	pthread_create(&thsearch[i],NULL,searchprime,(void *)&i);
	sleep(1);
}
	
	pthread_exit((void *)0);
}