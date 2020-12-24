#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<pthread.h>


struct thrd
{
  int var;
  char str[128];
};

void *tfn(void *arg)
{
  struct thrd *tval;
  
  tval = malloc(sizeof(struct thrd));
//  tval = malloc(sizeof(tval));
//  printf("%d\n",sizeof(tval));  
//  printf("%d\n",sizeof(struct thrd)); 
 
  tval->var = 100;
  strcpy(tval->str, "hello thread");
  return (void*)tval; 
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

  printf("child thread exit with var = %d, str = %s\n", retval->var, retval->str);

//  printf("%d\n",sizeof(char*));
  pthread_exit(NULL);

  return 0;
}
