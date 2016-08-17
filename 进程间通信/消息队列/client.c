//client.c
 
#include "comm.h"
 
#define _SEVER_TYPE_ 1
#define _CLIENT_TYPE_ 2
 
int main()
{
    int msg_id = get_msg_queue();
 
    char buf[_SIZE_];
     
    while(1)
    {
        printf("please enter: ");
        fflush(stdout);
 
        int size = read(0, buf, sizeof(buf)-1);        //先从标准输入中向 buf 中写消息
        buf[size-1] = '\0';
        send_msg(msg_id, _CLIENT_TYPE_, buf);          //向消息队列中写入 _CLIENT_TYPE_ 类型的消息
 
        if(strncmp("quit", buf, strlen(buf)) == 0)
        {
            break;
        }
 
        memset(buf, '\0', sizeof(buf));
 
        recv_msg(msg_id, _SEVER_TYPE_, buf);        //向 buf 中读入 _SEVER_TYPE_ 类型的消息
        printf("sever -> client : %s \n", buf);
 
        if(strncmp("quit", buf, strlen(buf)) == 0)
        {
            break;
        }
 
    }
 
    destroy_msg_queue(msg_id);
 
    return 0;
}