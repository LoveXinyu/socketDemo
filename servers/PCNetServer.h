/******************************************************************************
文件名: CNetBean.h
功能描述: 客户端网络对象，一个网络对象对应一个SOCKET
******************************************************************************/
#ifndef __CCNET_NETBEAN_H__
#define __CCNET_NETBEAN_H__
#include "net.h"
#include "PCSocket.h"
#include <mutex>
#include <string>
#include <thread>
/*//定义流类型 */
//typedef PCStream STREAM;
/*/ / 连接超时时间(秒)*/
#define SOCK_CONNECT_TIMEOUT 10
/*//接收器缓冲区大小*/
#define SOCK_RECVBUFFERSIZE 4096

/******************************************************************************
类    名: PCNetServer
功能描述: 客户端网络对象，一个网络对象对应一个SOCKET
******************************************************************************/

NS_CCSERVER_BEGIN

struct PCData
{
    std::string _client_data;
    unsigned char* _server_data;
    ssize_t _data_len;
    int _retCode ;
    PCData():
    _client_data(""),
    _data_len(0),
    _retCode(0)
    {
        _server_data = NULL;
    };
    PCData(std::string& data):
    _client_data(data),
    _data_len(0),
    _retCode(0)
    {
        _server_data = NULL;
    };
};


class PCNetServer
{
public:
	/*///构造函数，初始化默认值*/
	PCNetServer();
	/*///析构函数，释放所有资源*/
	virtual ~PCNetServer();
public:
	/*//设置于服务器连接的地址与端口*/
    void setAddress(std::string ip, unsigned short port);
    
public:
	/*///开始无阻塞方式连接*/
	virtual bool connect();
	/*///是否处于连接状态*/
	virtual bool isConnected();
	/*///关闭连接*/
	virtual void close();
	/*///帧循环，读写数据*/
	static void drive(PCNetServer* server);
	/*///往服务端写数据，无阻塞*/
    virtual void write();
    bool writeRetry(PCData* data);

	/*///释放本网络对象*/
	virtual void release();

	/*///读数据*/
	void read();
    int readRetry(unsigned char* buff,int len);

public:
	/*///设置连接状态回调函数*/
	void setonRecMsgCallBack(std::function<void(int status,const std::string)> onResMsg){ _onRecMsg = onResMsg; }
	void sendMsgToLogic();
    
	virtual void pushMsg(std::string& msg);
    void addSendQueue(PCData* data);

protected:
	/*///枚举连接状态*/
	enum {
		ENULL			= 1,	/*//无状态*/
		EConnecting		= 2,	/*//正在进行连接*/
		EConnected		= 3,	/*//连接成功*/
		EConnectTimeout = 4,	/*//连接超时*/
		EConnectError	= 5,	/*//连接异常*/
		EDisconnected	= 6,		/*//连接中断*/
		EConnectedSuccess	= 7	,	/*//连接成功*/
        EWriteErro = 8,
        EWriteDown = 9,
	} 
	m_nConnectStatus;
    
    

protected:
	/*///接收缓冲器*/
	unsigned char m_RecvBuffer[SOCK_RECVBUFFERSIZE];
protected:
	time_t _connectStartTime, _connectTime;
protected:
	/*///服务器IP地址*/
    string m_nnAddress;
	/*///服务器端口号*/
	unsigned short m_nnPort;
    /*发送队列*/
    std::vector<PCData*> _send_queue;
    /*发送互斥锁*/
    mutex _send_lock;
    /*接收队列*/
    std::vector<PCData*> _rev_queue;
    /*接收互斥锁*/
    mutex _rev_lock;
    /*线程驱动*/
    thread* _driveThread;

protected:
	/*///IO操作*/
	PCSocket	m_Sock;
	int _retryTimes;
	std::function<void(int status,const std::string)> _onRecMsg;
};
NS_CCSERVER_END
#endif //__CCNET_NETBEAN_H__