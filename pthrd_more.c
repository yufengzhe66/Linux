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

//传参采用强转类型方式也可，但会报警告  void*(i)   (int)arg


void* tfunc(void *arg)
{
   int i = *(int*)arg;              //强转 + 赋值
   sleep(i);                        //确保按顺序打印
   printf("I'm %dth thread: pid = %d, tid = %lu\n", i + 1 , getpid(), pthread_self());
   return NULL;
}

int main()
{
   int ret;
   pthread_t tid;
   int i;
      
   for(i = 0; i < 5; i++)
   {
      ret = pthread_create(&tid, NULL, tfunc, &i);
      if(ret != 0)
         perror("pthread_create error");
      sleep(i + 1);                             //传址调用必须延迟，否则会导致回调函数打印时出错
   }
   
   printf("main: pid = %d, tid = %lu\n", getpid(), pthread_self());
     
   return 0;
}
