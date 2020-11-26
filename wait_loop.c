#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/wait.h>
#include<pthread.h>

int main()
{
    int i;
    pid_t pid, wpid, tmpid;
    for(i = 0;i < 5; i++ )
    {
      pid = fork();
      if (pid == 0)
        break;
      if(i == 2)
      {
        tmpid = pid;
        printf("--------pid = %d\n", tmpid);
      }
    }
   
    if (i == 5)
      {
        sleep(5);
        printf("I am parent , before waitpid, pid = %d\n", tmpid);
        wpid = waitpid(tmpid, NULL, WNOHANG);
        //wpid = waitpid(tmpid, NULL, 0);
         if (wpid == -1)
         {
            perror("waitpid error");
            exit(1);
         }
      printf("I'm parent, wait a child finish : %d\n", wpid);
      }
     else
      {
         sleep(i);
         printf("I'm %dth child, pid = %d\n", i + 1,getpid());
      }
    return 0;
}
