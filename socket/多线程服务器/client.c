#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>

void msage(const char* p)
{
	printf("请输入：%s [ip] [port]\n",p);
}

int main(int argc, char* argv[])
{
	if(argc != 3)    //如果命令行参数不为3 则提醒用户后推出！
	{
		msage(agrv[0]);
		return 1;
	}

	int conn = socket(AF_INET,SOCK_STREAM,0);
	if(conn < 0)
	{
		printf("sock\n");
		return 2;
	}
	
	struct sockaddr_in remote;
	remote.sin_family = AF_INET;
	remote.sin_addr.s_addr = inet_addr(argv[1]);
	remote.sin_port = htons(atoi(argv[2]));
	if(connect(conn,(struct sockaddr*)&remote,sizeof(remote))<0)
	{
		perror("connece\n");
		return 3;
	}
	char buf[1024];
	while(1)
	{
		ssize_t _s = read(0,buf,sizeof(buf)-1);
		if(_s > 0)
		{
			buf[_s] = '\0';
			write(conn,buf,strlen(buf));
			read(conn,buf,sizeof(buf)-1);
			printf("%s\n",buf);
		}
	}
	return 0;
}
