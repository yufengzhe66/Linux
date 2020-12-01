#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<fcntl.h>
#include<string.h>

int main()
{
   char buf[1024];
   char *str = "------------test2 write sucess---------\n";
   int ret;

   sleep(1);
   
   int fd = open("test.txt", O_RDWR);

   ret = read(fd, buf, sizeof(buf));

   write(STDOUT_FILENO, buf, ret);
   
   write(fd, str, strlen(str));

   printf("test2 read/write finish\n");
   close(fd);
   return 0;
}
