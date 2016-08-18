#pragma once
 
#include <stdio.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <unistd.h>
#include <errno.h>
 
#define _PROJ_NAME_ "./tmp"
#define _PROJ_ID_ 0x6666
 
//struct sembuf
//{
//    unsigned short sem_num;  /* semaphore number */
//    short          sem_op;   /* semaphore operation */
//    short          sem_flg;  /* operation flags */
//};
 
int creat_sem_set(int sems);
int init_sem_set(int msg_id, int which, int val);
int get_sem_set();
int destory_sem_set(int msg_id);
int P(int sem_id, int which);
int V(int sem_id, int which);