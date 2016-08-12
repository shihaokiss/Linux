//主程序，包含tcp 服务端的bind listen
 
#include <stdio.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include "httpd.h"
#include <netdb.h>
#include <sys/utsname.h>
#include <net/if.h>
#include <sys/ioctl.h>
#define ERRORIP -1
 
int startup(char* ip,int port)         //设置监听套接字描述符
{
//  printf("%s %d\n",ip,port);
    int sock=socket(AF_INET,SOCK_STREAM,0);
    struct sockaddr_in server;
    server.sin_family=AF_INET;
    server.sin_addr.s_addr=inet_addr(ip);
    server.sin_port=htons(port);
    int flag=1;
    if( setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag)) == -1)     //防止 time_wait 的时间等待
    {  
        perror("setsockopt");  
        exit(1);                  
    }  
    if(bind(sock,(struct sockaddr *)&server,sizeof(server))<0){
        perror("bind");
        exit(-2);
    }
    if(listen(sock,5)<0){
        perror("listen");
        exit(-3);
    }
    //printf("%d\n",sock);
    return sock;
}
 
void usage(char* arg)
{
    printf("usage %s [ip] [port]\n",arg);
 
}
 
void http_start(int listen_sock){
 
    int done=0;
    int client_sock=-1;            
//  printf("http_start\n");
    struct sockaddr_in client;          //接收远端IP,PORT的结构体
    socklen_t len=sizeof(client);
    while(!done)
	{
//      printf("循环\n");
        if((client_sock=accept(listen_sock,(struct sockaddr *)&client,&len))<0)      //创建新的套接字描述符
		{   //
            printf("connect error!\n");
            //做访问记录 存入数据库
            continue;
        }
		
        pthread_t tid;     
	    printf("client:%d,%s,%d\n",client_sock, inet_ntoa(client.sin_addr), ntohs(client.sin_port));
        int ret=pthread_create(&tid,NULL,http_action,(void*)client_sock);      //进程创建
        if(ret!=0){
        //  printf("create thread error!\n");
            printf("%s\n",strerror(errno));
            continue;
        }
        else
		{
            pthread_detach(tid);          //线程的分离
        }
     
     
    }
     
}
 
int main(int argc,char* argv[])
{
    if(argc!=3){
        usage(argv[0]);
        exit(-1);
    }
    int listen_sock=0;
    char* ip=NULL;
    listen_sock=startup(argv[1],atoi(argv[2]));        //创建监听套接字
    printf("listen_sock : %d\n",listen_sock);
    http_start(listen_sock);                           //启动 http 服务器
    close(listen_sock);                                //关闭监听套接字
    return 0;
}
 
 
 
 

 
 
 
 

