#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<pthread.h>

void* tfunc(void *arg)
{
   while(1)
    {  
    printf("thread: pid = %d, tid = %lu\n",getpid(), pthread_self());
    sleep(1);
    }
   return NULL;
}

int main()
{
   pthread_t tid;
   

   int ret = pthread_create(&tid, NULL, tfunc, NULL);
   if(ret != 0)
      {
        fprintf(stderr, "pthread_create error:%s\n", strerror(ret));
        exit(1);
      }
   printf("main: pid = %d, tid = %lu\n", getpid(), pthread_self());
  
   sleep(5); 
  
   ret = pthread_cancel(tid);
   if(ret != 0)
      {
        fprintf(stderr, "pthread_cancel error:%s\n", strerror(ret));
        exit(1);
      }
   
   //while(1);
 
   pthread_exit((void*)0);

   return 0;
}
