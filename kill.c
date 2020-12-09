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
     printf("parent, pid = %d\n", getpid());
     while(1);
   }
   else if(pid == 0)
   {
     printf("child pid = %d, ppid = %d\n",getpid(), getppid());
     sleep(2);
     kill(getppid(), SIGKILL);
   }
   return 0;
}
