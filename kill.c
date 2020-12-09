#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
#include<pthread.h>
#include<signal.h>

int main()
{
  pid_t pid = fork();
  if(pid > 0)
   {
     while(1)
     {
     printf("parent, pid = %d\n", getpid());
     sleep(1);
     }
   }
   else if(pid == 0)
   {
     printf("child pid = %d, ppid = %d\n",getpid(), getppid());
     sleep(10);
     kill(0, SIGKILL);
   }
   return 0;
}
