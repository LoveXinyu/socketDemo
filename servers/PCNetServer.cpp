/******************************************************************************
文件名: CNetBean.cpp
功能描述: 客户端网络对象，一个网络对象对应一个SOCKET
******************************************************************************/
#include "PCNetServer.h"
NS_CCSERVER_BEGIN

/*
//构造函数，初始化默认值
*/
PCNetServer::PCNetServer() :
m_nnPort(0),
m_nConnectStatus(ENULL),
_retryTimes(3),
_driveThread(NULL)
{


}

/*
//析构函数，释放所有资源
*/
PCNetServer::~PCNetServer(){}

/*
///设置于服务器连接的地址与端口
*/
void PCNetServer::setAddress(std::string ip, unsigned short port)
{
	this->m_nnPort = port;
    this->m_nnAddress = ip;
}
/*
///开始无阻塞方式连接
*/
bool PCNetServer::connect()
{
	//there is connected or connecting 
	if(this->m_nConnectStatus == EConnected || this->m_nConnectStatus == EConnecting){
		return false;
	}
	//validate value
	if(this->m_nnAddress == "" || this->m_nnPort == 0) {
		return false;
	}
	if(!m_Sock.Create()) {
		return false;
	}
	int times= 0 ;
	while(!m_Sock.Connect(this->m_nnAddress.c_str(), this->m_nnPort,5000)) {
		times++;
		if (times >= _retryTimes)
		{
			this->m_nConnectStatus = EConnectError;
            //链接失败
            fprintf(stdout, "connect %d time failed \n",times);
			return false;
		}
        fprintf(stderr, "connect %d times failed for %s\n",times,this->m_nnAddress.c_str());
			
	}
	//set the connecting status
	this->m_nConnectStatus = EConnected;
    fprintf(stdout,"connect success\n");
	//get the connect time of started.
	//call back to virtual function
    if(_driveThread == NULL)
    {
        _driveThread = new thread(PCNetServer::drive, this);
    }
	return true;
}
/*
///是否处于连接状态
*/
bool PCNetServer::isConnected()
{
	if(this->m_nConnectStatus == EConnected)
	{
        if(m_Sock.IsConnected() == 1){
            return true;
        }
        else{
            this->m_nConnectStatus = EConnectError;
            return false;
        }
	}
	return false;
}
/*
///关闭连接
*/
void PCNetServer::close()
{
	this->m_Sock.Close();
	this->m_nConnectStatus = EDisconnected;
}
/*
///释放本网络对象
*/
void PCNetServer::release()
{
	this->close();
}
/*
///添加消息到发送队列
*/
void PCNetServer::pushMsg(std::string& msg)
{
    PCData* data = new PCData(msg);
    data->_server_data = (unsigned char*)msg.c_str();
    data->_data_len = msg.length();
    addSendQueue(data);
}
void PCNetServer::addSendQueue(PCData* data)
{
    _send_lock.lock();
    _send_queue.push_back(data);
    _send_lock.unlock();
}
/*
///往服务端写数据，无阻塞
*/
void PCNetServer::write()
{
	if (this->m_nConnectStatus != EConnected) {
		return;
	}
	//check io is alive
	if (m_Sock.IsWritable()) {
		//pack length
        _send_lock.lock();
        while( _send_queue.size() > 0 ){
            PCData* send_data = _send_queue.front();
            if( writeRetry(send_data) ){
                fprintf(stdout, "send msg => %s",send_data->_client_data.c_str());
                _send_queue.erase(_send_queue.begin());
            }
            else{
                break;
            }
        }
        _send_lock.unlock();
		
	}
	else {
		//log
	}
}

bool PCNetServer::writeRetry(PCData* data)
{
    int nLen =0;
    while(nLen < data->_data_len)
    {
        int ret= m_Sock.Write(data->_server_data + nLen, (int)data->_data_len-nLen);
        if(ret <= 0)
        {
            //有可能网络出现故障
            fprintf(stderr, "send data to server <=0,尝试重新连接。");
            
            if(isConnected())
            {
                continue;
            }
            else if(connect())
            {
                return false;
            }
            return false;
        }
        nLen += ret;
    }
    return true;
}
/*
///读数据
*/
void PCNetServer::read()
{
	//connect successed
	if (this->m_nConnectStatus == EConnected){
		//Read Buffer
		if (m_Sock.IsReadable())
		{
            unsigned char readDataLen[4];
            if(readRetry(readDataLen, 4) == 4)
            {
                std::reverse(readDataLen, readDataLen + 4);
                int len = *(int *)(readDataLen);
                fprintf(stdout, "msg read len = %d",len);

                unsigned char* buff = (unsigned char*)malloc(sizeof(unsigned char*)*len);
                if(readRetry(buff, len) == len)
                {
                    PCData* data = new PCData();
                    data->_server_data = (unsigned char*)buff;
                    fprintf(stdout, "-----%s",data->_server_data);
                    data->_data_len = len;
                    _rev_lock.lock();
                    _rev_queue.push_back(data);
                    _rev_lock.unlock();
                }
                else{
                    return;
                }
            }
           
        }
        else{
            return;
        }
	}
}

int PCNetServer::readRetry(unsigned char* buff,int len)
{
    int nLen = m_Sock.Read(buff, len);
    //check error
    if (nLen == 0 || nLen == SOCKET_ERROR) {
        //change connecting status
        this->m_nConnectStatus = EDisconnected;
        //release socket
        this->close();
        //call back to virtual function
    }
    else {
        //read data
        if(nLen < len){
            return  nLen += readRetry(buff+nLen,len-nLen);
        }
        
    }
    return nLen;
}
/*
///帧循环，读写数据
*/
void PCNetServer::drive(PCNetServer* server)
{
    while(1)
    {
        fprintf(stdout,"drive\n");
        //sock was keeping connecting status
        if (server->m_nConnectStatus == EConnected) {
            //check connected status of unblock socket
            int nRet = server->isConnected();
            if (nRet) {
                //set the connecting status
                server->m_nConnectStatus = EConnected;
                //call back to virtual function
                server->read();
                server->write();
            } else if (!nRet) {	//error
                fprintf(stdout, "error connect failed \n");
                //set the connecting status
                server->m_nConnectStatus = EConnectError;
                //close socket
                server->connect();
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
void PCNetServer::sendMsgToLogic()
{

}

NS_CCSERVER_END