#include "ClientEpoll.h"


bool CMyEpoll::m_exitflag = false;
int CMyEpoll::m_socket_fd[_SOCKET_FD_MAX_] = {0};
MY_SOCKET_FD_CALLBACK CMyEpoll::m_cb[_SOCKET_FD_MAX_] ={0};
#ifdef WIN32
bool CMyEpoll::socketConnect[_SOCKET_FD_MAX_] = {0};
#endif
CMyEpoll::CMyEpoll()
{
	m_epfd = -1;
	for( int i = 0; i < _SOCKET_FD_MAX_; i++ )
	{
		CMyEpoll::m_socket_fd[i] = -1;
		CMyEpoll::m_cb[i] = NULL;
	}

	CMyEpoll::m_exitflag = false;
}

CMyEpoll::~CMyEpoll()
{
	CMyEpoll::m_exitflag = true;
	
#ifdef WIN32
#else
	if( m_epfd >= 0 )
		{
			close( m_epfd );
			m_epfd = -1;
		}
#endif
}

//����socket
int CMyEpoll::MyCreateEpoll(void)
{
#ifdef WIN32 
	return 0;
#else
	/*
	�� ��������һ��epollר�õ��ļ�������������ʵ�����ں�����һ�ռ䣬
	������������ע��socket fd���Ƿ����Լ�������ʲô�¼���
	size�����������epoll fd���ܹ�ע�����socket fd�������㶨���ˡ�ֻҪ���пռ䡣
	*/
	m_epfd = epoll_create(1024);
	if( m_epfd == -1 )
	{
		return -1;
	}
	return m_epfd;
#endif
}

//handle: sockid
//cb : socket callback fun
int CMyEpoll::MyAddSocketFdEvent(int socket_fd, MY_SOCKET_FD_CALLBACK cb)
{
	for( int i = 0; i < _SOCKET_FD_MAX_; i++ )
	{
		if( CMyEpoll::m_socket_fd[i] == -1 )    //
		{
			CMyEpoll::m_socket_fd[i] = socket_fd;
			CMyEpoll::m_cb[i] = cb;
			#ifdef WIN32
			CMyEpoll::socketConnect[i] = false;
			#endif
			break;
		}
	}
#ifdef WIN32
	return 0;
#else
	struct epoll_event event = {0};
	event.data.fd = socket_fd;
	event.events = (EPOLLIN  | EPOLLOUT);
	
	/*
	�ú������ڿ���ĳ��epoll�ļ��������ϵ��¼���
	����ע���¼����޸��¼���ɾ���¼�
	*/
	int nRet = epoll_ctl (m_epfd, EPOLL_CTL_ADD, socket_fd, &event);

	if (nRet == -1)
	{
		return -1;
	}
	return 0;
#endif
}

//handle: sockid
//cb : socket callback fun
int CMyEpoll::MyDelSocketFdEvent(int socket_fd)
{
	for( int i = 0; i < _SOCKET_FD_MAX_; i++ )
	{
		if( CMyEpoll::m_socket_fd[i] == socket_fd )
		{
			CMyEpoll::m_socket_fd[i] = -1;
			CMyEpoll::m_cb[i] = NULL;
		}
	}
	return 0;
#ifdef WIN32

#else
	struct epoll_event event = {0};
	event.data.fd = socket_fd;
	event.events = EPOLLIN;
	int nRet = epoll_ctl (m_epfd, EPOLL_CTL_DEL, socket_fd, &event);

	if (nRet == -1)
	{
		return -1;
	}
	return 0;
#endif
}

MY_SOCKET_FD_CALLBACK CMyEpoll::MyGetSocketFdCallBack(int socket_fd)
{
	for( int i = 0; i < _SOCKET_FD_MAX_; i++ )
	{
		if( CMyEpoll::m_socket_fd[i] == socket_fd )
		{
			return CMyEpoll::m_cb[i];
		}
	}
	return NULL;
}

#define PROCESS_DATA_LEN	(4*1024)
static char data[PROCESS_DATA_LEN] = {0};
#ifdef WIN32

