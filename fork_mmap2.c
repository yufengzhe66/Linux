#include<stdio.h>
#include<stdlib.h>
#include<fcntl.h>
#include<unistd.h>
#include<string.h>
#include<sys/wait.h>
#include<sys/mman.h>

int var = 100;

int main()
{
  int *p;
  pid_t pid;
  
  
  p = (int*)mmap(NULL, 40, PROT_READ|PROT_WRITE, MAP_SHARED|MAP_ANONYMOUS, -1, 0);
  if(p == MAP_FAILED)
   {
     perror("mmap error");
     exit(1);
   }
  
  
  pid = fork();
  if(pid == 0)
   {
     *p = 2000;
     var = 1000;
     printf("child, *p = %d, var = %d\n", *p, var);
   }
   else
   {
     sleep(1);
     printf("parent, *p = %d, var = %d\n", *p, var);
     wait(NULL);
    
     int ret = munmap(p,4);
     if(ret == -1)
       {
         perror("munmap error");
         exit(1);
       }
   }
   
   return 0;
}
