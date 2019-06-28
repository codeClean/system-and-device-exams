#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <stdbool.h>
#include <semaphore.h>
#include "pthread.h"

#define maxbuff 100
typedef struct chlorine
{
	sem_t sem;
	int idNa;
	
}chlorine_t ;        // this struct will be used to wait for a sepecific semaphore by chlorin id
					 // and the idNa will be used to save the id of Na that singl this chlorine

typedef struct NaCl
{
		pthread_mutex_t mutex; 
		int myindex;  
		int var[maxbuff]; 
		
		chlorine_t *ListCl;

}NaCl_t ;
sem_t sem1,sem2;
NaCl_t Na,Cl;
int k;

int dorandom(int min, int max){
	int ran = (int)(rand() % (max - min + 1)) + min;
	  return ran;
	}

static void * threadNa (void *argc){
			int id = * (int *) argc; //printf("Sodium thread with id %d \n", id); 
			int clid;				
			

			sem_wait(&sem1);
			pthread_mutex_lock(&Na.mutex);

			clid = Cl.var[Na.myindex] ;			//  Na.myindex is the index that is used to get the clorin id

            //printf("some chlorine is available at index %d with chlorine id %d \n",Na.myindex,clid );

			Na.myindex++;	
			 pthread_mutex_unlock(&Na.mutex); 

			 Cl.ListCl[clid].idNa=id; 			//save my thread is in the indexed chlorine struct 

			//printf("saving my id %d into chlorine threads  with id %d \n",Cl.ListCl[clid].idNa,clid );

			printf(" id %d -  Na %d  Cl %d             ", id, Cl.ListCl[clid].idNa, clid );

			sem_post(&Cl.ListCl[clid].sem);
			sem_wait(&sem2);

			
			sleep(2);
}

static void *threadCl(void *argc){

		int id = * (int * ) argc ;  	//printf("\n i am chlorine thread %d \n",id); 
		

		pthread_mutex_lock(&Cl.mutex);

		Cl.var[Cl.myindex]=id; 
		//printf("current chlorine index is %d and its value is %d\n",Cl.myindex,Cl.var[Cl.myindex] );
  
  		Cl.myindex++; 
		 
		sem_post(&sem1);	// wake any sodium thread

		pthread_mutex_unlock(&Cl.mutex);

		sleep(3);

		sem_wait(&Cl.ListCl[id].sem);   // wait with my own id. the sodium thread knows me 

		printf("  id %d -  Na %d  Cl %d \n", id, Cl.ListCl[id].idNa, id );

		sleep(2);
		sem_post(&sem2);           // sem2 is used to block other sodium before chlorine thread finishes

}

static void *createNa(void *argc){

	pthread_t *thNa; 
	int randval;

	thNa = (pthread_t *) malloc (sizeof(pthread_t)* k);
	for (int i = 0; i < k; ++i)
	{ 
		pthread_create(&thNa[i],NULL,threadNa,(void *)&i);
		randval  = dorandom(0,4);
		sleep(randval);
		sleep(1);		
	}
}

static void *createCl(void *argc){

	pthread_t *thCl; 
	int randval;

	thCl = (pthread_t *) malloc (sizeof(pthread_t)* k);

	for (int i = k; i < k*2; ++i)
	{ 
 		sem_init(&Cl.ListCl[i].sem,0,0);
		 pthread_create(&thCl[k],NULL,threadCl,(void *)&i);
		randval  = dorandom(0,4);
		sleep(randval);
		sleep(1); 
	}
}
int main(int argc, char const *argv[])
{
		pthread_t tha,thb;

		if (argc!=2)
		{
		printf("usage : k  \n");
		return 0;
		}
		k=atoi(argv[1]);
		
		sem_init(&sem1,0,0);

		sem_init(&sem2,0,0);

		printf("you entered %d \n",k );
		pthread_mutex_init(&Na.mutex,NULL);
		pthread_mutex_init(&Cl.mutex,NULL);

		Cl.ListCl = (chlorine_t *)malloc(sizeof(chlorine_t)*k);
		pthread_create(&tha, NULL,createNa,NULL);
		pthread_create(&tha, NULL,createCl,NULL);


		pthread_exit((void *) 1);
}