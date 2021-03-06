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
  struct epoll_event tep, ep[OPEN_MAX];                                   //tep:epoll_ctl参数  ep[]:epoll_wait参数
  
  listenfd = socket(AF_INET, SOCK_STREAM, 0);
  int opt = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt ,sizeof(opt));       //端口复用
   bzero(&servaddr, sizeof(servaddr));
   
   servaddr.sin_family = AF_INET;
   servaddr.sin_port = htons(SERV_PORT);
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
  
  bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
  listen(listenfd,20);

  efd = epoll_create(OPEN_MAX);                                         //创建epoll模型，efd指向红黑树根节点
  if(efd == -1)
     {
        perror("epoll_create error");
        exit(1);
     }
  
   tep.events = EPOLLIN;                                                //指定lfd的监听事件为“读”
   tep.data.fd = listenfd;
   res = epoll_ctl(efd, EPOLL_CTL_ADD, listenfd, &tep);                 //将lfd及对应的结构体设置到树上，efd可找到该树
   if(res ==  -1)
    {
      perror("epoll_ctl error");
      exit(1);
    }
 
   while(1)                                                            //epoll为server阻塞监听事件， ep为struct epoll_event类型数组， OPEN_MAX为数组容量， -1表示永久阻塞
    {
      nready = epoll_wait(efd, ep, OPEN_MAX, -1);
      if(nready == -1)
         {
           perror("epoll_wait error");
           exit(1);
         }
      for(i = 0; i < nready; i++ )                                     //如果不是"读"事件，继续循环
        {
          if(!(ep[i].events & EPOLLIN))
           continue;
          
          if(ep[i].data.fd == listenfd)                              // 判断事件的fd是不是lfd
           {
              clilen = sizeof(cliaddr);
              connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);            //接收链接
              
              printf("received from %s at PORT %d\n", inet_ntop(AF_INET, &cliaddr.sin_addr, str, sizeof(str)), ntohs(cliaddr.sin_port));
              printf("cfd %d ---client %d\n", connfd, ++num);
              tep.events = EPOLLIN; tep.data.fd = connfd;
              res = epoll_ctl(efd, EPOLL_CTL_ADD, connfd, &tep);                                         //加入红黑树
              if(res == -1)
                {
                  perror("epoll_ctl error");
                  exit(1);
                }
            }
            else                                                                                         //不是lfd
                {
                  sockfd = ep[i].data.fd;
                  n = read(sockfd, buf, MAXLINE);
                  
                  if(n == 0)                                                                              //读到0，说明客户端关闭链接
                    {
                      res = epoll_ctl(efd, EPOLL_CTL_DEL, sockfd, NULL);                                   //将该文件描述符从红黑树摘除
                      if(res == -1)
                        {
                          perror("epoll_ctl error");
                          exit(1);
                        }  
                      close(sockfd);                                                                        //关闭该客户端的链接
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
 
  close(listenfd);
  close(efd);
  return 0;
  
}



