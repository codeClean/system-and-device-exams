
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <string.h>
#include <time.h> 
#include <fcntl.h>

#define BUFFER_SIZE 100 //will be changed after the argument is recieved

typedef struct _passengers
{
	 int id;
	int countPass;
	int stationStarting; 
	int stationDestination;		// one for the starting station the second destination station
	int flagme;		
}passengers_t;
passengers_t *passenger;

typedef struct _train
{
	int id;
	int stationId; 
	int passIds[100];			// the passengers currently in this train
	int passIndex;	
	int passOut;
	int passIn;	
	
}train_t;
train_t *train;

typedef struct _station
{
	sem_t *sem_Red;  			//block the currently in the  previous  station;

 	pthread_mutex_t lock;		 

	passengers_t *passInStation;
	int readpos,writepos,count;
	sem_t *notEmpty;
	sem_t *notFull;
			
}station_t;
station_t *station;

typedef struct pipes
{
	int fp[2];
	
}pipes_t ;
pipes_t *pps;
 


int T_TR,TRAIN_CAPACITY,T_MAX,T_NewPaas,L;
int fd;
int fp[2];

int dorandom(int min, int max){
	int ran = (int)(rand() % (max - min + 1)) + min;
	  return ran;
	}

static void init (station_t *b)	
{
	pthread_mutex_init (&b->lock,NULL);
	b->passInStation = (passengers_t *)malloc(sizeof(passengers_t)*L);
	b->sem_Red = (sem_t *)malloc(sizeof(sem_t));
	b->notEmpty = (sem_t *)malloc(sizeof(sem_t));
	b->notFull = (sem_t *)malloc(sizeof(sem_t));
	sem_init(b->notEmpty,0,0);
	sem_init(b->notFull,0,L);
	sem_init(b->sem_Red,0,1);
	b->readpos=0;
	b->writepos=0;
	b->count=0;
}

static void put(station_t *b, passengers_t data)
{
	pthread_mutex_lock (&b->lock);

	sem_wait(b->notFull); 

	b->passInStation[b->writepos]=data;
	b->writepos++;
	if(b->writepos>=L) b->writepos=0;
	b->count++;

	sem_post(b->notEmpty);
	pthread_mutex_unlock (&b->lock);

}

#define OVER (-1)

static passengers_t get (station_t *b)
{
	struct timespec ts;
	passengers_t data;
	int s; 

/////////////////////sem_timedwait //////////////////	
		ts.tv_sec = time (NULL) + T_MAX;
		ts.tv_nsec = 0;


		while ((s = sem_timedwait(b->notEmpty,&ts)) == -1 && errno == EINTR)
		continue;       /* Restart if interrupted by handler */

		/* Check what happened */

		if (s == -1) {
			if (errno == ETIMEDOUT)
			{
				//printf(" timed out\n");
				data.flagme=OVER;
				return data;
			}
			else
			perror("sem_timedwait");
		} 
//////////end sem_timedwait /////////////////////				 

	data = b->passInStation[b->readpos];
 	b->readpos++;
	if(b->readpos >= L) b->readpos=0;	

	b->count--;
	sem_post(b->notFull);
	return data;
}

