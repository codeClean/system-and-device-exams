  /* this car exam uses the classic producer-consumer example.
   * Illustrates mutexes and sem_trywait. 
   car data is used as a buffer 
    car thread inserts a stucture data and the pump thread reads the data
    and then the pump thread adds some values to the data and insert 
    the data into the buffer (uses another que name)
    finally the checkout thread reads from the buffer, do some calculation,
    then signals the car thread (to a specific car)  
   
   */


  #include <stdio.h>
  #include <stdlib.h>
  #include <unistd.h>
  #include "pthread.h"
  #include <time.h>
  #include <errno.h>
  #include <semaphore.h>

  #define BUFFER_SIZE 20

  #define diesel 1
  #define petrol 0


  typedef struct car
  {
    int cid;
    int fuelType;
    int queNum;
    int deliveryT;
    sem_t *semCar;
    
  }car_t;
  car_t *mycar; 


  typedef struct queue
  {
    car_t buffer[BUFFER_SIZE];	   /* the actual data */
    pthread_mutex_t lock;		       /* mutex ensuring exclusive access to buffer */
    int readpos, writepos, count;	/* positions for reading and writing */ 
    sem_t *notempty;              /* is the same as the number of data inserted to the buffer*/
    sem_t *notfull;               /* initialy buffersize but then it decreases whenever a data is read*/

  }queue_t;
  queue_t q1,q2,ck_q;

  int service;

  /* Initialize a buffer */
  static void
  init (queue_t *b)
  {
      pthread_mutex_init (&b->lock, NULL);

      b->notempty= (sem_t *)malloc(sizeof(sem_t));
      b->notfull= (sem_t *)malloc(sizeof(sem_t));
      sem_init(b->notempty,0,0);  //initailly noting is produced
      sem_init(b->notfull,0,BUFFER_SIZE); //maximum size of the buffer
      b->readpos = 0;
      b->writepos = 0;
      b->count = 0;
   } 

  /* Store a structure type in the buffer */
  static void
  put (  queue_t *b, car_t data)
  { 
      sem_wait(b->notfull);  /* Wait until buffer is not full */

      /* Write the data and advance write pointer */
      pthread_mutex_lock (&b->lock);
      b->buffer[b->writepos] = data;
      b->writepos++;
      if (b->writepos >= BUFFER_SIZE)
      b->writepos = 0;
      b->count++;
      pthread_mutex_unlock (&b->lock);

      
      sem_post(b->notempty);/* Signal that the buffer is now not empty */       
   }

  /* Read and remove a struct from the buffer */
  static car_t
  get (  queue_t *b)
  {
    sleep(2);
    car_t data;
    
    sem_wait(b->notempty); /* Wait until buffer has at least one value */

    /* Read the data and advance READ pointer */
    pthread_mutex_lock (&b->lock);
    data = b->buffer[b->readpos];
    b->readpos++;
    if (b->readpos >= BUFFER_SIZE)
      b->readpos = 0;
    b->count--;
    pthread_mutex_unlock (&b->lock);
    
    sem_post(b->notfull);/* Signal that the buffer is now not full */
    return data;
  }

 
  static void * thread_car (void *data)
  {
      car_t _car = *(car_t *)data;
      //  printf("car:-  car  %d to wait on queue %d \n\n",_car.cid,_car.queNum);
      
      if (_car.queNum==0) put (&q1, _car);
      else put (&q2, _car); 

      // printf("car:- waiting for check  \n" );
      sem_wait(_car.semCar);
      printf("car:- %d got the reciept  \n",_car.cid );

      //free(_car.semCar);  //used when car thread is created for forever
      return NULL;
  }

  static void *
  thread_pump (void *data)
  {
        int pid =  *(int *)data;
        int randServ,semresult,semval;
        printf("\npump:- inside pump %d  service time  %d\n",pid,service);

        car_t d; 

      while (1)
      {

          // printf(" pump %d:- is requesting to pump\n\n",pid);
          if (pid >=4)
          {
              semresult = sem_trywait(q1.notempty);
              if (semresult==-1 && errno==EAGAIN)
              {  printf("pump:- q1 is empty waiting on q2\n");
                  d =get(&q2);
              }
              else 
              { printf("pump:- q1 has cars \n");
                d =get(&q1);
              } 
          }
          else
          {
              semresult = sem_trywait(q2.notempty);
              if (semresult==-1 && errno==EAGAIN)
              { printf("pump:- q2 is empty waiting on q1\n");
                d =get(&q1);
              }
              else 
              {  printf("pump:- q2 already has cars \n");
                 d =get(&q2);
              } 
          }

          randServ = random()%service ;
          printf ("\n got car ---> %d, fuelType %d queue %d deliveryT = %d seconds \n\n",
          d.cid,d.fuelType,d.queNum,randServ);

          sleep(randServ);   /*delivery time */

          d.deliveryT=randServ;  //adding new value to the structure

          put(&ck_q,d);          //sending to checkout 

      } //end of while loop
  return NULL;
  }

  static void *thread_check(void *data)
  {
    while(1) 
      { 
          // printf("inside checkout geting car \n");
          float cost;
          car_t _car;
          _car= get(&ck_q);

          //printf("\n got car id %d Flue %d calculating delivery time %d \n",_car.cid,_car.fuelType,_car.deliveryT );

          if (_car.fuelType==diesel)  cost = _car.deliveryT * 1.4;

          else cost = _car.deliveryT * 1.6;

          printf("car id = %d queue number = %d  amount %f \n\n",_car.cid,_car.queNum,cost );

          sem_post(_car.semCar);
          printf ("checkout  :- waking  %d    --->\n",_car.cid );
      }
  }
  int
  main (int argc, const char * argv[])
  {
    pthread_t th_ch, *th_pump,*th_car;
    void *retval;
    int *pi,arrivals,intervals;
   

    if (argc != 4)
    {
      printf("usage: arrivals , intervals, service\n");
      return 0;
    }

    arrivals = atoi(argv[1]);
    intervals = atoi(argv[2]);
    service  = atoi(argv[3]);
    init(&q1);
    init(&q2);
    init(&ck_q);

  //////// pumps and checkout //////////////
        pthread_create(&th_ch,NULL,thread_check,NULL);
        sleep(1);
        th_pump = (pthread_t *)malloc(sizeof(pthread_t)*8);

        for (int i = 0; i < 8; ++i)
        {
           pi = (int *)malloc(sizeof(int));
           *pi =i+1;
           pthread_create(&th_pump[i],NULL,thread_pump,pi);
           sleep(1);
        }
  /////////// end pumps ///////////////

  int ar = random()%arrivals;
  int inter = random()%intervals;
  int randFuel, Q_num ;
  int carid=0;

   // while(1){
     printf("arrivals = %d intervals = %d service = %d \n\n",ar,inter,service );


      th_car = (pthread_t *) malloc(sizeof(pthread_t)*ar); 
      mycar = (car_t *)malloc (sizeof(car_t)*ar);
      int semval; 
      for (int i = 0; i < ar; ++i)
      {
          sleep(inter);              
          Q_num = random()%2;
          randFuel = random()%2;
          mycar[i].semCar= (sem_t *)malloc(sizeof(sem_t));
          sem_init(mycar[i].semCar,0,0);
          mycar[i].cid=i;
          mycar[i].queNum = Q_num;
          mycar[i].fuelType = randFuel;

          pthread_create(&th_car[i],NULL,thread_car,&mycar[i]);
           //carid++; //used for forever loop
      }
     // break;
      
    //}

  pthread_exit((void *)1);

  }
