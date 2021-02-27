#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<errno.h>
#include<signal.h>
#include<pthread.h>
#include<ctype.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<signal.h>
#include<sys/wait.h>

#define SRV_PORT 9999
#define BUFSIZE 1024

void catch_child(int signum)
{
   while(waitpid(0, NULL, WNOHANG) > 0);
   return ;      
}


int main()
{
   int lfd, cfd;
   pid_t pid;
  
   struct sockaddr_in srv_addr, clt_addr;
   socklen_t clt_addr_len;
   char buf[BUFSIZE];
   
   int ret, i;

   // memset(&srv_addr, 0, sizeof(srv_addr));        //地址结构清零
   bzero(&srv_addr, sizeof(srv_addr));
   
   srv_addr.sin_family = AF_INET;
   srv_addr.sin_port = htons(SRV_PORT);
   srv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    
   lfd = socket(AF_INET, SOCK_STREAM, 0);
   
   bind(lfd, (struct sockaddr *)&srv_addr, sizeof(srv_addr));

   listen(lfd, 128);

   clt_addr_len = sizeof(clt_addr);

   while(1)
   {
     cfd = accept(lfd, (struct sockaddr *)&clt_addr, &clt_addr_len);
    
     pid = fork();
     if(pid <  0)
     {
        perror("fork_error");
        exit(1);
     }
     else if(pid == 0)
     {
        close(lfd);
        break;    
     }
     else
     {
        struct sigaction act;
        act.sa_handler = catch_child;
        sigemptyset(&act.sa_mask);
        act.sa_flags = 0;
        
        ret = sigaction(SIGCHLD,&act, NULL);
        if(ret != 0)
         {
           perror("sigaction error");
           exit(1);
         }
        close(cfd);
        continue;
     }
   }
  
   if(pid == 0)
     {
      while(1)
       {
         ret = read(cfd,buf,sizeof(buf));
         if(ret == 0)
           {
             close(cfd);
             exit(1);
           }
          for(i = 0; i < ret; i++)
             buf[i]= toupper(buf[i]);
          
          write(cfd, buf, ret);
          write(STDOUT_FILENO, buf, ret);
       }
     }
   return 0;
}


