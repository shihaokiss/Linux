//httpd.c
#include "httpd.h"
 
#define DEFAULT "src_html/a.gif"
#define IMG "src_html"
#define CGI "src_cgi"
#define _DEBUG_

int times=0;
void echo_error(int fd,int _errno)        //处理错误的请求
{
    switch(_errno)
	{
    case 400://Bad Request  //客户端请求有语法错误，不能被服务器所理解
        break;
    case 404:////请求资源不存在，eg：输入了错误的URL
        //printf("*******404\n");
        error_all(fd,404,"NOT_FIND!!");
        break;
    case 401://请求未经授权，这个状态代码必须和WWW-Authenticate报头域一起使用 
        break;
    case 403://服务器收到请求，但是拒绝提供服务 
        break;
    case 500:// Internal Server Error //服务器发生不可预期的错误
        break;
    case 503://Server Unavailable  //服务器当前不能处理客户端的请求，一段时间后可能恢复正常
        break;
    default:
        break;
    }
    close(fd);
}
 
void error_all(int fd,int err,char* reason){   
    //#define _SIZE_ 1024
    char buf[_SIZE_]="";
    char error[_SIZE_]="";
    sprintf(buf,"HTTP/1.1 %d %s\r\n\r\n",err,reason); //buf 字符串初始化为 “” 中的内容   
	                                                  //（请求行）
    sprintf(error," %d %s\n",err,reason);                     
//  printf("err buf:%s\n",buf);
    write(fd,buf,strlen(buf));
    write(fd,"<html>\n",strlen("<html>\n"));
    write(fd,"<head>\n",strlen("<head>\n"));
    write(fd,"<h1> HELLO PPH!!!</h1>\n",strlen("<h1> HELLO PPH!!!</h1>\n"));
    write(fd,"<h2>\n",strlen("<h2>\n"));
    write(fd,error,strlen(error));
    write(fd,"</h2>\n",strlen("</h2>\n"));
    write(fd,"</head>\n",strlen("</head>\n"));
    write(fd,"</html>\n",strlen("</html>\n"));
//  echo_html(fd,"src_html/1.html",102400000);

}
 
int get_line(int sock_fd,char* line){                     //获取一行的信息并打印出来
    int index=0;
    ssize_t size=1;
    char ch=0;
	//\r->\n     \r\n->\n    \n->\n
    while(ch!='\n')
	{
        if((size=read(sock_fd,&ch,1))<0)
		{
            perror("read_get_line");
            return -1;
        }
        if(ch=='\r')
		{
            char tmp=0;
            if(recv(sock_fd,&tmp,1,MSG_PEEK)>0)          //MSG_PEEK 查看当前数据。数据将被复制到缓冲区
														 //中，但并不从输入队列中删除。
														 //这里的作用是来“窥探”一下,下一个字符是不是‘\n’
			{
                if(tmp=='\n')
				{
                    line[index++]=tmp;
                    read(sock_fd,&ch,1);
                }
				else
				{
					line[index++] = '\n';
				}
				continue;
            }
        }
        if(index==1024)
		{
            line[1023]='\0';
            return -1;
        }
             
        line[index++]=ch;
    }
    line[index]='\0';
    if(strcmp(line,"\n")==0)      //空行 return 0;
	{
        return 0;
    }
    return 1;                     //非空行 return 1;
}
 
void echo_html(int fd,const char* url,int fd_size ){        //通过rul进行相关的操作
    char buf[_SIZE_]="HTTP/1.1 200 OK\r\n\r\n";
	if(url==NULL)         //要向客户端返回默认的页面操作了
	{
        int rfd=-1;
        off_t set=0;
        ssize_t size=0;
		printf("%s\n",DEFAULT);
		if((rfd=open(DEFAULT, O_RDONLY))<0)
		{
            echo_error(fd,500);
            exit(0);
        }
#ifdef _DEBUG_		
#endif
        write(fd,buf,strlen(buf));            //将 "HTTP/1.1 200 OK\r\n\r\n" 发送到服务器端
        size=sendfile(fd,rfd,NULL,fd_size);   //第三个参数为偏移量
 		                                      //将 rfd 文件中的 从偏移set个位的位置开始复制 fd_size个字到 fd 文件中 
        if(size<0)
		{
            close(rfd);
            printf("senfile error!\n");
            return;
        }
        printf("size: %d\n",size);
        close(rfd);
     
    }
	else
	{
        int rfd=-1;
        off_t set=0;
        ssize_t size=0;
#ifdef _DUBUG_
        printf("url:%s\n",url);    
#endif		
        if((rfd=open(url, O_RDONLY))<0)   //以只读的方式打开 路径为 url 的文件
		{
            echo_error(fd,500);
        //  exit(0);
        }
        write(fd,buf,strlen(buf)); 
        size=sendfile(fd,rfd,NULL,fd_size);
        if(size<0)
		{
            printf("senfile error!\n");
            return;
        }
    //  printf("size: %d\n",size);
        close(rfd);
        return;
    }
}

