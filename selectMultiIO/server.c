#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<ctype.h>
#include<sys/socket.h>
#include<sys/wait.h>


#define SERV_PORT 6666

int main()
{
  int listenfd, connfd;
  char buf[BUFSIZ];
  
  struct sockaddr_in clie_addr, serv_addr;
  socklen_t clie_addr_len;

  listenfd = socket(AF_INET, SOCK_STREAM, 0);

  int opt = 1;
  setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
  bzero(&serv_addr, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
  serv_addr.sin_port = htons(SERV_PORT);

  bind(listenfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
  
  listen(listenfd, 128);
  
  fd_set rset, allset;                                                            //定义读集合和备份集合
  int ret, maxfd = 0, n, i, j;               
  maxfd = listenfd;                                                               //最大文件描述符

  FD_ZERO(&allset);                                                               //清空监听集合
  FD_SET(listenfd, &allset);                                                      //将待监听fd添加到监听集合中
  
  while(1)
  {
   rset = allset;
   ret = select(maxfd + 1, &rset, NULL, NULL, NULL);                              //使用select 监听
   if(ret < 0)
     {
       perror("select error");
       exit(1);
     }
   
   if(FD_ISSET(listenfd, &rset))                                                 //listenfd 满足监听的 读事件
     { 
       clie_addr_len = sizeof(clie_addr);
       connfd = accept(listenfd, (struct sockaddr *)&clie_addr, &clie_addr_len); //建立链接，  不会阻塞
   
       FD_SET(connfd, &allset);                                                  //将新产生的fd, 添加到监听集合中，监听数据读事件
       
       if(maxfd < connfd)                                                        //修改maxfd
          maxfd = connfd;
       if(ret == 1)                                                              //select只返回一个，并且是listenfd, 后续执行无须执行
          continue;  
     }

   for(i = listenfd + 1; i <= maxfd; i++)                                        //处理满足读事件的那个fd
    {
       if(FD_ISSET(i, &rset))
        {
          n = read(i, buf, sizeof(buf));                                           //读取数据
          if(n == 0)                                                              //检测到客户端已经关闭链接
           {
             close(i);
             FD_CLR(i, &allset);                                                 //移除监听套接字集合  
           }
          else if(n == -1)
           {
             perror("read error");
             exit(1);
           }
        
      for(j = 0; j < n; j++)
         buf[j] = toupper(buf[j]);
   
      write(i, buf, n);
      write(STDOUT_FILENO, buf, n);
    }
  }
 }

  close(listenfd);
  return 0;
}






























 
