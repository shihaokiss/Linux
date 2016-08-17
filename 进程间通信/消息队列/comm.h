//comm.h
 
#pragma once
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <string.h>
#include <errno.h>
 
#define _PROJ_NAME_ "./tmp"
#define _PROJ_ID 0x666
#define _SIZE_ 1024
 
#define _SEVER_TYPE_ 1
#define _CLIENT_TYPE_ 2
 
struct msgbuf {
    long mtype;       /* message type, must be > 0 */
    char mtext[_SIZE_];    /* message data */
};
 
int create_msg_queue();
int get_msg_queue();  //for client
int destroy_msg_queue(int msg_id);
int send_msg(int msg_id, int t, const char* msg);
int recv_msg(int msg_id, int t, char* out);