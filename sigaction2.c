#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>
#include<pthread.h>



void sys_err(const char *str)
{
   perror(str);
   exit(1);
}


void sig_catch(int signo)
{
   if(signo == SIGINT)
      {
         printf("catch you!! %d\n", signo);
         sleep(10);
      }
//   if(signo == SIGQUIT)
//      printf("------catch you!! %d\n", signo);
   return;
}



int main()
{
   struct sigaction act, oldact;
   
   act.sa_handler = sig_catch;                               // set callback function name
   sigemptyset(&(act.sa_mask));                              // set mask when sig_catch working
   act.sa_flags = 0;                                         // usually use
   sigaddset(&act.sa_mask, SIGQUIT);  

   int ret = sigaction(SIGINT, &act, &oldact); 
//   ret = sigaction(SIGQUIT, &act, &oldact);              
   if(ret == -1)
     {
        sys_err("sigaction error"); 
     }

   while(1);
   return 0;
}
