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

void handler(int sig)
{
   printf("recv a sig = %d\n",sig);
   exit(EXIT_SUCCESS);
}
int main(void)
{
   int listen_fd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if(listen_fd <0)
	   {
         ERR_EXIT("socket");
	   }

	   struct sockaddr_in seraddr;
	   memset(&seraddr, 0, sizeof(seraddr));
	   seraddr.sin_family = AF_INET;
	   seraddr.sin_port = htons(5188);
	   seraddr.sin_addr.s_addr = htonl(INADDR_ANY);
	   
	  //下面是设置sock选项 使进程处于time_wait状态也可以重启
	  int on = 1;
      if(setsockopt(listen_fd, SOL_SOCKET,SO_REUSEADDR, &on , sizeof(on)) < 0)
         ERR_EXIT("setsockopt");

	  if( bind(listen_fd, (struct sockaddr*)&seraddr, sizeof(seraddr))< 0)
	  {
      ERR_EXIT("bind");
	  }

      if(listen(listen_fd, SOMAXCONN ) < 0)   
	  {
           ERR_EXIT("listen");
	  }
      
	  struct sockaddr_in peeraddr;
      socklen_t peerlen = sizeof(peeraddr);//accept 的第三个参数必须初始化
	  int conn;
	  if((conn = accept(listen_fd, (struct sockaddr*)&peeraddr, &peerlen) ) < 0)
	  {
          ERR_EXIT("accept");
	  }
	  printf("ip = %s, port = %d\n", inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));
       char recvbuf[1024];

	   pid_t pid = fork();
	   if(pid < 0)
	      ERR_EXIT("fork");
	   if(pid == 0)
	   {
	        //如果没有这个信号监听。父进程退出 子进程并没有退出；父进程退出 发出kill 的指令；
	        signal(SIGUSR1,handler);
             char sendbuf[1024] = {0};
          while(fgets(sendbuf,sizeof(sendbuf),stdin) != NULL)
		  {
             write(conn,sendbuf,strlen(sendbuf));
			 memset(sendbuf,0,sizeof(sendbuf));
		  }
		  printf("child close\n");
		  exit(EXIT_SUCCESS);
	   }
	   else
	   {
         char recvbuf[1024];
     	 while(1)
	 	 {
     	     memset(recvbuf, 0, sizeof(recvbuf));
     	     int ret = read(conn, recvbuf, sizeof(recvbuf));
			 if(ret == -1)
			     ERR_EXIT("read");
			if(ret == 0)
			{
			     printf("peer close\n");
				 break;
			}
	 	     fputs(recvbuf,stdout);
	 	 }
		  printf("parent close\n");
		  kill(pid,SIGUSR1);//这里的pid是子进程的id 给子进程发送消息
		 exit(EXIT_SUCCESS);
	   }
   return 0;
}
