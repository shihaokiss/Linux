#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/types.h>         
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<pthread.h>

void* handler_data(void* arg)
{
	int sock = (int)arg;
	printf("创建了一个新线程，新客端的文件描述符为：%d\n",sock);
	char buf[1024];
	while(1)
	{
		ssize_t _s = read(sock,buf,sizeof(buf)-1);
		if(_s > 0)
		{
 				buf[_s] = '\0';
				printf("客户端的信息为：%s\n",buf);
				write(sock,buf,strlen(buf));
		}
		else if(_s == 0)
		{
 				printf("客户端退出！\n");
				break;
		}
		else
		{
			break;
		}
	}
	close(sock);
	pthread_exit((void*)0);
}

int main()
{
	int listen_sock = socket(AF_INET,SOCK_STREAM,0); //创建套接字
	if(listen_sock < 0)
	{
		printf("sock\n");
		return 1;
	}

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

		if(new_fd > 0)
		{
			pthread_t id;
			pthread_create(&id,NULL,handler_data,(void*)new_fd);  //创建线程
			pthread_detach(id);       //线程分离
		}
		else if(new_fd == 0)
		{
			perror("accept\n");
			return 4;
		}
	}
	return 0;
}
