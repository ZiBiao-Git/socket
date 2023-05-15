#include<stdio.h>
#include<stdlib.h>

#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<string.h>
#define  ERR_EXIT(m) \
         do  \
         {\
           perror(m);\
		       exit(EXIT_FAILURE);\
			  }while(0)

int main(void)
{
   int sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(sock <0)
	   {
         ERR_EXIT("socket");
	   }

	   struct sockaddr_in seraddr;
	   memset(&seraddr, 0, sizeof(seraddr));
	   seraddr.sin_family = AF_INET;
	   seraddr.sin_port = htons(5188);
	   seraddr.sin_addr.s_addr = inet_addr("127.0.0.1");
	   
	   if(connect(sock,(struct sockaddr*)&seraddr, sizeof(seraddr)) <0)
	      ERR_EXIT("connect");
      char sendbuf[1024] = {0};
      char recvbuf[1024] = {0};
		  while(fgets(sendbuf, sizeof(sendbuf),stdin) != NULL)
		  {
            write(sock, sendbuf, strlen(sendbuf));
			read(sock, recvbuf, sizeof(recvbuf));
			fputs(recvbuf, stdout);
			memset(&recvbuf, 0 , sizeof(recvbuf));
			memset(&sendbuf, 0 , sizeof(sendbuf));
		  }
		  close(sock);
   return 0;
}
