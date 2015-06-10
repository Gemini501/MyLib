#include "ClientSocket.h"
#include  <iostream>
using namespace std;
#ifdef WIN32
	#pragma comment(lib, "wsock32")
#endif
static char socketIP[30];
#ifdef WIN32
bool CMySocket::isWinSocketStart = false;
int CMySocket::socketNum = 0;
void CMySocket::WinSocketStart(void)
{
	socketNum ++;
	if(isWinSocketStart == false)
		{	
			WSADATA wsaData;
			//#define MAKEWORD(a,b) ((WORD) (((BYTE) (a)) | ((WORD) ((BYTE) (b))) << 8)) 
			WORD version = MAKEWORD(2, 0);
			int ret = WSAStartup(version, &wsaData);//win sock start up
			isWinSocketStart = true;
		}
}

void CMySocket::WinSocketClean(void)
{
	
	if(isWinSocketStart == true)
	{
		if(socketNum == 1)
		{
			WSACleanup();
			isWinSocketStart = false;
		}
	}
	socketNum -- ;		
}
#endif
char* CMySocket::InetNtoa(unsigned int  ip)
{
	struct in_addr addr1;
	memcpy(&addr1, &ip, 4 );
	char *tem = inet_ntoa(addr1);
	strcpy_s(socketIP, tem);
	return socketIP;
}

char*  CMySocket::DnsToIp(char *dns)
{
#ifdef WIN32
	WinSocketStart();
#endif
	struct hostent *host;
	host=gethostbyname(dns);
	char *tem = inet_ntoa(*(struct in_addr*)(host->h_addr));
	strcpy_s(socketIP, tem);
	return socketIP;
}


	
CMySocket::CMySocket( char* ip, int port )
{
#ifdef WIN32
	WinSocketStart();
#endif

	m_port = port;
	strcpy_s( m_ip, ip );
	m_sock_fd = -1;
	m_callback = NULL;
}

CMySocket::~CMySocket()
{
	this->MyClose();
#ifdef WIN32
	WinSocketClean();
#endif
}


int CMySocket::MyCreateSocket(void)
{
	int flags, s;
	int sock_fd = socket( AF_INET, SOCK_STREAM, 0 );//在进行网络连接前，需要用socket函数向系统申请一个通信端口
	if( sock_fd < 0 )
	{
		return -1;
	}
#ifdef WIN32

#else
	//Set non-block
	flags = fcntl (sock_fd, F_GETFL, 0);//get sock fd flags
	if (flags == -1)
	{
		close(sock_fd);
		return -1;
	}
	flags |= O_NONBLOCK;//set fd's flag to non-block
	s = fcntl (sock_fd, F_SETFL, flags);
	if (s == -1)
	{
		close(sock_fd);
		return -1;
	}
#endif
	m_sock_fd = sock_fd;
	return m_sock_fd;
}

int CMySocket::MyConnect(void)
{
	int ret;
	struct sockaddr_in clientAddr;
	memset(&clientAddr, 0, sizeof(clientAddr));//置字节字符串s的前n个字节为零且包括‘\0’。
	clientAddr.sin_family = AF_INET;//一般来说 AF_INET（地址族）PF_INET（协议族 ）
	clientAddr.sin_port = htons(m_port);//必须要采用网络数据格式,普通数字可以用htons()函数转换成网络数据格式的数字
	clientAddr.sin_addr.s_addr = inet_addr(m_ip);//ip地址
	ret = connect(m_sock_fd, (sockaddr*)&clientAddr, sizeof(sockaddr_in));//建立与一个端的连接 若无错误发生 连接成功返回0
#ifdef WIN32
#else
	if( ret == -1 )
	{
		if( errno == EINPROGRESS )
		{
			return EVENT_PROGRESS;
		}
	}
#endif
	return ret;
}
int CMySocket::MyRecv( void* buf, int len )
{
	return recv( m_sock_fd, (char *)buf, len, 0 );
}

int CMySocket::MySend( void* buf, int len )
{
	return send( m_sock_fd, (char *)buf, len, 0 );
}

int CMySocket::MyClose(void)
{
	if( m_sock_fd >= 0 )
	{
	#ifdef WIN32
		closesocket(m_sock_fd);
	#else
		close( m_sock_fd );
	#endif
		m_sock_fd = -1;
	}
	return 0;
}
 

void CMySocket::MySetSocketCallBack(MY_SOCKET_CALLBACK cb)
{
	m_callback = cb;
}
