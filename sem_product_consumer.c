#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<pthread.h>
#include<semaphore.h>

#define NUM 5
int queue[NUM];

sem_t product_number, blank_number;


void* producer(void *args)
{
   int i = 0;
   while(1)
   {
     sem_wait(&blank_number);
     queue[i] = rand() % 100 + 1;
     printf("-------Produce----%d\n", queue[i]);
     sem_post(&product_number);
     
     i = (i + 1) % NUM ;
     sleep(rand() % 2);
   }
   return NULL;
}

void* consumer(void *args)
{
   int i = 0;
   while(1)
   {
      sem_wait(&product_number);
      printf("-Consume---%d\n", queue[i]);
      queue[i] = 0;
      sem_post(&blank_number);

      i = (i + 1) % NUM;
      sleep(rand() % 3); 
   }
   return NULL;
}

int main()
{
  pthread_t pid, cid;

  srand(time(NULL));  
 
  sem_init(&blank_number, 0, NUM);
  sem_init(&product_number, 0, 0);
  
  pthread_create(&pid, NULL, producer, NULL);
  pthread_create(&pid, NULL, consumer, NULL);

  pthread_join(pid, NULL);
  pthread_join(cid, NULL);

  sem_destroy(&blank_number);
  sem_destroy(&product_number);
  
  return 0;
}
