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

//处理tcp粘包问题

struct packet
{
	int len;
	char buf[1024];
};

//size_t 无符号整数 ssize_t有符合整数
ssize_t readn(int fd, void *buf, size_t nbyte)
{
    size_t nleft = nbyte;//表示剩余的字节数
	size_t nread ;//表示接受的字节数
	char *pBuf = (char*)buf;
	while (nleft > 0)
	{
		if((nread = read(fd,pBuf, nleft))< 0)
		{
             if(nread == EINTR)
			 {
				 return nbyte - nleft;
			 }
			 return -1;
		}
		else if (nread == 0)
		{
			break;
		}
		pBuf += nread;
		nleft -=nread;
		
	}
	return nbyte;
	
}

ssize_t writen(int fd, const void *buf, size_t nbyte)
{
    size_t nleft = nbyte;//表示剩余的字节数
	size_t nwrite ;//表示接受的字节数
	char *pBuf = (char*)buf;
	while (nleft > 0)
	{
		if((nwrite = write(fd,pBuf, nleft))< 0)
		{
             if(nwrite == EINTR)
			 {
				 continue;
			 }
			 return -1;
		}
		else if (nwrite == 0)
		{
			continue;
		}
		pBuf += nwrite;
		nleft -=nwrite;
		
	}
	return nbyte;
	
}

void do_service(int conn)
{
		   struct packet recvbuf;
		   int n;
     	 while(1)
	 	 {
     	     memset(&recvbuf, 0, sizeof(recvbuf));
     	     int ret = readn(conn, &recvbuf.len, 4);
            if (ret == -1)
                 ERR_EXIT("read");
			else if(ret < 4)
			{
				printf("client close\n");
                break;
			 }
			
			n = ntohl(recvbuf.len); 

			ret = read(conn, recvbuf.buf,n);  
			if (ret == -1)
                 ERR_EXIT("read");
			else if(ret < n)
			{
				printf("client close\n");
                break;
			 }   
	 	     fputs(recvbuf.buf,stdout);
	 	     writen(conn, &recvbuf, 4 + n);
	 	 }

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
	  pid_t pid;
	  while(1)
	  {
	 	 if((conn = accept(listen_fd, (struct sockaddr*)&peeraddr, &peerlen) ) < 0)
	 	 {
     	     ERR_EXIT("accept");
	 	 }
	 	 printf("ip = %s, port = %d\n", inet_ntoa(peeraddr.sin_addr),ntohs(peeraddr.sin_port));
		  pid =  fork();
		  if(pid == -1)
		    {
     	     ERR_EXIT("pid");
			}
			if(pid == 0)
			{
			   close(listen_fd);
			   do_service(conn);
			   exit(EXIT_SUCCESS);
			}
             else
			 {
			   close(conn);
			 }

	  }
   return 0;
}
