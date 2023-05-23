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

//模拟实现和MSG_peek类型的recv
ssize_t recv_peek(int socket, void *buffer, size_t length)
{
    while (1)
	{
		int ret = recv(socket,buffer, length, MSG_PEEK);//recv函数不会将数据从缓冲区移除
		if(ret == -1 && errno == EINTR)
		{
			continue;
		}
		return ret;
	}
	
}

//readline 只能用于套接字(readline 遇到/n 就结束了)
ssize_t readline(int socket, void *buffer, size_t max_line)
{
    int ret;
	int nread;
	char *bufp = buffer;
	int nleft = max_line;
	while (1)
	{
		ret = recv_peek(socket,bufp,nleft);//并没有移走缓冲区的数据
		if(ret < 0)//失败
		   return ret;
        else if(ret == 0)//对方关闭了套接口
		    return ret;

		nread = ret;
		int i;
		for (size_t i = 0; i < nread; i++)
		{
			if(bufp[i] == '\n')
			{
				ret = readn(socket, bufp, i+1);
				if(ret != i+1)
				   exit(EXIT_FAILURE);
				return ret;   
			}
		}

		if(nread > nleft)
		{
			exit(EXIT_FAILURE);
		}	

		nleft -= nread;
		ret = readn(socket,bufp,nread);

		if(ret != nread)
		{
			exit(EXIT_FAILURE);
		}
		bufp += nread;
	}
	return -1;
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
        
       struct sockaddr_in localaddr;
        socklen_t  addrlen = sizeof(localaddr);
        if(getsockname(sock, (struct sockaddr *)& localaddr, &addrlen) < 0)
		{
              ERR_EXIT("getsockname error");
		}
        printf("ip = %s, port = %d\n", inet_ntoa(localaddr.sin_addr),ntohs(localaddr.sin_port));

		char sendbuf[1024] = {0};
		char recvbuf[1024] = {0};
		  while(fgets(sendbuf, sizeof(sendbuf),stdin) != NULL)
		  {
	
            writen(sock, &sendbuf, strlen(sendbuf));

			int ret = readline(sock, recvbuf, sizeof(recvbuf));
            if (ret == -1)
                 ERR_EXIT("readline error");
			else if(ret == 0)
			{
				printf("client close\n");
                break;
			 }
			

			fputs(recvbuf, stdout);

			memset(recvbuf, 0 , sizeof(recvbuf));
			memset(sendbuf, 0 , sizeof(sendbuf));
		  }
		  close(sock);
   return 0;
}
