#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>


int main()
{
   pid_t pid = fork();
   if(pid == -1)
   {
     perror("fork error");
     exit(1);
    }
   
   else if(pid == 0)
   {
    //execlp("ls","ls","-l","-h",NULL);
    execl("/bin/ls","ls","-l",NULL);
    perror("exec error");
    exit(1);
   }
   
   else if(pid > 0)
   {
    printf("---parent process:my child is %d , my pid:%d , my parent pid: %d\n",pid,getpid(),getppid());
    }

   return 0;
}
