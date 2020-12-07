#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/mman.h>
#include<fcntl.h>
#include<unistd.h>
#include<error.h>
#include<pthread.h>

struct student
{
  int id;
  char name[256];
  int age;
};

int main()
{
  int fd;
  fd = open("test_mmap", O_RDWR|O_CREAT|O_TRUNC, 0644);
  if(fd == -1)
   {
     perror("open error");
     exit(1);
   }
  
  struct student stu = {1, "xiaoming", 18};
  struct student *p;
  
  ftruncate(fd, sizeof(stu));
  p = (struct student*)mmap(NULL, sizeof(stu), PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);
  if(p == MAP_FAILED)
     {
       perror("mmap error");
       exit(1);
     }
  close(fd);

  while(1)
  {
    memcpy(p, &stu, sizeof(stu));
    stu.id++ ;
    sleep(1);
  }
  
  munmap(p, sizeof(stu));

  return 0;
}
