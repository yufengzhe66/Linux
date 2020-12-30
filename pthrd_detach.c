#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<pthread.h>


void* tfunc(void *arg)
{
   printf("thread: pid = %d, tid = %lu\n",getpid(), pthread_self());
   return NULL;
}

int main()
{
   pthread_t tid;
   

   int ret = pthread_create(&tid, NULL, tfunc, NULL);
   if(ret != 0)
     {
       fprintf(stderr,"pthread_create error:%s\n",strerror(ret));
       exit(1);
     }

   ret = pthread_detach(tid);                                    //设置线程分离
   if(ret != 0)
     {
       fprintf(stderr,"pthread_detach error:%s\n",strerror(ret));
       exit(1);
     }
  
   sleep(1); 
  
   ret = pthread_join(tid, NULL);
   if(ret != 0)
     {
       fprintf(stderr,"pthread_join error:%s\n",strerror(ret));
       exit(1);
     }

   printf("main: pid = %d, tid = %lu\n", getpid(), pthread_self());
   sleep(1);
   
  
   pthread_exit((void*)0);
}
