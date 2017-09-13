/**************************************************************************************
*    < Copyright by Littlemeng >
*    File		  :	RPCServer.h
*	 Description  : Remote Process Call Server Interface
*	 System	      : 
*	 Module	      : 
*    Implement    : 
*--------------------------------------------------------------------------------------
*    Update Date	   Update Reason		         Ver.	   Author     
*    (2017/09/13)(Create                          )(0.1    )(mengnaihong) 
**************************************************************************************/
#ifndef _RPCSERVER_H
#define _RPCSERVER_H
#include <string>
#include "zmq.hpp"
#ifndef _WIN32
#include <unistd.h>
#else
#include <windows.h>
#define sleep(n)    Sleep(n)
#endif

template<class T>
class RPCServer
{
public:
	RPCServer(std::string sAddr) : m_socket(m_context, ZMQ_REP), m_sAddr(sAddr){ Bind(); }
	~RPCServer(){ UnBind(); }

	// 收到远程调用后的处理函数，重写该虚函数实现处理远程调用
	virtual bool OnCall(T param){ 
		//返回true表示已执行，反之未执行
		return true; 
	}

	// 服务器监听函数
	void Loop(){
		zmq::message_t request;
		m_socket.recv(&request);
		T param;
		int nSize = request.size();
		param.ParseFromArray(request.data(), nSize - 1);
		char* data = (char*)request.data();
		char cAsyn = *(data + nSize - 1);
		if (cAsyn)
		{
			// 回复client，1代表执行成功
			SendReply('1');
			OnCall(param);
		}
		else
		{
			bool bRet = OnCall(param);
			// 回复client，0代表执行失败
			char cRet = bRet ? '1' : '0';
			SendReply(cRet);
		}
	}

protected:
	void Bind(){ m_socket.bind(m_sAddr); }
	void UnBind(){ m_socket.unbind(m_sAddr); }
	void SendReply(char cReply){ zmq::message_t reply(1); memcpy(reply.data(), &cReply, 1); m_socket.send(reply); }

private:
	zmq::context_t m_context;
	zmq::socket_t m_socket;
	std::string m_sAddr;
};

#endif //_RPCSERVER_H