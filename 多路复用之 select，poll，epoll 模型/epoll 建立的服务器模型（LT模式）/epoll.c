#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <string.h>

static int startup(const char *ip, int port)       //���������׽���������
{
	int sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock<0)
	{
		perror("sock");
		return 2;
	}

	int opt = 1;                      //��ֹ time_wait �ķ���
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
	int epfd = epoll_create(128);                        //����epollģ��
	if (epfd<0)
	{
		perror("epoll_create");
		return 5;
	}
	struct epoll_event ev;            //��ʼ��һ�� epoll_event �Ľṹ��
	ev.events = EPOLLIN;                //��ʾ��Ӧ���ļ����������Զ���epoll�������Ķ��¼���
	ev.data.fd = listen_sock;           //���׽�����������
	if (epoll_ctl(epfd, EPOLL_CTL_ADD, listen_sock, &ev)<0)      //epoll���¼�ע�ắ��
	{
		perror("epoll_ctl");
		return 6;
	}
	struct epoll_event evs[128];      //����һ�� epoll_event �Ľṹ��
	int len = sizeof(evs) / sizeof(evs[0]);
	int ready = 0;
	int timeout = -1;
	while (1)
	{
		switch (ready = epoll_wait(epfd, evs, len, timeout))       //�ռ���epoll��ص��¼����Ѿ��������¼�
		{													 //epoll����ѷ������¼���ֵ��evs������ 
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
							   ev.data.fd = new_sock;         //���µ��׽�����������ӵ� epoll ģ���м�������
							   if (epoll_ctl(epfd, EPOLL_CTL_ADD, new_sock, &ev)<0)
							   {
								   perror("epoll_ctl");
								   return 9;
							   }
						   }
					   }
					   else
					   {
						   //��� evs[i] epoll ģ�͹��ĵĶ�
						   if (evs[i].events&EPOLLIN)
						   {
							   char buf[1024];
							   ssize_t _s = read(evs[i].data.fd, buf, sizeof(buf)-1);
							   if (_s>0)
							   {
								   buf[_s] = '\0';
								   printf("client # %s\n", buf);
								   //read finish change to write
								   ev.data.fd = fd;                //�ı�� epoll ģ�Ͷ��׽����������Ĺ�ע״̬
								   ev.events = EPOLLOUT;
								   epoll_ctl(epfd, EPOLL_CTL_MOD, fd, &ev);   //ԭ�� epoll ģ�͹��ĵ��� ���׽����������Ķ��¼�
							   }                                         //���� epoll ģ�͹��ĵ��� ���׽�����������д�¼�
							   else if (_s == 0)
							   {
								   printf("client close ....\n");
								   epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);    //epoll ģ��ȥ���� ���׽��ֵĹ�ע
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
							   write(fd, msg, strlen(msg));                //�ı�� epoll ģ�Ͷ��׽����������Ĺ�ע״̬
							   epoll_ctl(epfd, EPOLL_CTL_DEL, fd, NULL);



							   //ԭ�� epoll ģ�͹��ĵ��� ���׽�����������д�¼�
							   close(fd);                                //���� epoll ģ�ͶԴ��׽����������Ĺ���
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