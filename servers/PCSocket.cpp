/******************************************************************************
文件名: PCSocket.cpp
功能描述: 套接字功能封装类的定义，采用select无阻塞模型
******************************************************************************/
#include "PCSocket.h"

#if( __android__ == 1 )
#define SO_NOSIGPIPE MSG_NOSIGNAL
#endif
/*///缺省构造函数*/
PCSocket::PCSocket()
{
	m_Socket = INVALID_SOCKET;
}

/*///析构函数*/
PCSocket::~PCSocket()
{
	if(m_Socket != INVALID_SOCKET)
	{
		Close();
	}
}

/*///创建IO对象*/
bool PCSocket::Create()
{
//choose socket version of win32
#if (__win__ == 1 )
	WSADATA wsaData;
	WSAStartup(MAKEWORD(2, 0), &wsaData);
#endif

	m_Socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if(m_Socket == INVALID_SOCKET)
	{
		/*//LOG_ERROR("创建套接字失败");*/
		return false;
	}

	/*//设置非阻塞模式*/
#if (__win__ == 1)
	unsigned long ul = 1;
	int nRet = ioctlsocket(m_Socket, FIONBIO, (unsigned long*)&ul);
	if (nRet == SOCKET_ERROR)
	{
		Close();
		/*//LOG_ERROR("设置非阻塞模式失败");*/
		return false;
	}
#else
    int nFlags = fcntl(m_Socket, F_GETFL, 0);
    int nRet = fcntl(m_Socket, F_SETFL, nFlags | O_NONBLOCK);
	if (nRet == SOCKET_ERROR)
	{
		Close();
		/*//LOG_ERROR("设置非阻塞模式失败");*/
		return false;
	}
#endif
	/*//设置套接字无延时*/
	int nNoDelay = 1;
	if(setsockopt (m_Socket , IPPROTO_TCP , TCP_NODELAY , (char *)&nNoDelay , sizeof(nNoDelay)) == SOCKET_ERROR)
	{
		Close();
		/*//LOG_ERROR("设置套接字无延时失败");*/
		return false;
	}

	return true;
}

/*///请求连接一个地址，无阻塞*/
bool PCSocket::Connect(const char* pIp, unsigned short uPort,int timeout)
{
	if(pIp == NULL)
	{
		return false;
	}
	else
	{
		return Connect(inet_addr(pIp), uPort,timeout);
	}
}

/*///请求连接一个地址，无阻塞*/
bool PCSocket::Connect(unsigned int uIp, unsigned short uPort,int timeout)
{
	if(m_Socket == INVALID_SOCKET)
	{
		return false;
	}

	PCInetAddress oAddr;
	oAddr.SetIP(uIp);
	oAddr.SetPorT(uPort);
    
	int nRet = connect(m_Socket, oAddr, oAddr.GetLength());
	if(nRet == 0)
	{
		return true;
	}
	else
	{
#if (__win__ == 1 )
		int nError = WSAGetLastError();
		if(nError ==  WSAEWOULDBLOCK)
		{
			return true;
		}
		else
		{
			return false;
		}
#else
        if(errno != EINPROGRESS){
            return false;
        }
        
        timeval tm;
        fd_set set;
        tm.tv_sec  = timeout;
        tm.tv_usec = 0;
        FD_ZERO(&set);
        FD_SET(m_Socket, &set);
        if(select(m_Socket +1, NULL, &set, NULL, &tm) > 0)
        {
            int error = -1;
            int len = sizeof(error);
            getsockopt(m_Socket, SOL_SOCKET, SO_ERROR, &error, (socklen_t *)&len);
            switch (error) {
                case 0:
                    return true;
                case ETIMEDOUT:
                    return false;
                default:
                    return false;
            }
        } else {
            return false;
        }
#endif
	}
    return true;
}

/*///绑定本地一个端口*/
bool PCSocket::Bind(const char* pIp, unsigned short uPort)
{
	return Bind(inet_addr(pIp), uPort);
}

