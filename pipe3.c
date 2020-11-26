#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<errno.h>
#include<pthread.h>

void sys_err(const char *str)
{
   perror(str);
   exit(1);
}

int main()
{
   int ret;
   int fd[2];
   pid_t pid;
   int i;
   char buf[1024];   
   int n;  
  
   ret = pipe(fd);
   if(ret == -1)
     sys_err("pipe error");
  
   for(i = 0; i < 2; i++)
   {
   if((pid = fork()) == 0)
     break;
   if(pid == -1)
      {
       sys_err("fork error");
       exit(1);
      }
   }
   
   if(i == 0)
   {
     close(fd[0]);
     write(fd[1],"1.hello\n",strlen("1.hello\n"));
   }
   else if(i == 1)
   {
     close(fd[0]);
     write(fd[1],"2.world\n",strlen("2.world\n"));
   }

   else
   {
     close(fd[1]);
     sleep(2);
     n = read(fd[0],buf,1024);
     write(STDOUT_FILENO,buf,n);
     for(i = 0;i < 2; i++)
       wait(NULL); 
   }
   return 0;
}
