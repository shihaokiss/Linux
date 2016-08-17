//sever.c
 
#include "comm.h"
 
#define _SEVER_TYPE_ 1
#define _CLIENT_TYPE_ 2
 
int main()
{
    int msg_id = create_msg_queue();       //创建一个新的 消息对列标识符
    char buf[_SIZE_];
     
    while(1)
    {
        memset(buf, '\0', sizeof(buf));
        sleep(10);
        recv_msg(msg_id, _CLIENT_TYPE_, buf);       //我要读 _CLIENT_TYPE_ 类型的消息
        printf("client -> sever : %s \n", buf);
 
        if(strncmp("quit", buf, strlen(buf)) == 0)
        {
            break;
        }
 
        printf("please enter: ");
        fflush(stdout);
 
        int size = read(0, buf, sizeof(buf)-1);     //重标准输入往 buf 中读消息
        buf[size-1] = '\0';
        send_msg(msg_id, _SEVER_TYPE_, buf);        //写的是 _SEVER_TYPE_ 类型的消息
 
        if(strncmp("quit", buf, strlen(buf)) == 0)
        {
            break;
        }
    }
 
    destroy_msg_queue(msg_id);             //程序退出时销毁消息队列
 
    return 0;
}