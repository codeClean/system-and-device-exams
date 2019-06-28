#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "pthread.h"
#include "semaphore.h"

typedef struct List_elem
{
  sem_t *sem;
  int priority;
  struct List_elem *next;
} List_elem;

typedef struct Sem
{
	pthread_mutex_t mutex;
	int cnt;
	List_elem *sem_list;
} mySem ;

mySem *S1;

typedef struct  threadvals
{
	long int thid;
	int priority;
	int N; 
}threadvals_t;

int dorandom(int min, int max){
	int ran = (int)(rand() % (max - min + 1)) + min;
	  return ran;
	}

mySem  * myinit(int value){
		
		mySem *s;
		s=(mySem * )malloc(sizeof(mySem));
		s->cnt=value;
 		pthread_mutex_init(&s->mutex,NULL);
		s->sem_list = (List_elem *)malloc(sizeof(List_elem));
		s->sem_list->priority =-1;
		s->sem_list->sem = (sem_t *) malloc(sizeof(sem_t));
		sem_init(s->sem_list->sem,0,1);
		s->sem_list->next = (List_elem *) malloc(sizeof(List_elem));
		s->sem_list->next->priority = 100000;
		s->sem_list->next->next = NULL;


		return s; 

	}

	

sem_t * enque(List_elem *head, int priority){
	List_elem *newElem,*p;

	p = head;
	while(priority > p->next->priority){

		p= p->next;
	}

	newElem  = (List_elem *)malloc (sizeof(List_elem));
	newElem->sem = (sem_t *)malloc (sizeof (sem_t));
	sem_init (newElem->sem, 0, 0);
	newElem->priority = priority;
    newElem->next = p->next; 
	p->next = newElem;

	return newElem->sem;

}

void mywait(mySem *s, int priority){
	sem_t *newSem;

 		 pthread_mutex_lock(&s->mutex); 
		if(--s->cnt < 0){
			//printf("count become %d \n", s->cnt );

			newSem = enque(s->sem_list,priority);

			pthread_mutex_unlock(&s->mutex);
			// printf("unlocked  \n");

			sem_wait(newSem);

		}
	 else
	 {
	 	pthread_mutex_unlock(&s->mutex);
	 } 
}
sem_t * mydeque (List_elem *head){

	List_elem *newElem;
	sem_t *s ;
	s = head->next->sem;  
	head->next = head->next->next;

	return s;
}
 
void mysignal (mySem *s){ 

		sem_t *newSem;

		pthread_mutex_lock(&s->mutex); 

		//printf("about to signal coutnt %d  \n",s->cnt);
		if(++s->cnt < 0){

		newSem = mydeque(s->sem_list);

		pthread_mutex_unlock(&s->mutex);

		sem_post(newSem);

		}
		else pthread_mutex_unlock(&s->mutex);
}

static void *threads (void * argc){

	int *class = (int *)argc;
	int c = *class;

	while(1){

		printf("got the class %d  thread %ld \n",c, pthread_self()); 

		mywait(S1,c);
		printf("\n\n");

		printf("got response %d  thread %ld \n",c, pthread_self()); 

	 

		sleep(dorandom(2,10));

	}


}

int main(int argc, char const *argv[])
{
	int k,*pi;
	pthread_t *th;


	S1 = myinit(0);


	if (argc != 2)
	{
		printf("usage : k\n");
		return 0;
	}

	k = atoi(argv[1]);
	th = (pthread_t *)malloc(sizeof(pthread_t)*k); 

	for (int i = 0; i < k; ++i)
	{
 		pi = (int *)malloc(sizeof(int));
		*pi =dorandom(1,3);
		
		pthread_create(&th[i],NULL,threads,pi) ;
		sleep(1);

	}

   //// can be seen as critical section
	 while(1){
	 	sleep(4);
		mysignal(S1);

		
	}
	pthread_exit((void *)1);
}