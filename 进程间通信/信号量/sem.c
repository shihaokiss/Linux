#include "comm.h"
 
int main()
{
 
    int sem_id = create_sem_set(1);
    init_sem_set(sem_id, 0, 1);
    pid_t id = fork();
    if(id == 0)  //child
    {
        int c_sem_id = get_sem_set();
        while(1)
        {
            P(c_sem_id, 0);       //获取 0 信号量的资源 -1
            printf("A");
            fflush(stdout);
            usleep(rand()%123456);
            printf("A");
            fflush(stdout);
            usleep(rand()%12345);
            V(c_sem_id, 0);       //释放 0 号信号量的资源 +1
        }
    }
    else  // father
    {
        while(1)
        {
            P(sem_id, 0);       //获取 0 号信号量的资源 -1
            printf("B");
            fflush(stdout);
            usleep(rand()%123459);
            printf("B");
            fflush(stdout);
            usleep(rand()%12344);
            V(sem_id, 0);       //获取 0 号信号量的资源时
        }
 
        wait(NULL);
        destory_sem_set(sem_id);
    }
 
    return 0;
}