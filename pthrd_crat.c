#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<errno.h>
#include<pthread.h>

void sys_err(const char *str)
{
  perror(str);
  exit(1);
}

void* tfunc(void *arg)
{
   printf("thread: pid = %d, tid = %lu\n",getpid(), pthread_self());
   return NULL;
}

int main()
{
   pthread_t tid;
   
   printf("main: pid = %d, tid = %lu\n", getpid(), pthread_self());

   int ret = pthread_create(&tid, NULL, tfunc, NULL);
   if(ret != 0)
     perror("pthread_create error");
   sleep(1);
   return 0;
}
