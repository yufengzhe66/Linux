#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<fcntl.h>
#include<errno.h>
#include<pthread.h>

//FIFO实现非血缘关系进程间通信 读端

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
   int len;   

   if(argc < 2)
   {
     printf("Enter like this: ./a.out fifoname\n");
     exit(-1);
   } 
   fd = open(argv[1],O_RDONLY);
   if(fd < 0)
   {
     sys_err("open");
   }
   
   i = 0;
   while(1)
   {
    len = read(fd, buf, sizeof(buf));
    write(STDOUT_FILENO, buf, len);
    sleep(2);
   }
   close(fd);
   return 0;
}
