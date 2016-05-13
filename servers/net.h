/******************************************************************************
文件名: net.h
功能描述: 整合了所有需要的头文件
******************************************************************************/
#ifndef __CCNET_NET_H__
#define __CCNET_NET_H__

#define NS_CCSERVER_BEGIN namespace playcrab {


#define NS_CCSERVER_END }

#define USING_NS_SERVER                    using namespace playcrab


#include "PCInetpredef.h"
#include "PCInetAddress.h"
#include "PCNetServer.h"
#include "PCSocket.h"


#endif //__CCNET_NET_H__