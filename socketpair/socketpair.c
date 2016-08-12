//先说说我的理解：socketpair创建了一对无名的套接字描述符（只能在AF_UNIX域中使用），描述符存储于一个二元数组,eg. s[2] .这对套接字可以进行双工通信，每一个描述符既可以读也可以写。这个在同一个进程中也可以进行通信，向s[0]中写入，就可以从s[1]中读取（只能从s[1]中读取），也可以在s[1]中写入，然后从s[0]中读取；但是，若没有在0端写入，而从1端读取，则1端的读取操作会阻塞，即使在1端写入，也不能从1读取，仍然阻塞；反之亦然.....

#include<stdio.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<errno.h>
#include<string.h>

int main()
{
	int pipe[2];
	if(socketpair(AF_LOCAL,SOCK_STREAM,0,pipe)<0)  //创建双端管道
	{
		perror("socketpair\n");
		return 1;
	}

	pid_t pid = fork();
	if(pid == 0)
	{//child
		close(pipe[0]);
		char buf[1024];
		char* msg = "你好！\n";
		while(1)
		{
			memset(buf,'\0',sizeof(buf));
			strcpy(buf,msg);
			write(pipe[1],buf,strlen(msg));
			read(pipe[1],buf,sizeof(buf)-1);
			sleep(1);
			printf("来自父亲反馈的消息：%s\n\n",msg);
			sleep(2);
		}
	}
	else if(pid > 0)
	{//father
		close(pipe[1]);
		char buf[1024];
		while(1)
		{
			memset(buf,'\0',sizeof(buf));
			read(pipe[0],buf,sizeof(buf)-1);
			printf("来自孩子的信息 ：%s\n",buf);
			write(pipe[0],buf,strlen(buf));
		}
	}
	return 0;
}