static void *threadTrain(void *argc)
{
	int id = *(int *)argc;
	int next_station,semval;


	while(1)
	{
 				char buffer[100];
				int in,innew;
				passengers_t passLoco;
				next_station =  (train[id].stationId + 1) % 4;

				pthread_mutex_lock(&station[next_station].lock);        //this doesnt affect for now 


				sem_wait(station[next_station].sem_Red); //block the  train in the privious station


				train[id].id = id;
				int trainPassIndex = train[id].passIndex;
				train[id].passOut=0;
////////////get off the train//////////////
				train[id].passIndex = 0 ;
				for (int j = 0; j < trainPassIndex; ++j)
				{
					int passengerid = train[id].passIds[j];

					if (train[id].stationId == passenger[passengerid].stationDestination)
					{  
					train[id].passOut++;
					}
					else
					{
					train[id].passIds[train[id].passIndex] = train[id].passIds[j]; 
					train[id].passIndex++; 
					}
				}
//////////end get off the train/////////////

/////////////////load the new passengers ///////////////				
				train[id].passIn=0;
				while(1)
				{
					passLoco = get(&station[train[id].stationId]);
	 				if (passLoco.flagme==OVER || train[id].passIndex >= TRAIN_CAPACITY) 
					{
						passenger[passLoco.id].flagme=0;
						break;
					}
					train[id].passIds[train[id].passIndex] = passLoco.id;
					train[id].passIndex++; 
					train[id].passIn++; 
				}
////////////////end loading //////////////////////////
 
				sprintf(buffer," Train  %d   station  %d ",id,train[id].stationId);

				//printf("\n%s\n",buffer );
				in = write(fd,buffer,strlen(buffer));
				if(in < 0  ) { printf("error writing \n"); exit(0); }

				for (int i = 0; i < train[id].passIndex; ++i)
				{
					sprintf(buffer," \t %d ",train[id].passIds[i]); 
					in = write(fd,buffer,strlen(buffer)); 
					if(in  <0) { printf("error writing \n"); exit(0); }
				}
				innew = write(fd,"\n",1);

				//int numOfPassdest = station[train[id].stationId].passIndexDest;
				sprintf(buffer," Destination    \t");

				in = write(fd,buffer,strlen(buffer));
				if(in < 0){printf("error writing \n"); exit(0); }


				//printf(" \n  Destination   \t ");
				for (int i = 0; i < train[id].passIndex; ++i)
				{
					int passengerid = train[id].passIds[i];
					sprintf(buffer,"\t %d ", passenger[passengerid].stationDestination );
					in = write(fd,buffer,strlen(buffer)); 
					if(in  <0) { printf("error writing \n"); exit(0); }
				}
				innew = write(fd,"\n\n",2);


				if(write(pps[train[id].stationId].fp[1],&train[id],sizeof(train[id]))<0){
				printf("error writing in pipe \n");
				exit(0);
				}

				printf("\n\n"); 

				sem_post(station[train[id].stationId].sem_Red);					// free the station 
				pthread_mutex_unlock(&station[next_station].lock);
				sleep(T_TR); 				// travelling to the next station 
				train[id].stationId = next_station;

				}
}
static void *threadPass(void *argc)
{
	int id = *(int *)argc;
	int  i,destSt,startSt;

	//printf("inside thread passenger  %d . \n",id);

	while(1){
	 	srand(time(NULL)); 
		destSt =random()%4;  
		startSt = random()%4; 
		if (startSt != destSt) break; //if equal, do random again

	 } 
	passenger[id].flagme=0;
	passenger[id].id =id;
	passenger[id].stationStarting =startSt;
	passenger[id].stationDestination=destSt;

	put(&station[startSt],passenger[id]); 
	//printf("inside thread passenger  %d inserting in station %d  . \n",id,passenger[id].stationStarting);
 	 
}
static void *threadStaion(void *argc)
{
	int id = *(int *)argc; 
	int n;
 	train_t train;
 	while(1){

		if ( (n = read(pps[id].fp[0], &train, sizeof(train))) <= 0)
		{
		printf( "error  reading from the pipe");
		exit(0);
		}  

	printf("station  : %d train %d  passengers in : %d  passengers out : %d \n", id,train.id,train.passIn,train.passOut);
	}
}

int main(int argc, char const *argv[])
{

		//declarations 	
			int *vet;
			pthread_t *thTrain,*thStation,*thPassenger;

			if (argc != 6)
			{
				printf("%s usage: T_TR,TRAIN_CAPACITY,T_MAX,T_NewPaas,L \n", argv[0]);
				return 0;
			}

			T_TR           = atoi(argv[1]);
			TRAIN_CAPACITY = atoi(argv[2]);
			T_MAX          = atoi(argv[3]);
			T_NewPaas      = atoi(argv[4]);
			L              = atoi(argv[5]);
			printf(" T_TR = %d ,TRAIN_CAPACITY = %d ,T_MAX = %d,T_NewPaas = %d,L =%d\n\n", 
			T_TR ,TRAIN_CAPACITY ,T_MAX ,T_NewPaas ,L );

		//end declarations 	

		//Initialization 
			if(pipe(fp)<0){printf("error while piping \n"); exit(0);}
			fd=open("log.txt",O_CREAT|O_RDWR|O_TRUNC,0775);
			if(fd<0)
			{
				printf("error opening log file \n");
				return -1;
			}
			printf("all train information will be loged in log.txt \n");
			thTrain =(pthread_t *)malloc(sizeof(pthread_t)*2);
			thStation =(pthread_t *)malloc(sizeof(pthread_t)*4);
			pps =(pipes_t *)malloc(sizeof(pipes_t)*4);
			thPassenger =(pthread_t *)malloc(sizeof(pthread_t)*L);
			vet =(int *)malloc(sizeof(int )*4); 

			train = (train_t *)malloc(sizeof(train_t)*2);
			station = (station_t *)malloc(sizeof(station_t)*4);
			passenger = ( passengers_t *)malloc(sizeof(passengers_t)* L);
		

			for (int st = 0; st < 4; ++st)
			{
				if(pipe(pps[st].fp)<0){printf("error piping pipe %d\n",st );}
				init(&station[st]);
			}
		// asssigning train a unique station id
			int  j,k;
			for (int i=0;i<4;i++)
		    vet[i] = i; 

 			k = 4;
			for (int i=0;i<2;i++) {
				j = rand() % k;
				train[i].stationId = vet[j] % 4;  
 				vet[j] = vet[k-1];
				k--;
		  } 
		//end assignment;

		 
		//create threads 
		  	
		  	for (int i = 0; i < 4; ++i)
		  	{
		  		pthread_create(&thStation[i],NULL,threadStaion,(void *)&i);
		  		sleep(1);
		  	}
		  	sleep(3);
		  	for (int i = 0; i < 2; ++i)
		  	{
		  		 pthread_create(&thTrain[i],NULL,threadTrain,(void *)&i);
		  		 sleep(2);
		  	}
		  	int randPass;
		  	for (int i = 0; i < L; ++i)
		  	{
		  		randPass = dorandom(0,T_NewPaas);
		  		sleep(randPass);
		  		pthread_create(&thPassenger[i],NULL,threadPass,(void *)&i);		  		
		  	}
		//end creation of threads  

		pthread_exit((void *)1);
}