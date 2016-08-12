#include<stdio.h>
#include<stdlib.h>
#include<poll.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<string.h>
#include<errno.h>
#define _NUM_ 128

int start_up(char* _ip,int _port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0){}//
	
	struct sockaddr_in local;        //本地
	local.sin_family = AF_INET;
	local.sin_port = htons(_port);
	local.sin_addr.s_addr = inet_addr(_ip);

	if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
	{}//

	if(listen(sock,5) < 0)
	{}//
	return sock;
}
int main()
{
	int listen_sock = start_up("192.168.188.128",8080); 

	struct sockaddr_in client;
	socklen_t len = sizeof(client);

	int i;  
	struct pollfd fds[_NUM_];    //初始化pollfd数组
	for(i = 1;i<_NUM_;++i)
	{
		fds[i].fd = -1;
		fds[i].events = 0;
		fds[i].revents = 0;
	}
	fds[0].fd = listen_sock;     //将fds[0].fd 初始化为 listen_sock
	fds[0].events = POLLIN;     //poll 关心他的读事件
	fds[0].revents = 0;          //反馈先不填

	int size = 1;                //数组的有效长度为 1
	int _timeout = 10000;            //毫秒

	while(1)
	{
		switch(poll(fds,size,_timeout))
		{
			case -1:
				perror("poll");
				break;
			case 0:
				printf("timeout\n");
				break;
			default:
 				for(i=0;i<_NUM_;++i)
				{
 					if(fds[i].fd == listen_sock && fds[i].revents == POLLIN)
					{
						printf("链接到新的用户：IP：%s  PORT：%d\n",inet_ntoa(client.sin_addr),\
								ntohs(client.sin_port));
 						int new_sock = accept(listen_sock,(struct sockaddr*)&client,\
								&len);
						if(new_sock < 0){}
						int j;
						for(j=0;j<_NUM_;j++)
						{
							if(fds[j].fd == -1)
							{
								fds[j].fd = new_sock;
								fds[j].events = POLLIN;
								size++;
								break;
							}
						}
						if(j == _NUM_)
						{
							close(new_sock);
						}
					}
					else if(fds[i].fd > 0 && fds[i].revents == POLLIN) //poll 关注的读事件就绪了
					{
						char buf[1024];
						ssize_t _s = read(fds[i].fd,buf,sizeof(buf)-1);
						if(_s > 0)
						{
							buf[_s] = '\0';
							printf("来自客户端的消息：%s\n",buf);
						}
						else if(_s == 0)
						{//客户端关闭
							printf("客户端关闭！\n");
							struct pollfd tmp = fds[i];  //将关闭的文件描述符与最后一个文件描述符
							fds[i] = fds[size-1];        //交换
							fds[size-1] = tmp;
						    
							close(fds[size-1].fd);
							fds[size-1].fd = -1;
							fds[size-1].events = 0;
							fds[size-1].revents = 0;

							--size;   //数组大小减一
						}
						else
						{
							perror("read");
							return;
						}
					}
					else //更多的sock情况 
					{}
				}
				break;
		}
	}

	return 0;
}

