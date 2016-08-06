#include<stdio.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<errno.h>
int main()
{
	int fd = open("./log",O_CREAT | O_RDWR);  //调用open打开文件
	if(fd < 0)
	{
		perror("open()");
		return 1;
	}
	close(1);
	int new_fd = dup(fd);   //将文件描述符 fd 重定向为 stdin 
	if(new_fd == -1)
	{
		printf("dup");
		return 2;
	}
	close(fd);
	char buf[1024];
	while(1)
	{
		memset(buf,'\0',sizeof(buf));
		fgets(buf,sizeof(buf),stdin);
		if(strcmp("quit",buf,4) == 0)
		{
			break;
		}
		printf("%s",buf);
		fflush(stdout);
	}
		return 0;
}
