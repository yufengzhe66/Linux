#include<stdio.h>
#include<sys/wait.h>
#include<signal.h>
#include<time.h>
#include<stdlib.h>
#include<sys/time.h>

void myfunc(int signo)
{
   printf("hello world\n"); 
}

int main()
{
  struct itimerval it, oldit;
  
  signal(SIGALRM, myfunc);
  
  it.it_value.tv_sec = 2;
  it.it_value.tv_usec = 0;
   
  it.it_interval.tv_sec = 5;
  it.it_interval.tv_usec = 0;

  if(setitimer(ITIMER_REAL, &it, &oldit) == -1)
    {
      perror("setitimer error");
      exit(-1);
    }
   while(1);
 
   return 0;
}
