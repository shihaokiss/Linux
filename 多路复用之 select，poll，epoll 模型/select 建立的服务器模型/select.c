#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>
int start_up(char* _ip,int _port)
{
	int sock = socket(AF_INET,SOCK_STREAM,0);
	if(sock < 0){} //
	
	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(_port);
	local.sin_addr.s_addr = inet_addr(_ip);
	
	if(bind(sock,(struct sockaddr*)&local,sizeof(local)) < 0)
	{}//

	if(listen(sock,5) < 0)
	{}//
	return sock;
}

int fds[256];
int fws[256];
int main(int argc,char* argv[])
{
	int listen_sock = start_up("192.168.188.128",8080);
	int done = 0;
	int new_sock = -1;

	struct sockaddr_in flient_sock;
	socklen_t len = sizeof(flient_sock);

	int i;
	int max_fd = -1;
	for(i = 0; i<256 ;++i)
	{
		fds[i] = -1;
		fws[i] = -1;
	}
	fds[0] = listen_sock;

	fd_set _reads,_writes;

	while(!done)
	{
		/*初始化工作*/
		FD_ZERO(&_reads);
		FD_ZERO(&_writes);
		struct timeval _timeout = {1,0}; 
		for(i = 0; i < 256 ;++i)
		{
			if(fds[i] > 0)
			{
 				FD_SET(fds[i],&_reads);
				max_fd = max_fd > fds[i] ? max_fd : fds[i];
			}
			if(fws[i] > 0)
			{
				FD_SET(fws[i],&_writes);
				max_fd = max_fd > fws[i] ? max_fd : fws[i];
			}
		}

		switch(select(max_fd+1,&_reads,&_writes,NULL,&_timeout))
		{
			case -1:
				perror("select error\n");
				break;
			case 0:
				printf("timeout!\n");
				break;
			default:
				{
					for(i = 0;i<256;++i)
					{
						if(fds[i] == listen_sock && FD_ISSET(fds[i],&_reads))
						{//钓到鱼杆了
							printf("钓到鱼杆了！\n");
							new_sock = accept(listen_sock,(struct sockaddr*)&flient_sock,&len);
							if(new_sock < 0){}//
							for(i=0;i<256;++i)
							{
								if(fds[i] == -1)
								{
									fds[i] = new_sock;
									break;
								}
							}
							if(i == 256)
							{
								close(new_sock);
							}
						}
						else if(fds[i] > 0 && FD_ISSET(fds[i],&_reads))
						{//钓到鱼了
 							char buf[1024];
							ssize_t _s = read(fds[i],buf,sizeof(buf)-1);
							if(_s > 0)
							{
 								buf[_s] = '\0';
								printf("客户端的消息：%s\n",buf);
								int j = 0;
								for(j = 0;j<256;++j)
								{
									if(fws[j] == -1)    //从读的队列中去除，加入到写的队列
									{
										fws[j] = fds[i];
										fds[i] = -1;
										break;
									}
								}
							}
							else if(_s == 0)
							{
								fds[i] = -1;
								close(new_sock);
								printf("客户端已经关闭！\n");
							}
							else
							{}//
						}
						else if(fws[i] > 0 && FD_ISSET(fws[i],&_writes))
						{
							char* p = "HTTP/1.1 200 OK\r\n\r\n<html><h1>hello world!</h1></html>\r\n";                       write(fws[i],p,strlen(p));
							int j;
							for(j = 0;j<256;j++)
							{
								if(fds[j] == -1)
								{
									fds[j] = fws[i];
									fws[i] = -1;
									break;
								}
							}
						}
					}
				}
				break;
		}
	}
	return 0;
}