/*///绑定本地一个端口*/
bool PCSocket::Bind(unsigned int uIp, unsigned short uPort)
{
	if(m_Socket == INVALID_SOCKET)
	{
		return false;
	}

	PCInetAddress oAddr;
	oAddr.SetIP(uIp);
	oAddr.SetPorT(uPort);

	return ::bind(m_Socket, oAddr, oAddr.GetLength()) == 0;
}

/*///开始监听（暂没有测试）*/
bool PCSocket::Listen()
{
	if(m_Socket == INVALID_SOCKET)
	{
		return false;
	}

	return listen(m_Socket, 5) == 0;
}

/*///读取数据，无阻塞*/
int PCSocket::Read(unsigned char* pBuffer, int nLen)
{
	if(m_Socket == INVALID_SOCKET)
	{
		return SOCKET_ERROR;
	}
#if ( __win__ == 1 )
	return recv(m_Socket, (char*)pBuffer, nLen, 0);
#else
	return recv(m_Socket, pBuffer, nLen, 0);
#endif
	
}

/*///写入数据，无阻塞*/
int PCSocket::Write(unsigned char* pBuffer, int nLen)
{
	if(m_Socket == INVALID_SOCKET)
	{
		return SOCKET_ERROR;
	}
	ssize_t sendSize = 0;
	while (true)
	{
#if ( __win__ == 1 )
		ssize_t size = send(m_Socket, (char*)pBuffer + sendSize, nLen - sendSize, 0);
#else
		ssize_t size = send(m_Socket, pBuffer, nLen, SO_NOSIGPIPE);
#endif
			if (size < 0 )
			{
				return -1;
			}
			else if (size + sendSize < nLen)
			{
				sendSize = sendSize + size;
			}
			else if (sendSize + size == nLen)
			{
				break;
			}
				

	}
	return nLen;
}

/*///主动断开连接*/
void PCSocket::Disconnect()
{
	if(m_Socket != INVALID_SOCKET)
	{
#if (__win__ == 1)
		shutdown(m_Socket, SD_BOTH);
#else
        shutdown(m_Socket, SHUT_RDWR);
#endif
	}
}

/*///关闭IO对象*/
void PCSocket::Close()
{
	if(m_Socket != INVALID_SOCKET)
	{
#if (__win__ == 1)
		closesocket(m_Socket);
#else
		close(m_Socket);
#endif
		m_Socket = INVALID_SOCKET;
	}
}

/*///检测是有数据可读*/
bool PCSocket::IsReadable()
{
	fd_set	fd;
	struct timeval tv;

	FD_ZERO(&fd);
	FD_SET(m_Socket, &fd);

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if(select((int)(m_Socket + 1), &fd, NULL, NULL, &tv) > 0)
	{
		if(FD_ISSET(m_Socket, &fd))
		{
			return true;
		}
	}

	return false;
}

/*///检测是否可以写入数据*/
bool PCSocket::IsWritable()
{
	fd_set	fd;
	struct timeval tv;

	FD_ZERO(&fd);
	FD_SET(m_Socket, &fd);

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if(select((int)(m_Socket + 1), NULL, &fd, NULL, &tv) >	0)
	{
		if(FD_ISSET(m_Socket, &fd))
		{
			return true;
		}
	}

	return false;
}

/*///检测是否已经建立连接*/
int PCSocket::IsConnected()
{
	fd_set	fd;
	struct timeval tv;

	FD_ZERO(&fd);
	FD_SET(m_Socket, &fd);

	tv.tv_sec = 0;
	tv.tv_usec = 0;

	if(select((int)(m_Socket + 1), NULL, &fd, NULL, &tv) > 0)
	{
		if(FD_ISSET(m_Socket, &fd))
		{
#if (__win__ == 1)
			return 1;
#else
			int error;
			socklen_t len = sizeof (error);
			if (getsockopt(m_Socket, SOL_SOCKET, SO_ERROR, &error, &len) < 0)
			{
				return -1;
			}
			//LINUX ECONNREFUSED 111
			//UNIX ECONNREFUSED 61
			if(error == ECONNREFUSED)
			{
				return -1;
			}
			return 1;
#endif
		}
	}
	return 0;
}

/*//检测是否接受到连接（暂没有测试）*/
bool PCSocket::IsAcceptable()
{
	return IsReadable();
}
