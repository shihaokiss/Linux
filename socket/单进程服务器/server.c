#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/types.h>         
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>

int main()
{
	int listen_sock = socket(AF_INET,SOCK_STREAM,0); //创建套接字
	int bDontLinger = 0;
	if(listen_sock < 0)
	{
		printf("sock\n");
		return 1;
	}
	// 如果要已经处于连接状态的soket在调用closesocket后强制关闭，不经历
	// TIME_WAIT的过程：SO_DONTLINGER
	setsockopt(listen_sock,SOL_SOCKET,1,\
			(const char*)&bDontLinger,sizeof(int));
	struct sockaddr_in local;    //初始化 sockaddr_in 结构体
	local.sin_family = AF_INET;
	local.sin_port = htons(8080);
	local.sin_addr.s_addr = htonl(INADDR_ANY);

	if(bind(listen_sock,(struct sockaddr *)&local,sizeof(local)) < 0)
	{							 //将套解字与指定的IP地址和端口等信息绑定
		printf("sock\n");
		return 2;
	}

	if(listen(listen_sock,5) < 0) //使该进程可以接受其他进程的请求
	{
		printf("listen\n");
		return 3;
	}

	struct sockaddr_in peer;     //远端的sockaddr_in结构体
	socklen_t len = sizeof(peer);   //远端的sockaddr_in结构体的大小
	while(1)
	{
		int new_fd = accept(listen_sock,(struct sockaddr*)&peer,&len);
		//accept函数将从连接请求队列中获得连接信息，创建新的套接字，并
		//返回该套接字的文件描述符。新创建的套接字用于服务器与客户机的
		//通信，而原来的套接字仍然处于监听状态。
	setsockopt(new_fd,SOL_SOCKET,1,\
			(const char*)&bDontLinger,sizeof(int));
		if(new_fd > 0)      
		{
			char buf[1024];
			while(1)
			{
				ssize_t _s = read(new_fd,buf,sizeof(buf)-1);
				if(_s > 0)
				{
					buf[_s] = '\0';
					printf("用户端的消息：%s\n",buf);
					write(new_fd,buf,strlen(buf));
				}
				else if(_s == 0)
				{
					printf("用户端退出！\n");
					break;
				}
				else
				{
					break;
				}
			}
		}
		else
		{
			break;
		}
	}
	return 0;
}
