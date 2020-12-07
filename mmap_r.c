#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<sys/mman.h>
#include<fcntl.h>
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
  fd = open("test_mmap", O_RDONLY);
  if(fd == -1)
   {
     perror("open error");
   }
  
  struct student stu;
  struct student *p;
  p = (struct student*)mmap(NULL, sizeof(stu), PROT_READ, MAP_SHARED, fd, 0);
  if(p == MAP_FAILED)
     {
       perror("mmap error");
     }
  close(fd);

  while(1)
  {
    printf("id = %d, name = %s, age = %d\n", p->id, p->name, p->age);
    sleep(1);
  }
  
  munmap(p, sizeof(stu));

  return 0;
}
