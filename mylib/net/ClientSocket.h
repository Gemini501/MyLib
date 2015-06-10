#ifndef __CLIENT_SOCKET__H__
#define __CLIENT_SOCKET__H__

#ifdef WIN32
	#include <winsock.h>
#else
	#include <sys/socket.h>
	#include <sys/errno.h>
	#include <fcntl.h>
	#include <arpa/inet.h>
	#include <netinet/in.h>
	#include <netdb.h>
#endif

enum{
	EVENT_CONNECT_ERROR = -10,/*buf = NULL len = 0*/
	EVENT_READ_ERROR,
	EVENT_ERROR = -1,/*buf = NULL len = 0*/
	EVENT_PROGRESS = 0,/*socket阻塞状态*/
	EVENT_CONNECTED,/*buf = NULL len = 0*/
	EVENT_READ,/*buf = 收到数据类型 len = 收到数据长度*/
};


//event: 消息类型
//buf: 数据
//len: 长度 
typedef void (*MY_SOCKET_CALLBACK)(int event,void* buf,int len);

/*
当我们以非阻塞的方式来进行连接的时候，
返回的结果如果是 -1,这并不代表这次连
接发生了错误，如果它的返回结果是 EINPROGRESS，
那么就代表连接还在进行中。 后面可以通过poll或
者select来判断socket是否可写，如果可以写，说
明连接完成了。
*/
class CMySocket
{
public:
	CMySocket( char* ip, int port );
	~CMySocket();

public:
	int MyCreateSocket(void);//创建socket并设置为非阻塞
	int MyConnect(void);
	int MyRecv( void* buf, int len );
	int MySend( void* buf, int len );
	int MyClose(void);
	void MySetSocketCallBack(MY_SOCKET_CALLBACK cb);
	int MyGetSockFd(void){return m_sock_fd;}
	MY_SOCKET_CALLBACK MyGetSocketCallBack(void){


		return m_callback;
	}
	static char* InetNtoa(unsigned int  ip);
	static char*  DnsToIp(char *dns);
#ifdef WIN32
	static void WinSocketStart(void);
	static void WinSocketClean(void);
	static bool isWinSocketStart;
	static int socketNum;
#endif
private:
	//socket句柄
	int m_sock_fd;
	char m_ip[24];
	int m_port;
	MY_SOCKET_CALLBACK m_callback;
};

#endif
