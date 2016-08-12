#include<stdio.h>                                                           
#include<stdlib.h>
#include<sys/epoll.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<fcntl.h>
#include<assert.h>
#include<errno.h>
#include<string.h>

#define _BACKLOG_ 5
#define _SIZE_ 256
#define _MAX_FD_SIZE_ 64
#define _BUF_SIZE_ 10240

typedef struct data_buf
{
    int fd;
    char buf[_BUF_SIZE_];
}data_buf_t,*data_buf_p;

static int set_non_block(int fd)
{
    int old_fl=fcntl(fd,F_GETFL);    //F_GETFL  取得fd的文件状态标志
    if(old_fl<0)
    {
        perror("fcntl");
        return -1;
    }
    if(fcntl(fd,F_SETFL,old_fl|O_NONBLOCK))     //F_SETFL  设置描述符状态标志    
    {                                           //O_NONBLOCK  非阻塞I/O，如果 read() 调用没有可读取的数据，
        perror("fcntl");                        //或者如果 write() 没有可写的数据，则read或write调用将返回-1和EAGAIN错误
        return -2;
    }
    return 0;
}

static void Usage(char* const proc)      //检测参数是否正确
{
    assert(proc);
    printf("%s [ip][port]\n",proc);
}
    
static int startup(char *ip,int port)    //获取监听套接字描述符
{
    assert(ip);
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        perror("socket");
        exit(1);
    }
    
    int opt=1;             //取消 time_wait 时间的
    setsockopt(sock,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));

    struct sockaddr_in local;
    local.sin_family=AF_INET;
    local.sin_port=htons(port);
    local.sin_addr.s_addr=inet_addr(ip);

    if(bind(sock,(struct sockaddr*)&local,sizeof(local))<0)
    {
        perror("bind");
        exit(2);   
    }
    if(listen(sock,_BACKLOG_)<0)
    {
        perror("listen");
        exit(3);
    }
    return sock;
}


int read_data(int fd,char *buf,int size)
{
    assert(buf);
    memset(buf,'\0',size);
    int index=0;
    ssize_t _s=-1;
    while(_s=read(fd,buf+index,size-index)<size)
    {
        if(errno==EAGAIN)          
		                           //当应用程序进行一些非阻塞(non-blocking)操作(对文件或socket)的时候。
								   例如，以 O_NONBLOCK的标志打开文件/socket/FIFO，如果你连续做read操作
								   而没有数据可读，此时程序不会阻塞起来等待数据准备就绪返回，read函数会
								   返回一个错误EAGAIN，提示你的应用程序现在没有数据可读请稍后再试。		   
        {
            break;
        }
        index+=_s;     
    }
    return index;
}

int write_data(int fd,char *buf,int size)
{
    int index=0;
    ssize_t _s=-1;
    while(_s=write(fd,buf+index,size-index)<size)
    {
        if(errno==EAGAIN)                   //errno=EAGAIN表示的是你的write本来是非阻塞情况，现在没有数据可写，
		                                    //这个时候就会置全局变量errno为EAGINA,表示你的应用程序现在没有数据
											//可写请稍后再试。
        {
            break;
        }
        index+=_s;
    }
    return index;
}

