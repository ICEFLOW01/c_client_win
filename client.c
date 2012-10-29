#include"client.h"

int sockfd;//连接套接字描述符；
char ip[256] = {0};
int port;

int tcp_close()
{ 
#ifndef _MINGW_
	shutdown(sockfd, SHUT_RDWR );
	close(sockfd);
#else
	closesocket(sockfd);
#endif
	return 0;
}

int Socket_Read(int nSock,char* pBuf,int nSize)
{
	int ret;
#ifndef _MINGW_        
	ret = read(nSock,pBuf,nSize);
#else
	ret = recv(nSock,pBuf,nSize,0);
#endif
	return ret;
}

int Socket_Write(int nSock,char* pBuf,int nSize)
{
	int ret;
#ifndef _MINGW_        
	ret = write(nSock,pBuf,nSize);
#else
	ret = send(nSock,pBuf,nSize,0);
#endif 
	return ret;
}

int Socket_Create(int af,int type,int protocol)
{
	int sock=0;

#ifdef _MINGW_
	WSADATA wsaData ={0};
	if(WSAStartup(MAKEWORD(2,2),&wsaData)) return -1;

	sock=socket(af,type,protocol);
	if (sock == INVALID_SOCKET) return -1;
#else

	sock=socket(af,type,protocol);
	if (sock == -1) return -1;
#endif

	return sock;
}

int unpackage(char*buf, int len)//解析查询结果数据包；
{
	QA_HEAD* 	head 	= (QA_HEAD*)buf;;
	INFOR* 	p 	= (INFOR*)(buf + sizeof(QA_HEAD));
	// printf("\n******len %d\n******id %d\n", change_buf->package_len, change_buf->package_id);
	if(head->package_id != 11)
	{
		return -1;
	}
	if(head->ack != 0)
	{
		printf("你要查询的信息不存在！\n");
		return -2;
	}

	if(len != QA_HEAD +  head->numb * sizeof(INFOR) || len != head->package_len){
		printf("包长度错误\n");
		return -3;
	}

	int i;
	for(i = 0 ; i < head->numb ; i++){
		printf("你要查询的信息如下:\n姓名：%s\n简拼：%s\n全拼：%s\n公司电话：%s\n私人电话：%s\n分机号：%s\nEmail：%s\n",
				p->myname,
				p->abbreviation,
				p->full,
				p->company,
				p->privation,
				p->extension,
				p->emall
		      );
		p += sizeof(INFOR);
	};
	return 0;    
}

struct name_type name = {
	.init = 0 ,
	.number = 0 ,
	.version = 3 ,
}

#define MAX_LEN 4097	// 返回包最大是4095

int read_ser()//读数据的函数；
{
	int 	ret 		= -1;
	int 	len 		= 0;
	char 	buf[MAX_LEN]	= {0};
	QA_HEAD* 	head 		= (QA_HEAD*) buf;

	do{
		ret = Socket_Read(sockfd, buf+len, sizeof(buf) - len - 1);
		if( ret < 0 ) {
			printf("stock读错误\n");
			return ret;
		}
		len += ret;
		if ( len == sizeof(buf) - 1 && ret ) {
			printf("返回包太长\n");
			return -2;
		}
	}while( ret != 0 );

	if(len < sizeof(QA_HEAD)){
		printf("返回包太短\n");
		return -3;
	}

	if(len != head->package_len) {
		printf("返回包长度错误\n");
		return -4;
	}

	return len;
}

int read_stdin(char*data, int len)
{
	int ret = -1;
	fflush(stdout);
	fflush(stdin);

	printf("查找支持：中文名、简拼、全拼、公司手机号、私人号码、分机号、邮箱！\n退出系统请输入：quit\n");
	printf("input message:");
	fflush(stdout);
	ret = read(0, data, len-1);//从标准输入中读取数据；

	if(ret == -1)
	{   
		perror("write_ser read error\n");
		exit(0);
	}
	if(strncmp(data, "quit", sizeof("quit")-1)==0)
	{
		exit(0);
	}
	fflush(stdout);
	fflush(stdin);
	// memcpy(data, "lgj", sizeof("lgj"));
	// sleep(1);
	return 0;
}

int read_input(char* buf, int buf_size)
{
	fflush(stdin);
	read_stdin(buf + sizeof(QR_HEAD), QR_BODY_LEN );
	return QR_BODY_LEN ;
}

int write_ser(char* buf, int buf_len)//写数据的函数；
{   
	int len = 0;
	char buf[sizeof(QR_PACK)] = {0};    
	QR_PACK* pack = (QR_PACK*)buf;

	if(buf_len != QR_BODY_LEN ) {
		printf("write_ser buf_len error\n");
		return -1;
	}

	pack->package_len = 72;//定长包；
	pack->package_id = 10;//协议类型；

	while(1) {
		int ret = -1;
		ret = Socket_Write(sockfd, buf + len, sizeof(buf) - len);//把标准输入的数据写入到套接字里面；
		if(ret < 0) {
			perror("write_ser write error\n");
			return ret;
		}
		if(ret == sizeof(buf)) {
			break;
		}
		len = +ret;
		if(len == sizeof(buf)) {
			break;
		}
	}
	return len ;
}


int tcp_init()
{
	int ret = -1;

	sockfd = Socket_Create(AF_INET, SOCK_STREAM, 0);
	if (sockfd == -1)
	{
		perror("socket error\n");
		return -1;
	}	
	//IPv4地址族结构；
	struct sockaddr_in seraddr;
	memset (&seraddr, 0, sizeof(seraddr));
	seraddr.sin_family = AF_INET;
	seraddr.sin_port = htons(port);
	seraddr.sin_addr.s_addr=inet_addr(ip);


	ret = connect(sockfd, (struct sockaddr*)&seraddr, sizeof(seraddr));//连接；
	if(ret != 0)
	{
		perror("connect error\n");
		exit(0);
	}
	printf("与服务器连接成功！\n");

	return 0;
}

int main(int argc, char* argv[])
{
	int ret = -1;;

	if(argc<3)
	{
		printf("Usage:%s option\n",argv[0]);
		printf("for example:%s ip port \n",argv[0]);
		return 0;
	}
	sscanf(argv[1], "%s", ip);
	sscanf(argv[2], "%d", &port);
	while(1)
	{
		int len = 0 ;
		int ret = -1 ;
		ret = tcp_init();
		if(ret < 0) {
			tcp_close();
			return -1;
		}
		build_package();
		write_ser();
		len = read_ser();
		if(len > 0)
			unpackage(buf, len);
		tcp_close();
	}

	return 0;
}
