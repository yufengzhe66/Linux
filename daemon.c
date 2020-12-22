#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>
#include<errno.h>
#include<pthread.h>
#include<sys/stat.h>

void sys_err(const char* str)
{
   perror(str);
   exit(1);
}

int main()
{
  pid_t pid;
  int ret,fd;
  
  pid = fork();
  if(pid > 0)
    exit(0);
 
  pid = setsid();
  if(pid == -1)
    {
      sys_err("setsid error");
    }
  
  umask(0022);

  close(STDIN_FILENO);

  fd = open("/dev/null", O_RDWR);
  if(fd == -1)
    sys_err("open error");
  
  dup2(fd, STDOUT_FILENO);
  dup2(fd, STDERR_FILENO);
  
  while(1);
  return 0;
}
