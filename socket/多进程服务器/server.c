#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<sys/types.h>         
#include<sys/socket.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<string.h>
#include<signal.h>

void colect_child(int sig)         //清理子进程的函数
{
	while(waitpid(-1,0,WNOHANG)>0)
	{
		printf("colect child done ...\n");
	}
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

	signal(SIGCHLD,colect_child);    //注册一个处理自己程清理函数
	while(1)
	{	
		int new_fd = accept(listen_sock,(struct sockaddr*)&peer,&len);
		pid_t id = fork();
		if(id > 0)
		{//father
			close(new_fd);    //父进程关闭文件描述符，去等待新的用户端信息
		}
		else if(id == 0)
		{//child
			printf("子进程建立结束：pid ：%d，得到一个新的用户端信息->文件描述符：%d， pid : %s, 端口号：%d\n",\
					getpid(),new_fd,inet_ntoa(peer.sin_addr),ntohs(peer.sin_port));
			char buf[1024];
			while(1)
			{
    			ssize_t _s = read(new_fd,buf,sizeof(buf)-1);   //从新进程读消息
				if(_s > 0)
				{
 					buf[_s] = '\0';
					printf("子进程消息：%s\n",buf);
					write(new_fd,buf,strlen(buf));      //向新进程写消息
				}
				else if(_s == 0)
				{
 					printf("子进程退出！\n");
					break;
				}
				else
				{
					break;
				}
			}
			close(new_fd);  //关闭文件描述符
			exit(0);       //子进程退出
		}
	
	}
	return 0;
}