//sock=listen_sock
static int server_epoll(int sock)
{   
    int epoll_fd=epoll_create(_SIZE_);       //创建一个 epoll 模型
    if(epoll_fd<0)
    {
        perror("epoll_create");
        return -1;
    }

    struct epoll_event ev;                  //设置 eopll_event 结构体
    ev.data.fd=sock;                         
    ev.events=EPOLLIN|EPOLLET;              //表示此 epoll 模型关心 sock 套接字描述符的读属性，并且
	                                        //模式选择为 ET 模式（效率高）
											//ET模式在很大程度上减少了epoll事件被重复触发的次数，因此效率要比LT模式高。
    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,sock,&ev)<0)   //向 ET 模式下 epoll 模型注册一个 sock 套接字描述符
    {
        perror("epoll_ctl");
        return -2;
    }   
    struct epoll_event ev_out[_MAX_FD_SIZE_];
    int max=_MAX_FD_SIZE_;

    int timeout=5000;
    int i=0;
    int num=-1;
    int done=0;
    while(!done)    
    {
        switch(num=epoll_wait(epoll_fd,ev_out,max,timeout))           //将对 epoll 模型中的事件进行检测
        {
            case -1://errno
                perror("epoll_wait");
                break;
            case 0://timeout
                printf("timeout...\n");
                break;
            default: //events ready 
                {
                    for(i=0;i<num;++i)
                    {
                        //get a new connect
                        if(ev_out[i].data.fd==sock&&(ev_out[i].events&EPOLLIN))
                        {
                            struct sockaddr_in client;
                            socklen_t len=sizeof(client);

                            int fd=ev_out[i].data.fd;
							//迎宾人员接到客人了
                            int new_sock=accept(fd,(struct sockaddr*)&client,&len);                                                                                       158                             if(new_sock<0)
                            {
                                perror("accept");
                                printf("%s:%d\n",strerror(errno),new_sock);
                                continue;
                            }
                            set_non_block(new_sock);    //设置文件描述符状态（使 read，write 都为非阻塞方式工作）
							                            //防止进程阻塞
                            ev.events=EPOLLIN|EPOLLET;  //使 epoll 模型关心此文件描述符的读状态，并且使它工作在 ET 模式
                            ev.data.fd=new_sock;
                            epoll_ctl(epoll_fd,EPOLL_CTL_ADD,new_sock,&ev);  //向 epoll 模型中注册新的文件描述符
                            printf("get a new connect...\n");
                        }
                        else
                        {
                            //read events ready
							//服务人员准备好服务了（read 类的服务）
                            if(ev_out[i].events&EPOLLIN)
                            {
                                int fd=ev_out[i].data.fd;
								char buf[10240];                       
                                ssize_t _s=read_data(fd,buf,sizeof(buf)-1);
                                if(_s>0)
                                {
                                    buf[_s]='\0';
                                    printf("client:%s\n",buf);
									ev.data.fd=fd;
                                    ev.events=EPOLLOUT|EPOLLET;     // 将此套接字描述符监测的信息修改
                                    epoll_ctl(epoll_fd,EPOLL_CTL_MOD,fd,&ev);
                                }
                                else if(_s==0)
                                {
                                    printf("client close...\n");
                                    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,fd,NULL);
                                    close(fd); 
                                }
                                else
                                {
                                    continue;
                                }
                            }
							//服务人员准备好服务了（write 类的服务）
                            else if(ev_out[i].events&EPOLLOUT)
                            {
								char *msg="HTTP/1.1 200 OK\r\n\r\n<html><h1>hello world,i am here ~~</h1></html>\r\n";
                                write_data(fd,msg,strlen(msg));
                                close(fd);
                                epoll_ctl(epoll_fd,EPOLL_CTL_DEL,mem->fd,NULL);    // epoll 模型取消对此套接字描述符的关注
                            }
                            else  
                            {
                            }
                        }
                    }
                }
                break;
        }   
    }
}

int main(int argc,char *argv[])
{
    if(argc!=3)
    {
        Usage(argv[0]);
        exit(1);
    }
    char *ip=argv[1];
    int port=atoi(argv[2]);      
    int listen_sock=startup(ip,port);   //建立监听套接字
    server_epoll(listen_sock);          //实现 epoll 模型的 ET 模式
    close(listen_sock);                 //关闭监听套接字
    return 0;
}


//epoll首先调用epoll_create建立一个epoll对象，参数size是内核保证能够正确处理的最大句柄数，多于这个最大数时内核可不保证效果。
//epoll_ctl可以操作上面的epoll，例如，将刚建立的socket加入到epoll中让其监控，或者把epoll正在监控的某个socket句柄移出epoll，
//不再监控它等等。
//epoll_wait在调用时，在给定的timeout时间内，当在监控的所有句柄中有事件发生时，就返回用户态的进程。
//因此可以看出epoll优于select/poll：因为后者每次调用时都要传递你所要监控的所有socket给socket/poll系统调用，这意味着需要将用
//户态的socket列表copy到内核态，如果以万计的句柄每次都要copy几十几百KB的内存到内核态，非常低效。而调用epoll_wait时就相当于以
//往调用select/poll，但是这时却不用传递socket句柄给内核，因为内核已经在epoll_ctl中拿到了要监控的句柄列表。








