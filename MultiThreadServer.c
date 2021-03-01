#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<ctype.h>
#include<unistd.h>
#include<fcntl.h>

#define MAXLINE 8192
#define SERV_PORT 8000

struct s_info                                   //定义一个结构体，将地址结构和cfd绑定
{
   struct sockaddr_in cliaddr;
   int connfd;
};

void *do_work(void* arg)
{
  int n,i;
  struct s_info *ts = (struct s_info*)arg;
  char buf[MAXLINE];
  char str[INET_ADDRSTRLEN];                   //#define INET_ADDRSTRLEN 16
  
  while(1)
  {
    n = read(ts -> connfd, buf, MAXLINE);
    if(n == 0)
      {
        printf("the client %d closed...\n", ts -> connfd);
        break;
      }
    printf("received from %s at PORT %d\n",inet_ntop(AF_INET,&(*ts).cliaddr.sin_addr, str, sizeof(str)), ntohs((*ts).cliaddr.sin_port));
    
    for(i = 0; i < n; i++)
      {
        buf[i] = toupper(buf[i]);
      }
  
    write(STDOUT_FILENO, buf, n);
    write(ts -> connfd, buf, n);
  }
    close(ts -> connfd);
    return (void *)0;
}

int main()
{
   struct sockaddr_in servaddr, cliaddr;
   socklen_t cliaddr_len;
   int listenfd, connfd;
   pthread_t tid;

   struct s_info ts[256];
   int i = 0;
   
   listenfd = socket(AF_INET, SOCK_STREAM, 0);          //创建结构体数组
    
   bzero(&servaddr, sizeof(servaddr));                  //地址结构清零
   servaddr.sin_family = AF_INET;                       //制定本地任意IP
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);        //指定端口号
   servaddr.sin_port = htons(SERV_PORT);

   bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));      //绑定
   listen(listenfd, 128);                                               //设置同一时刻链接服务器上限数
  
   printf("Accepting client connect ...\n");
   
   while(1)
    {
      cliaddr_len = sizeof(cliaddr);
      connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddr_len);   //阻塞监听客户端链接请求
      ts[i].cliaddr = cliaddr;
      ts[i].connfd = connfd;

      pthread_create(&tid, NULL, do_work, (void*)&ts[i]);
      pthread_detach(tid);
      i++;
    }
   
   return 0;
}
