//comm.c
 
#include "comm.h"
 
//#define _SEVER_TYPE_ 1
//#define _CLIENT_TYPE_ 2
 
 
static int comm_msg_queue(int flag)  //for sever
{
    key_t _key = ftok(_PROJ_NAME_, _PROJ_ID);    //数ftok把一个已存在的路径名和一个整数标识得转换成一个key_t值，
    if(_key < 0)
    {
        perror("ftok");
        return -1;
    }
 
    int msg_id = msgget(_key, flag);      //创建新消息队列或取得已存在消息队列 
    if(msg_id < 0)
    {
        perror("msgget");
        return -2;
    }
 
    return msg_id; 
}
 
int create_msg_queue()          //server 创建的消息队列
								//IPC_CREAT   如果IPC不存在，则创建一个IPC资源，否则打开操作。                                       
					            //IPC_EXCL：只有在共享内存不存在的时候，新的共享内存才建立，否则就产生错误。
{
    int flag = IPC_CREAT | IPC_EXCL | 0644;
    return comm_msg_queue(flag);
}
 
int get_msg_queue()  //client 创建消息队列
					  
{
    int flag = IPC_CREAT;
    return comm_msg_queue(flag);
}
 
int destroy_msg_queue(int msg_id)        //销毁消息队列
{
    int ret = msgctl(msg_id, IPC_RMID, NULL);
	//IPC_STAT : 该命令用来获取消息队列对应的 msqid_ds 数据结构，并将其保存到 buf 指 定的地址空间。 
	//IPC_SET : 该命令用来设置消息队列的属性，要设置的属性存储在buf中。            
	//IPC_RMID : 从内核中删除 msqid 标识的消息队列。
    if(ret < 0)
    {
        perror("msgctl");
        return -1;
    }
    return 0;
}
 
int send_msg(int msg_id, int t, const char* msg)
{
    struct msgbuf _buf;                  //自己写出来的结构体 msgbuf；
    _buf.mtype = t;                      //初始化 msgbuf 结构体
    strncpy(_buf.mtext, msg, strlen(msg)+1);
 
    int ret = msgsnd(msg_id, &_buf, strlen(_buf.mtext)+1, 0);
	//如果msgﬂg和常 数IPC_NOWAIT合用，则在msgsnd()执行时若是消息队列已满，
	//则msgsnd()将不会阻塞，而 会立即返回-1，如果执行的是msgrcv()，则在
	//消息队列呈空时，不做等待马上返回-1，并设定 错误码为ENOMSG。当msgﬂg为
	//0时，msgsnd()及msgrcv()在队列呈满或呈空的情形时，采取阻塞等待的处理模式。 
    if(ret < 0)
    {
        perror("msgsend");
        return -1;
    }
 
    return 0;
}
 

int recv_msg(int msg_id, int t, char* out)
{
    struct msgbuf _buf;
    _buf.mtype = t;
 
    int ret = msgrcv(msg_id, &_buf, sizeof(_buf.mtext), t, 0);
    if(ret < 0)
    {
        perror("msgrcv");
        return -1;
    }
    strcpy(out, _buf.mtext);
 
    return 0;
}