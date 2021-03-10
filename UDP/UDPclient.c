#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<ctype.h>



#define SERV_PORT 8000

int main()
{
    struct sockaddr_in serv_addr;
    
    int sockfd;
    char buf[BUFSIZ];
  
    int ret;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);


    while(1)
    {
       fgets(buf, sizeof(buf), stdin);
     
       sendto(sockfd, buf, strlen(buf), 0, (struct sockaddr *)&serv_addr, sizeof(serv_addr));
       ret = recvfrom(sockfd, buf, sizeof(buf), 0, NULL, 0);
       write(STDOUT_FILENO, buf, ret); 
    }

    close(sockfd);

return 0;
}
