//首先定义了一个people数据结构，（在这里采用数据结构的方式是因为，共享内存区的数据往往是有固定格式的，这由通信的各个进程决定，采用结构的方式有普遍代表性）。map_normfile1首先打开或创建一个文件，并把文件的长度设置为5个people结构大小。然后从mmap()的返回地址开始，设置了10个people结构。然后，进程睡眠10秒钟，等待其他进程映射同一个文件，最后解除映射。 
#include<sys/mman.h> 
#include<sys/types.h> 
#include<fcntl.h> 
#include<unistd.h> 
#include<string.h>
#include<stdio.h>
typedef struct{ 
	char name[4]; 
	int   age; 
}people; 
main(int argc, char** argv) // map a normal file as shared mem: 
{ 
	int fd,i; 
	people *p_map; 
	char temp[2] = {'a','\0'}; 
	fd=open(argv[1],O_CREAT|O_RDWR|O_TRUNC,0644); 
	lseek(fd,sizeof(people)*5-1,SEEK_SET); 
	write(fd,"",1); 
	p_map = (people*) mmap( NULL,sizeof(people)*10,PROT_READ|PROT_WRITE,\
			MAP_SHARED,fd,0 ); 
	close( fd ); 
	for(i=0; i<10; i++) 
	{ 
		temp[0] += 1; 
		memcpy( ( *(p_map+i) ).name, &temp,2); 
		( *(p_map+i) ).age = 20+i; 
	}
	printf("init over\n");
	sleep(10);
	munmap( p_map, sizeof(people)*10 ); 
	printf( "umap ok \n" ); 
}
