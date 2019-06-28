#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <sys/wait.h>
#include<time.h>


typedef struct  dotproduct
{
	FILE *f1;
	FILE *f2;
	int pos;
	int *readpos;
	int sum; 
}dotproduct_t ;

dotproduct_t dotpro;
sem_t sem1,sem2;
pthread_mutex_t mutex;
int filesize;
int randomGlobal;


int dorandom(int min, int max){
	int ran = (int)(rand() % (max - min + 1)) + min;
	  return ran;
	}

int init (int t)
{
	
	dotpro.pos=0;
	dotpro.sum = 0;

	dotpro.f1 = fopen("fv1.b","rb");
	if (dotpro.f1==NULL) {  printf("error opnening file 1 to read \n");return 1;}
	

	dotpro.f2 = fopen("fv2.b","rb");
	if (dotpro.f2==NULL) {  printf("error opnening file 2 to read \n");return 1;}

	fseek(dotpro.f1,0,SEEK_END);
	long size1 = ftell(dotpro.f1);

     filesize = size1;
	 printf("ready to write fle size  new  %ld \n", size1);
	dotpro.readpos = (int *)malloc(sizeof(int)*filesize);

	rewind(dotpro.f1);
	rewind(dotpro.f2); 
	
	for (int i = 0; i < randomGlobal; i++) {     // fill array
    dotpro.readpos[i] = i;
	}

	for (int i = 0; i < randomGlobal; i++) {    // shuffle array to make it unique
    int temp = dotpro.readpos[i];
    int randomIndex = rand() % randomGlobal;

    dotpro.readpos[i]   = dotpro.readpos[randomIndex];
    dotpro.readpos[randomIndex] = temp;

	}
	//printf("file 1 size %ld file 2 size %ld \n",size1,size2 );
}


static void *threadProduct(void *argc){
	int id = *(int *)argc;
	int i; 

	//printf("i  got id %d \n",id );

	while(1)
	{ 
		sem_wait(&sem1);
		while(1){
		//printf("posted for %d \n", id );
		pthread_mutex_lock(&mutex);

		if(dotpro.pos > randomGlobal-1) {pthread_mutex_unlock(&mutex); break;}
		int randpos = dotpro.readpos[dotpro.pos]; // add the position 
		randpos  = sizeof(int)*randpos;
		//printf("thread  %d reading at position %d number of read %d \n",id,randpos,dotpro.pos );
		dotpro.pos++;
 		unsigned int vara,varb,mul;

		fseek(dotpro.f1,randpos,SEEK_SET);
		fseek(dotpro.f2,randpos,SEEK_SET);
		fread(&vara,sizeof(int),1,dotpro.f1);
		fread(&varb,sizeof(int),1,dotpro.f2);
		 
		mul=vara * varb;
		dotpro.sum += mul;

		rewind(dotpro.f1);
		rewind(dotpro.f2);
 
 printf("thread %d reading var a %d var b %d mult %d sum %d \n",id,vara ,varb,mul,dotpro.sum );


		pthread_mutex_unlock(&mutex);
		sleep(3);
	}
//printf(" reached end thread is %d \n",id );	
	sem_post(&sem2);
	}

}

int main(int argc, char const *argv[])
{
	int t, n,r, randval1, randval2;
	pthread_t *thProduct; 

	//srand(time(NULL));

	if (argc != 3)
	{
		printf("usage number of thread , random number \n");
		return 0;
	}

	t = atoi(argv[1]);
	n = atoi(argv[2]);

	FILE *f1,*f2;
	sem_init(&sem1,0,0);
	sem_init(&sem2,0,0);

	pthread_mutex_init(&mutex,NULL);

thProduct = (pthread_t *)malloc(sizeof(int)*t);

	for (int i = 0; i < t; ++i)
	{
		pthread_create(&thProduct[i],NULL,threadProduct,(void *)&i);
		sleep(1);
	}
	
for (int j = 1; j <=10; j++)
{
	
	r = dorandom(1,n); printf("%d number of random integers will be created for the %d time \n",r,j );
	int varrandom[r], varrandom2[r];
	randomGlobal= r;

	for (int i = 0; i <r; ++i)
	{ 
		randval1 = dorandom(0,1000);
		varrandom[i]=randval1;

		randval2 = dorandom(0,100);
		varrandom2[i]=randval2;
	}

	 f1 =  fopen("fv1.b","wb");
	 if (f1==NULL)  { printf("error opning for write on file1 \n"); exit(0);	 }

	 fwrite(varrandom,sizeof(int),r,f1);
	 fclose(f1);


	 f2=  fopen("fv2.b","wb");
	 if (f2==NULL)  { printf("error opning for write on file2 \n"); exit(0);	 }

	 fwrite(varrandom2,sizeof(int),r,f2);
	 fclose(f2);

	init(t);
	//printf("\n ready to write for %d times\n", j);  

	for (int i = 0; i < t; ++i) sem_post(&sem1);
	 
	sleep(3);

	for (int i = 0; i < t; ++i) sem_wait(&sem2);
	
	 
	 unsigned int vara[filesize],varb[filesize],mul,sum=0;

	 fread(vara,sizeof(int),filesize,dotpro.f1);

	 fread(varb,sizeof(int),filesize,dotpro.f2);

 	  printf("\n\n  in main \n");
	 for (int i = 0; i < r; ++i)
	 {
	 	mul = vara[i] * varb[i];
	 	sum +=  mul;

	 	printf("var a %d var b %d mult %d sum %d \n",vara[i],varb[i],mul,sum );
	 }

	 printf("computed by the threads %d \n\n",dotpro.sum ); 

	 sleep(3);

}
 
	return 0;
}
