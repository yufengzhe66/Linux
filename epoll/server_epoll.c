#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<arpa/inet.h>
#include<sys/epoll.h>
#include<errno.h>
#include<ctype.h>

#define MAXLINE 8192
#define SERV_PORT 8000

#define OPEN_MAX 5000

int main()
{
  int i, listenfd, connfd, sockfd;
  int n, num = 0;
  ssize_t nready, efd, res;
  char buf[MAXLINE], str[INET_ADDRSTRLEN];
  socklen_t clilen;

  struct sockaddr_in cliaddr, servaddr;
  struct epoll_event tep, ep[OPEN_MAX];
  
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  

  efd = epoll_create(OPEN_MAX);
  if(efd == -1)
     {
        perror("epoll_create error");
        exit(1);
     }
  
   tep.events = EPOLLIN;
   tep.data.fd = listenfd;
   res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);
   if(res ==  -1)
    {
      perror("epoll_ctl error");
      exit(1);
    }
 
   while(1)
    {
      nready = epoll_wait(efd, ep, OPEN_MAX, -1);
      if(nready == -1)
         {
           perror("epoll_wait error");
           exit(1);
         }
      for(i = 0; i < nready; i++ )
        {
          if(!(ep[i].events & EPOLLIN))
           continue;
          
          if(ep[i].data.fd == listenfd)
           {
              clilen = sizeof(cliaddr);
              connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
              
              printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));
              printf("cfd %d ---client %d\n", connfd, ++num);
              tep.events = EPOLLIN; tep.data.fd = connfd;
              res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);
              if(res == -1)
                {
                  perror("epoll_ctl error");
                  exit(1);
                }
              else
                {
                  sockfd = ep[i].data.fd;
                  n = read(sockfd, buf, MAXLINE);
                  
                  if(n == 0)
                    {
                      res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                      if(res == -1)
                        {
                          perror("epoll_ctl error");
                          exit(1);
                        }  
                      close(sockfd);
                      printf("client[%d] close connection\n", sockfd);
                    }
                   else if(n < 0)
                    {
                      perror("read n < 0 error:");
                      res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);
                      close(sockfd);
                    }
                   else
                    {
                      for(i = 0; i < n; i++)
                         buf[i] = toupper(buf[i]);
                      write(STDOUT_FILENO, buf, n);
                      write(sockfd, buf, n);
                    }
                }                     
           }
        }
    }
  close(listenfd);
  close(efd);
  return 0;
  
}



























