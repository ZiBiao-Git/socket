## getsockname()
用于获取本地协议地址
int getsockname(int socket, struct sockaddr *restrict address, socklen_t *restrict address_len); 

    参数：

        参数1：套接字描述符
        参数2：用来保存地址信息的套接字结构体
        参数3：相对于参数2的结构体地址大小指针

    返回值：

        成功：0
        失败：-1
## getsockname应用场景

    在一个没有调用bind的TCP客户上，connect成功返回后，getsockname用于返回由内 核赋予该连接的本地IP地址和本地端口号。
    在以端口号0调用bind（告知内核去选择本地端口号）后，getsockname用于返回由内 核赋予的本地端口号


```c
#include<sys/socket.h>
int sockfd_to_family(int sockfd)
{
    struct sockaddr_storage ss;
    socklen_t len;
 
    len = sizeof(ss);
    if (getsockname(sockfd, (SA *) &ss, &len) < 0)
        return(-1);
    return(ss.ss_family);
} 
```
   在一个以通配IP地址调用bind的TCP服务器上，与某个客户的连接一旦建立 （accept成功返回），getsockname就可以用于返回由内核赋予该连接的本地IP地址。在这样的调用中，套接字描述符参数必须是已连接套接字的描述符，而不是监听套接字的 描述符


## getperrname()
**getpeername：用于获取外地协议地址**
int getperrname(int sockfd,struct sockaddr *perraddr,socklen_t *addrlen);


当一个服务器是由调用过accept的某个进程通过调用exec执行程序时，它能够获取客 户身份的唯一途径便是调用getpeername。inetd（13.5节）fork并exec某个TCP服务 器程序时就是如此情形，如修所示。inetd调用accept（左上方方框）返回两个值： 已连接套接字描述符connfd，这是函数的返回值；客户的IP地址及端口号，如图中标有 “对端地址”的小方框所示（代表一个网际网套接字地址结构）。inetd随后调用fork， 派生出inetd的一个子进程。既然子进程起始于父进程的内存映像的一个副本，父进程 中的那个套接字地址结构在子进程中也可用，那个已连接套接字描述符也是如此（因为 描述符在父子进程之间是共享的）。然而当子进程调用exec执行真正的服务器程序（譬 如说Telnet服务器程序）时，子进程的内存映像被替换成新的Telnet服务器的程序文件（也 就是说包含对端地址的那个套接字地址结构就此丢失），不过那个已连接套接字描述符 跨exec继续保持开放。Telnet服务器首先调用的函数之一便是getpeername，用于获取 客户的IP地址和端口号
https://img-blog.csdnimg.cn/20191103161011824.png?x-oss-process=image/watermark,type_ZmFuZ3poZW5naGVpdGk,shadow_10,text_aHR0cHM6Ly9ibG9nLmNzZG4ubmV0L3FxXzQxNDUzMjg1,size_16,color_FFFFFF,t_70


```c
int sockfd=socket(AF_INET,SOCK_STREAM,0);
struct sockaddr_storage ss;
socklen_t len;
len=sizeof(ss);
 
if(getsockname(sockfd,(struct sockaddr*)&ss,&len)<0)
    return -1;

```

既然不知道要分配的套接字地址结构的类型，我们于是采用sockaddr_storage这个通 用结构，因为它能够承载系统支持的任何套接字地址结构

## gethostname()
int gethostname(char *name, size_t namelen);
函数的作用是：返回当前机器的标准主机名。namelen参数应指定name参数所指向的数组的大小。返回的名称应以null结尾，但如果namelen的长度不足以容纳主机名，则返回的名称应被截断，并且未指定返回的名称是否以null结尾。
返回值：
成功时返回0；否则返回-1。

## gethostbyname()
struct hostent *gethostbyname(const char *name);
gethostbyname()函数主要作用：用域名或者主机名获取地址，操作系统提供的库函数。

这个函数的传入值是域名或者主机名，例如"www.google.cn"等等。传出值，是一个hostent的结构。如果函数调用失败，将返回NULL。

    返回hostent结构体类型指针
```c
    struct hostent
    {
        char    *h_name;               
        char    **h_aliases;
        int     h_addrtype;
        int     h_length;
        char    **h_addr_list;
        #define h_addr h_addr_list[0]
    };
```
    hostent->h_name
    表示的是主机的规范名。例如www.google.com的规范名其实是www.l.google.com。
    
    hostent->h_aliases
    表示的是主机的别名.www.google.com就是google他自己的别名。有的时候，有的主机可能有好几个别名，这些，其实都是为了易于用户记忆而为自己的网站多取的名字。

    hostent->h_addrtype    
    表示的是主机ip地址的类型，到底是ipv4(AF_INET)，还是pv6(AF_INET6)

    hostent->h_length      
    表示的是主机ip地址的长度

    hostent->h_addr_lisst
    表示的是主机的ip地址，注意，这个是以网络字节序存储的。千万不要直接用printf带%s参数来打这个东西，会有问题的哇。所以到真正需要打印出这个IP的话，需要调用inet_ntop()。
    onst char *inet_ntop(int af, const void *src, char *dst, socklen_t cnt) ：
    这个函数，是将类型为af的网络地址结构src，转换成主机序的字符串形式，存放在长度为cnt的字符串中。返回指向dst的一个指针。如果函数调用错误，返回值是NULL。
```c
#include  < netdb.h >
#include  <sys/socket.h >
#include  < stdio.h >
 
int  main( int  argc,  char   ** argv)
{
    char     * ptr,  ** pptr;
    struct  hostent  * hptr;
    char    str[ 32 ];
    ptr  =  argv[ 1 ];

    if ((hptr  =  gethostbyname(ptr))  ==  NULL)
    {
        printf( "  gethostbyname error for host:%s\n " , ptr);
        return   0 ;
    }

    printf( " official hostname:%s\n " ,hptr -> h_name);
    for (pptr  =  hptr -> h_aliases;  * pptr  !=  NULL; pptr ++ )
        printf( "  alias:%s\n " , * pptr);

    switch (hptr -> h_addrtype)
    {
        case  AF_INET:
        case  AF_INET6:
            pptr = hptr -> h_addr_list;
            for (;  * pptr != NULL; pptr ++ )
                    printf( "  address:%s\n " ,
                        inet_ntop(hptr -> h_addrtype,  * pptr, str,  sizeof (str)));
            printf( "  first address: %s\n " ,
                        inet_ntop(hptr -> h_addrtype, hptr -> h_addr, str,  sizeof (str)));
        break ;
        default :
            printf( " unknown address type\n " );
        break ;
    }

    return   0 ;
}

# gcc test.c
# ./a.out www.baidu.com
official hostname:www.a.shifen.com
alias:www.baidu.com
address:121.14.88.11
address:121.14.89.11
first address: 121.14.88.11
```    