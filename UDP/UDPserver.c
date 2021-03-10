#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<ctype.h>



#define SERV_PORT 8000

int main()
{
    struct sockaddr_in serv_addr, clie_addr;
    socklen_t clie_addr_len;
    
    int sockfd;
    char buf[BUFSIZ];
    char str[INET_ADDRSTRLEN];
    int i, n;

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    
    bzero(&serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr));

    printf("Accepting connection ...\n");
    while(1)
    {
       clie_addr_len = sizeof(clie_addr);
       n = recvfrom(sockfd, buf, BUFSIZ, 0, (struct sockaddr *)&clie_addr, &clie_addr_len);
       if(n == -1)
          perror("recvfrom error");
      
       printf("Receiving from %s at PORT %d\n",inet_ntop(AF_INET, &clie_addr.sin_addr, str, sizeof(str)), ntohs(clie_addr.sin_port));

       for(i = 0; i < n; i++)
          buf[i] = toupper(buf[i]);

       n = sendto(sockfd, buf, n, 0, (struct sockaddr *)&clie_addr, sizeof(clie_addr));
       if(n == -1)
          perror("sendto error"); 
    }

    close(sockfd);

return 0;
}
