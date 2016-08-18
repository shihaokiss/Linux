#include "comm.h"
 
 
static int comm_sem_set(int sems, int flag)
{
    key_t _key = ftok(_PROJ_NAME_, _PROJ_ID_);        //创建一个键值 
    if(_key < 0)
    {
        perror("ftok");
        return -1;
    }
								  //属性
    int sem_id = semget(_key, sems, flag);        //创建信号量的描述符
							  //创建的信号量集中的信号量的个数
    if(sem_id < 0)
    {
        perror("semget");
        return -2;
    }
 
    return sem_id;
}
 
int create_sem_set(int sems)
{
    int flag = IPC_CREAT | IPC_EXCL | 0644;
    return comm_sem_set(sems, flag);
}
 
int init_sem_set(int msg_id, int which, int val)
{						     //第几个信号量 //value
    int ret = semctl(msg_id, which, SETVAL, val);
	               //标识符         //方式 设置信号量集中的一个单独的信号量的值。 
    if(ret < 0)
    {
        perror("semctl");
        return -1;
    }
    return 0;
}
 
 
int get_sem_set()
{
    int flag = IPC_CREAT;
    return comm_sem_set(0, flag);
}
 
int destory_sem_set(int sem_id)
{
    int ret = semctl(sem_id, 0, IPC_RMID);
    if(ret < 0)
    {
        perror("semctl");
        return -1;
    }
    return 0;
}
 
static int op(int sem_id, int which, int op)
{
    struct sembuf _sem;            //sembuf 结构体
    _sem.sem_num = which;
    _sem.sem_op = op;
    _sem.sem_flg = 0;
 
    int ret = semop(sem_id, &_sem, 1);
    if(ret < 0)
    {
        perror("semop");
        return -1;
    }
    return 0;
}
 
int P(int sem_id, int which)
{
    return op(sem_id, which, -1);
}
 
int V(int sem_id, int which)
{
    return op(sem_id, which, +1);
}