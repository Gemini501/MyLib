#ifndef __CLIENT_EPOLL_H__
#define __CLIENT_EPOLL_H__

//#include "cocos2d.h"
#include "ClientSocket.h"
#ifdef WIN32
	#include <winsock.h>
#else
#include <sys/epoll.h>
#include <pthread.h>
#endif
//USING_NS_CC;

#define _SOCKET_FD_MAX_	3
#define EVENTMAX	(_SOCKET_FD_MAX_*3)

typedef void (*MY_SOCKET_FD_CALLBACK)(int event,void* buf,int len);

class CMyEpoll
{
public:
	CMyEpoll();
	~CMyEpoll();
public:
	int MyCreateEpoll(void);
	int MyAddSocketFdEvent(int socket_fd, MY_SOCKET_FD_CALLBACK cb);
	int MyDelSocketFdEvent(int socket_fd);
	void RunThread(void);
	void MyRun(float time);
	MY_SOCKET_FD_CALLBACK MyGetSocketFdCallBack(int socket_fd);
public:
	static bool m_exitflag;
	static int m_socket_fd[_SOCKET_FD_MAX_];
	static MY_SOCKET_FD_CALLBACK m_cb[_SOCKET_FD_MAX_];
#ifdef WIN32
	static bool socketConnect[_SOCKET_FD_MAX_];
#endif
	int m_epfd;
};
#endif
