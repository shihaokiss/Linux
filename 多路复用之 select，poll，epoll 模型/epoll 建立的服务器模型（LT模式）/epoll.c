#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <string.h>

static int startup(const char *ip, int port)       //创建监听套接字描述符
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock<0)
	{
		perror("sock");
		return 2;
	}

	int opt = 1;                      //防止 time_wait 的发生
	setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	struct sockaddr_in local;
	local.sin_family = AF_INET;
	local.sin_port = htons(port);
	local.sin_addr.s_addr = inet_addr(ip);
	if (bind(sock, (struct sockaddr *)&local, sizeof(local))<0)
	{
		perror("bind");
		return 3;
	}
	if (listen(sock, 5)<0)
	{
		perror("listen");
		return 4;
	}
	return sock;
}

static void usage(const char *proc)
{
	printf("%s [ip] [port]", proc);
}

int main(int argc, char *argv[])
{
	if (argc != 3)
	{
		usage(argv[0]);
		return 1;
	}
	int listen_sock = startup(argv[1], atoi(argv[2]));
	int epfd = epoll_create(128);                        //创建epoll模型
	if (epfd<0)
	{
		perror("epoll_create");
		return 5;
	}
	struct epoll_event ev;            //初始化一个 epoll_event 的结构体
	ev.events = EPOLLIN;                //表示对应的文件描述符可以读（epoll关心它的读事件）
	ev.data.fd = listen_sock;           //和套接字描述符绑定
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &ev)<0)      //epoll的事件注册函数
	{
		perror("epoll_ctl");
		return 6;
	}
	struct epoll_event evs[128];      //定义一个 epoll_event 的结构体
	int len = sizeof(evs) / sizeof(evs[0]);
	int ready = 0;
	int timeout = -1;
	while (1)
	{
		switch (ready = epoll_wait(epfd, evs, len, timeout))       //收集在epoll监控的事件中已经就绪的事件
		{													 //epoll将会把发生的事件赋值到evs数组中 
		case 0:
			printf("timeout..\n");
			break;
		case -1:
			perror("epoll_wait");
			return 7;
			break;
		default:
		{
				   int i = 0;
				   for (i; i<ready; i++)
				   {
					   //LISTEN  SOCKET
					   int fd = evs[i].data.fd;
					   if (i == 0 && fd == listen_sock&&evs[i].events&EPOLLIN)
					   {
						   struct sockaddr_in peer;
						   socklen_t len = sizeof(peer);
						   int new_sock = accept(listen_sock, (struct sockaddr*)&peer, &len);
						   if (new_sock<0)
						   {
							   perror("accept");
							   return 8;
						   }
						   else
						   {
							   printf("get new socket:ip %s:port %d\n", inet_ntoa(peer.sin_addr), ntohs(peer.sin_port));
							   ev.events = EPOLLIN;
							   ev.data.fd = new_sock;         //将新的套接字描述符添加到 epoll 模型中监视起来
							   if (epoll_ctl(epfd, EPOLL_CTL_ADD, new_sock, &ev)<0)
							   {
								   perror("epoll_ctl");
								   return 9;
							   }
						   }
					   }
					   else
					   {
						   //这个 evs[i] epoll 模型关心的读
						   if (evs[i].events&EPOLLIN)
						   {
							   char buf[1024];
							   ssize_t _s = read(evs[i].data.fd, buf, sizeof(buf)-1);
							   if (_s>0)
							   {
								   buf[_s] = '\0';
								   printf("client # %s\n", buf);
								   //read finish change to write
								   ev.data.fd = fd;                //改变该 epoll 模型对套接字描述符的关注状态
								   ev.events = EPOLLOUT;
								   epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);   //原先 epoll 模型关心的是 此套接字描述符的读事件
							   }                                         //现在 epoll 模型关心的是 此套接字描述符的写事件
							   else if (_s == 0)
							   {
								   printf("client close ....\n");
								   epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);    //epoll 模型去消对 此套接字的关注
								   close(fd);
							   }
							   else
							   {
								   perror("read");
								   return 10;
							   }
						   }
						   //write
						   else if (evs[i].events & EPOLLOUT)
						   {
							   char *msg = "HTTP/1.1 200 OK\r\n\r\n<html><h1>hello momo</h1></html>\r\n";
							   //my_send(fd,msg,sizeof(msg)-1);
							   write(fd, msg, strlen(msg));                //改变该 epoll 模型对套接字描述符的关注状态
							   epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);



							   //原先 epoll 模型关心的是 此套接字描述符的写事件
							   close(fd);                                //现在 epoll 模型对此套接字描述符的关心
						   }
						   else
						   {
							   continue;
						   }

					   }
				   }
		}
			break;
		}
	}
	return 0;

}