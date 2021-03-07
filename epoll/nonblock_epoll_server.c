#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<signal.h>
#include<netinet/in.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<sys/epoll.h>
#include<unistd.h>
#include<fcntl.h>

#define MAXLINE 10
#define SERV_PORT 9000

int main()
{
   struct sockaddr_in servaddr, cliaddr;
   socklen_t cliaddr_len;
   int listenfd, connfd;
   char buf[MAXLINE];
   char str[INET_ADDRSTRLEN];
   int efd;

   listenfd = socket(AF_INET, SOCK_STREAM, 0);

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port = htons(SERV_PORT);
  
   bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

   listen(listenfd, 20);

   struct epoll_event event;
   struct epoll_event resevent[10];
   int res, len;

   efd = epoll_create(10);

   event.events = EPOLLIN | EPOLLET;  
   //event.events = EPOLLIN;

   printf("Accepting connection...\n");

   cliaddr_len = sizeof(cliaddr);
   connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);
   printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));

   int flag;                                                                    //修改connfd为非阻塞读
   flag = fcntl(connfd,F_GETFL);
   flag |= O_NONBLOCK;
   fcntl(connfd,flag);
  

   event.data.fd = connfd;
   epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &event);

   while(1)
     {
       printf("epoll_wait begin\n");
       res = epoll_wait(efd, resevent, 10, -1);
       printf("epoll_wati end res %d\n",res);
       if(resevent[0].data.fd == connfd)
         {
           while((len = read(connfd, buf, MAXLINE/2)) >  0)                //非阻塞读，轮询
              write(STDOUT_FILENO, buf, len);
         }     
     } 

   return 0;
}
