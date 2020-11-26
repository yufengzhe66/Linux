#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
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
   char *str = "hello pipe\n";
   char buf[1024];


   ret = pipe(fd);
   if(ret == -1)
     {
       sys_err("pipe error");
     }
   
   pid = fork();
   if(pid > 0)
   {
     close(fd[0]);
     sleep(3);
     write(fd[1],str,strlen(str));
     close(fd[1]);
   } 
   if(pid == 0)
   {
     close(fd[1]);
     ret = read(fd[0],buf,sizeof(buf));
     write(STDOUT_FILENO, buf, ret);
    // sleep(2);
     close(fd[0]);
   }

   return 0;
}
