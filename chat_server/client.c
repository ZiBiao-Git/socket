#include<stdio.h>
#include<stdlib.h>

#include<signal.h>
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

void  handler(int sig)
{
  printf("recv a signal = %d\n", sig);
  exit(EXIT_SUCCESS) ;
}
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

		pid_t pid = fork();
		if(pid <0)
		  ERR_EXIT("fork");
		if(pid == 0)
		{
         	 char recvbuf[1024] = {0};
           while(1)
		   {
		 	   memset(&recvbuf, 0 , sizeof(recvbuf));
			   int ret = read(sock, recvbuf,sizeof(recvbuf));
			   if(ret == -1)
			     ERR_EXIT("recv");
			   else if(ret == 0)
			   {
			     printf("peer close\n");
				 break;
			   }
		 	   fputs(recvbuf, stdout);
				 
		   }
		   close(sock);
		   kill(getppid(),SIGUSR1);
		}
		else
		{
         	 char sendbuf[1024] = {0};
			 signal(SIGUSR1,handler);
		 	 while(fgets(sendbuf, sizeof(sendbuf),stdin) != NULL)
		 	 {
         	   write(sock, sendbuf, strlen(sendbuf));
		 	   memset(&sendbuf, 0 , sizeof(sendbuf));
		 	 }
		  close(sock);
		}
   return 0;
}
