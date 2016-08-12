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
    int old_fl=fcntl(fd,F_GETFL);    //F_GETFL  ȡ��fd���ļ�״̬��־
    if(old_fl<0)
    {
        perror("fcntl");
        return -1;
    }
    if(fcntl(fd,F_SETFL,old_fl|O_NONBLOCK))     //F_SETFL  ����������״̬��־    
    {                                           //O_NONBLOCK  ������I/O����� read() ����û�пɶ�ȡ�����ݣ�
        perror("fcntl");                        //������� write() û�п�д�����ݣ���read��write���ý�����-1��EAGAIN����
        return -2;
    }
    return 0;
}

static void Usage(char* const proc)      //�������Ƿ���ȷ
{
    assert(proc);
    printf("%s [ip][port]\n",proc);
}
    
static int startup(char *ip,int port)    //��ȡ�����׽���������
{
    assert(ip);
    int sock=socket(AF_INET,SOCK_STREAM,0);
    if(sock<0)
    {
        perror("socket");
        exit(1);
    }
    
    int opt=1;             //ȡ�� time_wait ʱ���
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
		                           //��Ӧ�ó������һЩ������(non-blocking)����(���ļ���socket)��ʱ��
								   ���磬�� O_NONBLOCK�ı�־���ļ�/socket/FIFO�������������read����
								   ��û�����ݿɶ�����ʱ���򲻻����������ȴ�����׼���������أ�read������
								   ����һ������EAGAIN����ʾ���Ӧ�ó�������û�����ݿɶ����Ժ����ԡ�		   
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
        if(errno==EAGAIN)                   //errno=EAGAIN��ʾ�������write�����Ƿ��������������û�����ݿ�д��
		                                    //���ʱ��ͻ���ȫ�ֱ���errnoΪEAGINA,��ʾ���Ӧ�ó�������û������
											//��д���Ժ����ԡ�
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
    int epoll_fd=epoll_create(_SIZE_);       //����һ�� epoll ģ��
    if(epoll_fd<0)
    {
        perror("epoll_create");
        return -1;
    }

    struct epoll_event ev;                  //���� eopll_event �ṹ��
    ev.data.fd=sock;                         
    ev.events=EPOLLIN|EPOLLET;              //��ʾ�� epoll ģ�͹��� sock �׽����������Ķ����ԣ�����
	                                        //ģʽѡ��Ϊ ET ģʽ��Ч�ʸߣ�
											//ETģʽ�ںܴ�̶��ϼ�����epoll�¼����ظ������Ĵ��������Ч��Ҫ��LTģʽ�ߡ�
    if(epoll_ctl(epoll_fd,EPOLL_CTL_ADD,sock,&ev)<0)   //�� ET ģʽ�� epoll ģ��ע��һ�� sock �׽���������
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
        switch(num=epoll_wait(epoll_fd,ev_out,max,timeout))           //���� epoll ģ���е��¼����м��
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
							//ӭ����Ա�ӵ�������
                            int new_sock=accept(fd,(struct sockaddr*)&client,&len);                                                                                       158                             if(new_sock<0)
                            {
                                perror("accept");
                                printf("%s:%d\n",strerror(errno),new_sock);
                                continue;
                            }
                            set_non_block(new_sock);    //�����ļ�������״̬��ʹ read��write ��Ϊ��������ʽ������
							                            //��ֹ��������
                            ev.events=EPOLLIN|EPOLLET;  //ʹ epoll ģ�͹��Ĵ��ļ��������Ķ�״̬������ʹ�������� ET ģʽ
                            ev.data.fd=new_sock;
                            epoll_ctl(epoll_fd,EPOLL_CTL_ADD,new_sock,&ev);  //�� epoll ģ����ע���µ��ļ�������
                            printf("get a new connect...\n");
                        }
                        else
                        {
                            //read events ready
							//������Ա׼���÷����ˣ�read ��ķ���
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
                                    ev.events=EPOLLOUT|EPOLLET;     // �����׽���������������Ϣ�޸�
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
							//������Ա׼���÷����ˣ�write ��ķ���
                            else if(ev_out[i].events&EPOLLOUT)
                            {
								char *msg="HTTP/1.1 200 OK\r\n\r\n<html><h1>hello world,i am here ~~</h1></html>\r\n";
                                write_data(fd,msg,strlen(msg));
                                close(fd);
                                epoll_ctl(epoll_fd,EPOLL_CTL_DEL,mem->fd,NULL);    // epoll ģ��ȡ���Դ��׽����������Ĺ�ע
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
    int listen_sock=startup(ip,port);   //���������׽���
    server_epoll(listen_sock);          //ʵ�� epoll ģ�͵� ET ģʽ
    close(listen_sock);                 //�رռ����׽���
    return 0;
}


//epoll���ȵ���epoll_create����һ��epoll���󣬲���size���ں˱�֤�ܹ���ȷ��������������������������ʱ�ں˿ɲ���֤Ч����
//epoll_ctl���Բ��������epoll�����磬���ս�����socket���뵽epoll�������أ����߰�epoll���ڼ�ص�ĳ��socket����Ƴ�epoll��
//���ټ�����ȵȡ�
//epoll_wait�ڵ���ʱ���ڸ�����timeoutʱ���ڣ����ڼ�ص����о�������¼�����ʱ���ͷ����û�̬�Ľ��̡�
//��˿��Կ���epoll����select/poll����Ϊ����ÿ�ε���ʱ��Ҫ��������Ҫ��ص�����socket��socket/pollϵͳ���ã�����ζ����Ҫ����
//��̬��socket�б�copy���ں�̬���������Ƶľ��ÿ�ζ�Ҫcopy��ʮ����KB���ڴ浽�ں�̬���ǳ���Ч��������epoll_waitʱ���൱����
//������select/poll��������ʱȴ���ô���socket������ںˣ���Ϊ�ں��Ѿ���epoll_ctl���õ���Ҫ��صľ���б�








