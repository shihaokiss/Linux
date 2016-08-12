//httpd.h
#include<stdio.h> 
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <fcntl.h>
#include <pthread.h>
#include <sys/stat.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<netinet/in.h>
#define _SIZE_ 1024
#define _DEBUG_
 
char* get_text(int fd,char* buf);

void error_all(int fd,int err,char* reason);

void cgi_action(int fd,char* method,char* url,char* parameter);
 
char* get_length(int fd,char * content_length);
 
void* http_action(void* client_sock);
 
void echo_error(int fd,int _errno);
 
void echo_html(int fd,const char* url,int size );
