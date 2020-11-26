#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<pthread.h>


int main()
{
   int i;
   pid_t pid;
   
   for(i = 0;i < 5; i++)
    {
      if(fork()==0)
       break;
    }
    if(i==5)
    {sleep(5);
     printf("I'm paternt\n");}
    else
    {
    sleep(i);
    printf("I'm %dth child\n",i+1);
    }
   return 0;
}