//清除client_sock
int  clear_buf(int fd)
{
    char buf[_SIZE_]="";
    ssize_t size=1;
    size=read(fd,buf,_SIZE_);      
     
    if(size<0)
	{
        perror("clear_buf_read");
        return -1;
    }
    return 0;
}
 
 
//获取post正文参数的大小
char* get_length(int fd,char* content_length){
    int size=1;
    int tag=0;
    int index=0;
	char* tmp;
    while(size!=0)
	{//通过持续读取一行 直到读到空行结束
        size = get_line(fd,tmp);
        if(strncasecmp(tmp,"content-length: ",16) == 0)
		{
            content_length = tmp;
        }
        if(size==-1)
		{
            printf("get line出错\n");
            return NULL;
        }
   
    }														// content_length == content-length: 1000000\n
    content_length[strlen(content_length)-1]='\0';          // content_length == content-length: 1000000\0
    strcpy(content_length,content_length+16);               // content_length == "1000000"
    printf("con end: %s\n",content_length);
    return content_length;
}
 
void cgi_action(int fd,char* method,char* url ,char* parameter)
{
    char env[20]="METHOD=";
    char par[_SIZE_]="PARAMETER=";
    int input[2];
	int output[2];
    if((pipe(input)<0))
	{
        perror("pipe");
        return;
    }
	if((pipe(output)<0))
	{
        perror("pipe");
        return;
    }
    strcat(env,method);                //"METHOD=GET/POST"          
    strcat(par,parameter);             //方法是GET eg:"data1=100&data2=200"      
									   //方法是POST eg:"PARAMETER=2000"
//#ifdef _DUBUG_									   
    printf(" act url:%s\n",url);       //路径
    printf("parameter:%s\n",par);      //参数
//#endif

    if(putenv(env)<0)        //将env添加为环境变量            
	{
        perror("putenv");
        return;
    }
    if(putenv(par)<0)		//将par添加为环境变量    
	{
        perror("putenv par");
        return;
    }

    pid_t id=fork();    //创建子进程
    if(id<0)
	{
        perror("fork");
        return;
    }
	
    else if(id==0) 
	{//子进程                        
        close(input[1]);             //子进程关闭input写端
		close(output[0]);			 //关闭output读端
		
        //printf("child\n");
        if(dup2(input[0],0)<0)      //文件描述符的重定向    
		{
            perror("dup2.1");
            return;
        }
        if(dup2(output[1],1)<0)		 //文件描述符的重定向    
		{
            perror("dup2.2");
            return;
        }
        if(execl(url,NULL)<0)       //头文件：#include <unistd.h>
                                    //函数定义：int execl(const char *path, const char *arg, ...);
									//函数说明：execl()用来执行参数path字符串所代表的文件路径， 接下来的
									//参数代表执行该文件时传递的argv[0],argv[1].....最后一个参数必须用空指针NULL作结束。
									//进程的程序替换
		{
            perror("execl");
            printf("exit url:\n",url);
            exit(-2);
        }
          
		  
		/*需要再写一个 add() 函数*/
    }
    else
	{//父进程
        close(input[0]);         
		close(output[1]);
        char buf[_SIZE_]="";
        int count=0;
        int i=0;
        ssize_t size=-1;
        while(size>0)
		{
			size=read(fd,buf,_SIZE_);           //从客户端读数据
			if(size<0)
			{
                echo_error(fd,500);
                break;
            }

            size=write(input[1],buf,_SIZE_);    //向 input 管道中写数据
            if(size<0)
			{
                echo_error(input[1],500);
                break;
            }
          

            size = read(output[0],buf,strlen(buf));    //从 output 管道中读数据
			if(size <0)
			{
				echo_error(output[0],500);
                break;
			}
			
			size = write(fd,buf,strlen(buf));     //向服务器里面写数据
			if(size < 0)
			{
				echo_error(output[1],500);
                break;
			}
         
        }
        waitpid(0,NULL,0);
    }  
}
 
 
 
 
void* http_action(void * client_sock){ 

	printf("haha i am in http_action fun\n");
    int fd = (int)client_sock;
	printf("get a new client : %d\n", fd);
    struct stat stat_buf; 
    char line[_SIZE_];
    char* method=NULL;        //方法
    char* url=NULL;           //路径及参数
	char* version=NULL;       //版本
    char* parameter=NULL;     //get方法 参数
    char  text_length[_SIZE_]="";
    int cgi=0;        //看到时候是否使用cgi方法
	if (get_line(fd, line) == -1)    //获取客户端的请求行信息 ： 
 	                             //例一 ：GET /hao.360.cn/？data1=100&data2=200
								 //例二 ：POST /hao.360.cn/？data1=100&data2=200
	{
        printf("this is a cache requset , so can't process!\n");
        return NULL;
    }

	printf("获取的信息：%s\n",line);

    int index=strlen(line)-1;      
    //printf("%d\n index",index);
    while(index>0)
	{//提取method url method
        if(line[index]==' '&&version==NULL)
		{
            version=((char*)line)+index+1;
            line[index]=0;
        }
        if(line[index]==' '&&url==NULL){
            url=line+index+1;
            line[index]=0;
        }
        --index;
    }
    method=line;    

//#ifdef _DEBUG_
	printf("method is %s\n", method);
	printf("url is %s\n",url);              //      url                                   version
	printf("version is %s\n",version);      //       ↓                                    ↓
  						                    //GET'\0'/hao.360.cn/？data1=100&data2=200'\0'HTTP/1.0
	                    					//↑
						                    //method
//#endif
						
    char real_url[128]=IMG;       // "src_cgi + ..." 在服务器本地的绝对路径
	printf("real_url : %s\n",real_url);
    //判断请求方法
    if(strcasecmp("POST",method)==0)      //判断是 POST 方法时
	{

        strcat(real_url,url);
        printf("real_url :%s\n",real_url);
		int i = 0;
		while(real_url[i] != '\0')              // src_cgi/hao.360.cn/'\0'
		{
			if (real_url[i] = '?')
			{
 				real_url[i] = '\0';
				break;
			}
			++i;
		}
//#ifdef _DUBUG_
        printf("start:%s %s %s",method,url,version);   
//#endif
        get_length(fd,text_length);        //获取 请求正文的长度！
        cgi_action(fd,method,real_url,text_length);   
        printf("fork ...end!\n");
    }
    else if(strcasecmp("GET",method)==0)
	{
        index=0;
        while(url[index] != '\0')
		{
            if(url[index]=='?')
			{
                cgi = 1;
				url[index] = '\0';
				parameter = url+index+1;
                break;
            }
            ++index;
        }
        if(cgi==1)
		{   
			                                 //    real_url               parameter
                                             //cgi;                           //    ↓                      ↓
            strcat(real_url,url);            //    scr_cgi/hao.360.cn/'\0'data1=100&data2=200'\0'
            printf("start:%s %s %s",method,real_url ,version);   
            cgi_action(fd,method,real_url,parameter);
            printf("ret cgi\n");
        }
        else
		{
            if(strcmp("/",url)==0)           //非 cgi 模式
			{
				if(stat(DEFAULT,&stat_buf) < 0)
				{
					printf("i am wrong \n");
				}
				printf("%s   %d",__FUNCTION__,__LINE__);
				printf("fd : %d st_size : %d\n",fd,stat_buf.st_size);
				echo_html(fd, NULL,stat_buf.st_size);
            }
            else
			{
                 strcat(real_url,url);             //"src_cgi + url"
#ifdef _DUBUG_
                printf("real_url :%s\n",real_url);
#endif
                if(stat(real_url, &stat_buf)<0)
				{   
                     echo_error(fd,404);
                    //printf("times:%d end!\n",times++);
                    //close(fd);
                    //return;
                }
                else
				{
                     printf("fd size:%d",stat_buf.st_size);
                    echo_html(fd,real_url,stat_buf.st_size);
                }
            }
             
        }
    }
    else
	{
        echo_error(fd,403);
    }
    if(strcasecmp(method,"POST")==0)
	{
        printf("fork ...end!\n");
        close(fd);
        printf("times:%d end!\n",times++);
        return;
    }
    if(clear_buf(fd)<0)
	{
        printf("clear error\n");
    }
    else
	{
		printf("clear success!\n");	
	}
    printf("times:%d end!\n",times++);
	close(fd);
	return;  
}
