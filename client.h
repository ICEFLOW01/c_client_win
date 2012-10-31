#include<stdio.h>
#include<stdlib.h>
#include<errno.h>
#include<assert.h>
#include<string.h>
#include <locale.h>
#ifndef _MINGW_
#include<event2/event.h>
#include<event2/bufferevent.h>
#include<mysql/mysql.h>
#include<time.h>
#include<netinet/in.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<pthread.h>
#include<sys/types.h>
#include<unistd.h>
#include<fcntl.h>
#else
#include <windows.h>

#endif

#define HASH_TABLE_MAX_SIZE 10000//哈希数组大小；
#define TCP_MAX 60000//设定最大的连接数；
#define LISTEN_PORT 5000
#define LISTEN_BACKLOG 32
#define QR_PACK_LEN sizeof(QR_PACK)
#define QR_ID       10
#define MAX_LEN 4097	// 返回包最大是4096
#define TERMINAL_PRINT printf

typedef struct client_mes_struct
{
		int id_flag;             //标志位；
		int total_len;             //包长度；
        int had_len;          //已经存取长度；
		char buf[4096];      //保存信息的buf；
        struct event *write_event;
        struct event *read_event;
}CLIENT_MES;

CLIENT_MES client_mes[65535];

typedef struct Infor_Struct//员工信息结构体；
{
    char myname[32];
    char abbreviation[32];
    char full[32];
    char company[32];
    char privation[32];
    char extension[32];
    char emall[32];
}INFOR;

typedef struct HashNode_Struct//定义哈希数组类型；
{
    char* sKey;
    INFOR* infor;
    struct HashNode_Struct*pNext;
}HashNode;

#define QR_BODY_LEN 64

typedef struct {//定义请求包；
    int package_len;
    int package_id;
    char body[QR_BODY_LEN];
}QR_PACK;

typedef struct {
    int package_len;
    int package_id;
}QR_HEAD;

typedef struct {
    int package_len;
	int infor_num;
    int package_id;
}QA_HEAD;

int main_loop();
int tcp_init();
int read_input(char*, int);
int read_stdin(char*, int);
int build_package(char*, int);
int write_ser(char*, int);
int read_ser(char*, int);
int unpackage(char*, int);
int Socket_Create(int, int, int);
int Socket_Write(int, char*, int);
int Socket_Read(int, char*, int);
int tcp_close();

#ifndef _MINGW_
int server_init();
int tcp_init();
void do_accept(evutil_socket_t listener, short event, void *arg);
void read_cb(int fd, short event, void*arg);
int judgment_existence(int fd);
int data_processing(int fd, char*line, int len, void*arg);
int exception_handling(int fd, char*buf, int len, void*arg);
int write_mes(int fd, char*buf, QA_HEAD qa_head, void*arg);
void write_cb(int fd, short event, void*arg);
#endif
void hash_table_init();
unsigned int hash_table_hash_str(const char*skey);
void hash_table_release();
int update_hash_table();
int init_hash_table();
void display_header();
void hash_table_insert(const char* skey, INFOR* nvalue);
HashNode** hash_table_lookup(char* skey);
