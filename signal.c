#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>
#include<pthread.h>

void sys_err(const char *str)
{
   perror(str);
   exit(1);
}
void sig_catch(int signo)
{
   printf("catch you!! %d\n", signo);
   return;
}

int main()
{
  signal(SIGINT, sig_catch);
  while(1);
}
