/******************************************************************************
文件名: inetaddress.cpp
功能描述: 地址封装
******************************************************************************/
#include "PCInetAddress.h"

#if (__win__ == 1)
	#pragma comment(lib, "Ws2_32.lib")
#endif

// Constructions
PCInetAddress::PCInetAddress()
{
#if ( __ios__ == 1 )
    sin_len = sizeof(struct sockaddr_in);
#endif
	sin_family = AF_INET;
	sin_addr.s_addr = INADDR_ANY;
	sin_port = 0;
	memset(sin_zero, 0, 8);
}

// Constructions from ip and port
PCInetAddress::PCInetAddress(const char* ip, unsigned short port)
{
	sin_family = AF_INET;
	sin_addr.s_addr = inet_addr(ip);
	sin_port = htons(port);
	memset(sin_zero, 0, 8);
}

// Constructions from struct sockaddr
PCInetAddress::PCInetAddress(const struct sockaddr * addr)
{
	memcpy(&this->sin_family, addr, sizeof(struct sockaddr));
}

// Deconstructions
PCInetAddress::~PCInetAddress(void)
{
}

//Returns the raw IP address of this CInetAddress object. 
PCInetAddress::operator struct sockaddr*()
{
#if ( __ios__ != 1 )
	return (struct sockaddr *)(&this->sin_family);
#else
	return (struct sockaddr *)(&this->sin_len);
#endif
}

//Returns the raw IP address of this CInetAddress object. 
PCInetAddress::operator const struct sockaddr*() const
{
#if ( __ios__ != 1 )
	return (const struct sockaddr *)(&this->sin_family);
#else
	return (const struct sockaddr *)(&this->sin_len);
#endif
}

//Returns the IP address string in textual presentation form. 
const char* PCInetAddress::GetHostAddress() const
{
	static char addr[64];
#if (__win__ == 1 )
	sprintf_s(addr, 64, "%s:%u", inet_ntoa(sin_addr), GetPort());
#else
	snprintf(addr, 64, "%s:%u", inet_ntoa(sin_addr), GetPort());
#endif
	return addr;
}

const char* PCInetAddress::GetIP() const
{
	return inet_ntoa(sin_addr);
}

//Returns the port
unsigned short PCInetAddress::GetPort() const
{
	return ntohs(sin_port);
}

//Set IP
void PCInetAddress::SetIP(const char* ip)
{
	sin_addr.s_addr = inet_addr(ip);
}

//Set IP
void PCInetAddress::SetIP(unsigned int ip)
{
	sin_addr.s_addr = ip;
}

//Set port
void PCInetAddress::SetPorT(unsigned short port)
{
	sin_port = htons(port);
}

//Set domain
void PCInetAddress::SetHost(const char* name)
{
	hostent* h = gethostbyname(name);
	if(h != NULL)
	{
		sin_addr.s_addr = *((u_long *)h->h_addr_list[0]);
	}
}

//Get length of address
int PCInetAddress::GetLength()
{ 
	return sizeof(sockaddr_in); 
}

