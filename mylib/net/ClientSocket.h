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
	EVENT_PROGRESS = 0,/*socket����״̬*/
	EVENT_CONNECTED,/*buf = NULL len = 0*/
	EVENT_READ,/*buf = �յ��������� len = �յ����ݳ���*/
};


//event: ��Ϣ����
//buf: ����
//len: ���� 
typedef void (*MY_SOCKET_CALLBACK)(int event,void* buf,int len);

/*
�������Է������ķ�ʽ���������ӵ�ʱ��
���صĽ������� -1,�Ⲣ�����������
�ӷ����˴���������ķ��ؽ���� EINPROGRESS��
��ô�ʹ������ӻ��ڽ����С� �������ͨ��poll��
��select���ж�socket�Ƿ��д���������д��˵
����������ˡ�
*/
class CMySocket
{
public:
	CMySocket( char* ip, int port );
	~CMySocket();

public:
	int MyCreateSocket(void);//����socket������Ϊ������
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
	//socket���
	int m_sock_fd;
	char m_ip[24];
	int m_port;
	MY_SOCKET_CALLBACK m_callback;
};

#endif
