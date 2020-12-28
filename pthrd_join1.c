#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<pthread.h>



void *tfn(void *arg)
{
  return (void*)74; 
}

int main()
{
  pthread_t tid;
  struct thrd *retval;

  int ret = pthread_create(&tid, NULL, tfn, NULL);
  if(ret != 0)
    perror("pthread_create error");
  
  ret = pthread_join(tid, (void**)&retval);
  if(ret != 0)
    perror("pthread_join error");

  printf("child thread exit with %d\n",retval);
  pthread_exit(NULL);
  return 0;
}
