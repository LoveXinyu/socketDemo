/******************************************************************************
文件名: inetaddress.h
功能描述: 地址封装
******************************************************************************/
#ifndef __CCNET_INETADDRESS_H__
#define __CCNET_INETADDRESS_H__

#include "PCInetpredef.h"

/******************************************************************************
类    名: CInetAddress
功能描述: 套接字功能封装类的定义，采用select无阻塞模型
******************************************************************************/
class PCInetAddress : public sockaddr_in
{
public:
	// Constructions
	PCInetAddress();
	// Constructions from ip and port
	PCInetAddress(const char* ip, unsigned short port);
	// Constructions from struct sockaddr
	PCInetAddress(const struct sockaddr * addr);
	// Deconstructions
	virtual ~PCInetAddress(void);

public:
	//Returns the raw IP address of this CInetAddress object. 
	operator struct sockaddr*();
	//Returns the raw IP address of this CInetAddress object. 
	operator const struct sockaddr*() const;
	//Returns the IP address and port string in textual presentation form. ex 192.168.1.201:8888
	const char* GetHostAddress() const;
	//Returns the IP in textual presentation form.
	const char* GetIP() const;
	//Returns the port
	unsigned short GetPort() const;
	//Set IP
	void SetIP(const char* ip);
	//Set IP
	void SetIP(unsigned int ip);
	//Set port
	void SetPorT(unsigned short port);
	//Set host
	void SetHost(const char* name);

public:
	//Get length of address
	int GetLength();
};


#endif // __CCNET_INETADDRESS_H__
