#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<errno.h>
#include<pthread.h>

//FIFO实现非血缘关系进程间通信 写端

void sys_err(const char *str)
{
   perror(str);
   exit(1);
}

int main(int argc, char* argv[])
{
   int fd;  
   int i;
   char buf[4096];    
   
   if(argc < 2)
   {
     printf("Enter like this: ./a.out fifoname\n");
     exit(-1);
   } 
   fd = open(argv[1],O_WRONLY);
   if(fd < 0)
   {
     sys_err("open");
   }
   
   i = 0;
   while(1)
   {
     sprintf(buf,"hello itcast %d\n",i++);
     write(fd, buf, strlen(buf));
     sleep(1);
   }
   close(fd);
   return 0;
}
