#include<stdio.h>
#include<stdlib.h>

#include<unistd.h>
#include<errno.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<sys/socket.h>
#include<string.h>


struct packet
{
	int len;
	char buf[1024];
};

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

		struct packet sendbuf;
		struct packet recvbuf;
		memset(&sendbuf, 0, sizeof(sendbuf));  
		memset(&recvbuf, 0, sizeof(recvbuf));  
         int n;//先定义一个包的长度
		  while(fgets(sendbuf.buf, sizeof(sendbuf.buf),stdin) != NULL)
		  {
			n = sizeof(sendbuf.buf);
			sendbuf.len = htonl(n);//必须统一成网络字节序
            writen(sock, &sendbuf, 4 + n);//包长应该包含四个字节的头部


			int ret = readn(sock, &recvbuf.len, 4);
            if (ret == -1)
                 ERR_EXIT("read");
			else if(ret < 4)
			{
				printf("client close\n");
                break;
			 }
			
			n = ntohl(recvbuf.len); 

			ret = read(sock, recvbuf.buf,n);  
			if (ret == -1)
                 ERR_EXIT("read");
			else if(ret < n)
			{
				printf("client close\n");
                break;
			 }  

			fputs(recvbuf.buf, stdout);

			memset(&recvbuf, 0 , sizeof(recvbuf));
			memset(&sendbuf, 0 , sizeof(sendbuf));
		  }
		  close(sock);
   return 0;
}