void CMyEpoll::RunThread( void )
{
	fd_set         fdread;
	fd_set         fdwrite;
	struct timeval tv = {0,20};
	//while(1)
		{
			if(CMyEpoll::m_exitflag )
				return ;
			
			FD_ZERO(&fdread);
			FD_ZERO(&fdwrite);

			for( int i = 0; i < _SOCKET_FD_MAX_; i++ )
			{
				if(CMyEpoll::m_socket_fd[i] != -1)
					{
						FD_SET(CMyEpoll::m_socket_fd[i], &fdread);
						FD_SET(CMyEpoll::m_socket_fd[i], &fdwrite);
					}
			}
				
			switch(select(0, &fdread, &fdwrite, NULL, &tv))
				{
					case 0:
						break;

					default:
					
						for( int i = 0; i < _SOCKET_FD_MAX_; i++ )
							{
								if(CMyEpoll::m_socket_fd[i] != -1)
									{
										if(FD_ISSET(CMyEpoll::m_socket_fd[i],&fdread))
											{
												char data[PROCESS_DATA_LEN] = {0};
												int count = recv(CMyEpoll::m_socket_fd[i], (char*)data, PROCESS_DATA_LEN, 0);
												if(CMyEpoll::m_cb[i] && count > 0)
													CMyEpoll::m_cb[i]( EVENT_READ, data, count );
											}
										if(!CMyEpoll::socketConnect[i] && FD_ISSET(CMyEpoll::m_socket_fd[i],&fdwrite))
											{
												CMyEpoll::socketConnect[i] = true;
												if(CMyEpoll::m_cb[i])
													CMyEpoll::m_cb[i]( EVENT_CONNECTED, NULL, 0 );
											}
									}
							}
						
						break;
				}
		}
	return ;
}
#else

void CMyEpoll::RunThread( void )
{
	int count = 0;
	int socket_fd;
	int eventnum = 0;
	int loop = 0;
	struct epoll_event allEvents[EVENTMAX] = {0};
	MY_SOCKET_FD_CALLBACK pfun = NULL;
	CMyEpoll *pmyepoll = (CMyEpoll *)this;
	//��ѭ��
	//while(1)
	{
		if( CMyEpoll::m_exitflag )
		{
			//pthread_exit((void*)& "exit MyRun" );
			return ;
		}
		
		eventnum = 0;
		loop = 0;
		memset( allEvents, 0, sizeof(allEvents) );
		//�ȴ��¼�
		eventnum = epoll_wait (pmyepoll->m_epfd, allEvents, EVENTMAX, 0 );
		for( loop = 0; loop < eventnum; loop++ )
		{
			socket_fd = allEvents[loop].data.fd;
			pfun = pmyepoll->MyGetSocketFdCallBack( socket_fd );
			//���������¼�
			//���ӳɹ�
			if( allEvents[loop].events == EPOLLOUT )
			{
				struct epoll_event event = {0};
				event.data.fd = socket_fd;
				event.events = (EPOLLIN);
				epoll_ctl (pmyepoll->m_epfd, EPOLL_CTL_MOD, socket_fd, &event);
				if( pfun )
				{
					
					pfun( EVENT_CONNECTED, NULL, 0 );
					continue;
				}
			}
			//����ʧ��
			if( allEvents[loop].events == (EPOLLIN | EPOLLERR | EPOLLHUP) )
			{
				if( pfun )
				{
					pfun( EVENT_CONNECT_ERROR, NULL, 0 );
					pmyepoll->MyDelSocketFdEvent( socket_fd );
					continue;
				}
			}
			//��������
			if ( (allEvents[loop].events & EPOLLERR) || (allEvents[loop].events & EPOLLHUP) )
			{
				if( pfun )
				{
					pfun( EVENT_ERROR, NULL, 0 );
					pmyepoll->MyDelSocketFdEvent( socket_fd );
					continue;
				}
			}
			//ѭ����������
			if( allEvents[loop].events & EPOLLIN )
			{
				while( 1 )
				{
					count = recv( socket_fd, data, PROCESS_DATA_LEN, 0 );
					if( count < 0 )
					{
						if( errno != EAGAIN )
						{
							//֪ͨӦ��
							if( pfun )
							{
								pfun( EVENT_ERROR, NULL, 0 );
							}
							pmyepoll->MyDelSocketFdEvent( socket_fd );
						}
						break;
					}
					else if( count == 0 )
					{
						//֪ͨӦ��
						if( pfun )
						{
							pfun( EVENT_READ_ERROR, NULL, 0 );
						}
						pmyepoll->MyDelSocketFdEvent( socket_fd );
						break;
					}
					else
					{
						if( pfun )
						{
							pfun( EVENT_READ, data, count );
						}
						if( count < PROCESS_DATA_LEN )
						{
							break;
						}
					}
				}
			}
		}
	}
	return ;
}
#endif
