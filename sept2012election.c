/*
 *  Install a signal handler and receive signal from shell.
 *  Run from shell in background and then:
 *  kill -USR1/-USR2/-KILL/-SIGUSR1/-SIGUSR2/-SIGKILL pid
 *
 *  Show:
 *  - managing different signal
 *  - ignore signal
 *  - use sleep and pause
 */

#include <stdio.h>
#include<stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <pthread.h>
#include <semaphore.h>

typedef struct election
{
  pthread_t thid;
  int rank;
  
}election_t;
 election_t *elect;


 typedef struct globals
 {
      int bestrank;
      pthread_t thleader;
      pthread_mutex_t mutex; 
      int count;
 }globals_t;
 globals_t glo;
 
int thnumber; 
sem_t sem;

int dorandom(int min, int max){
  int ran = (int)(rand() % (max - min + 1)) + min;
    return ran;
  } 

static void sigUsr1 (int signo) {
  if (signo == SIGUSR1)
    printf("Received SIGUSR1\n");
  else
    printf("Received wrong SIGNAL\n");

   
  return;
}


static void *thElection(void *argc ){
    election_t *myElect;

    myElect = (election_t * )argc;
    pthread_t myid;
    myid =pthread_self(); 
    //printf("inside thread %ld  rank %d \n",myElect->thid,myElect->rank);
    ///updating global rank
    pause();

    pthread_mutex_lock(&glo.mutex);
    if(glo.bestrank < myElect->rank){
      glo.bestrank=myElect->rank;
      glo.thleader=myid;
    }
    glo.count++;
    if(glo.count>=thnumber-3)
    {
      printf("\n count become %d  waking every thread \n",glo.count);
      for (int i = 0; i < thnumber-3; ++i)
      {
      sem_post(&sem);
      }
    }
    pthread_mutex_unlock(&glo.mutex);

    sem_wait(&sem);  // wait untill every thread finish
      
    printf("\n thread id \n my id %ld    rank %d   \n",
      myid,myElect->rank );

    printf("leader thread id %ld  best rank is %d  \n",
      glo.thleader,glo.bestrank );

    sleep(2);

    pthread_exit((void *)1);

}
 
int
main (int argc,char const *argv[]) {

  pthread_t *thElect;
  int status,rankrandom;
  
  if(argc!=2 ){printf("usage: number of threads \n");return 1;}

  if (signal(SIGUSR1, sigUsr1) == SIG_ERR) {
    fprintf (stderr, "Signal Handler Error.\n");
    return (1);
  }
 thnumber = atoi(argv[1]);
 if(thnumber<8) {printf("please enter greater than 8 \n");return 1;}

 pthread_mutex_init(&glo.mutex,NULL);

 sem_init(&sem,0,0);
 glo.count =0;
 glo.bestrank = 0;

 thElect = (pthread_t *) malloc (sizeof(pthread_t)*thnumber);

 elect = (election_t *) malloc (sizeof(election_t)*thnumber);

 for (int i = 0; i < thnumber; ++i)
 {
  rankrandom = dorandom(1,thnumber);

    elect[i].thid=i;
    elect[i].rank=rankrandom;

    pthread_create(&thElect[i],NULL,thElection,&elect[i]);
    sleep(1);
 }

int var[thnumber];
for (int i = 0; i < thnumber; i++) {     // fill array
    var[i] = i;
  }
for (int i = 0; i < thnumber; i++) {    // shuffle array to make it unique
  int temp = var[i];
  int randomIndex = rand() % thnumber;
  
  var[i]   = var[randomIndex];
  var[randomIndex] = temp;

}

int signalrandom;

 for (int i = 0; i < thnumber-3; ++i)
 {
  signalrandom=dorandom(2,5);
  sleep(signalrandom);

status = pthread_kill( thElect[var[i]], SIGUSR1);                                     
 if ( status <  0)                                                              
    perror("pthread_kill failed");

//printf("pthread_kill sent successfully to %ld  \n",thElect[var[i]]);
  
 }
sleep(3);
printf("\n exiting  \n");
  pthread_exit((void *)1);
}
