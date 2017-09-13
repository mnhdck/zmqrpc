/**************************************************************************************
*    < Copyright by Littlemeng. Co. >
*    File		  :	RPCClient.h
*	 Description  : Remote Process Call Client Interface
*	 System	      : 
*	 Module	      : 
*    Implement    : 
*--------------------------------------------------------------------------------------
*    Update Date	   Update Reason		         Ver.	   Author     
*    (2017/09/13)(Create                          )(0.1    )(mengnaihong) 
**************************************************************************************/
#ifndef _RPCCLIENT_H
#define _RPCCLIENT_H
#include <string>
#include "zmq.hpp"

template<class T>
class RPCClient
{
public:
	RPCClient(std::string sAddr) :m_socket(m_context, ZMQ_REQ), m_sAddr(sAddr){  }
	~RPCClient(){ DisConnect(); }

	//异步调用，直接返回
	void CallAsyn(T& obj){ SendPrivate(obj, true); zmq::message_t reply; m_socket.recv(&reply); }

	//同步调用，服务器处理完才返回
	bool CallSync(T& obj){
		SendPrivate(obj, false); 
		zmq::message_t reply;
		m_socket.recv(&reply);
		std::string sRet = ((char*)reply.data());
		return !(sRet.at(0) == '0');
	}

protected:
	void Connect(){ m_socket.connect(m_sAddr); }

	void DisConnect(){ if (m_socket.connected()) m_socket.disconnect(m_sAddr); }

	void SendPrivate(T& obj, bool bAsyn = 1){
		Connect();
		int nSize = obj.ByteSize();
		zmq::message_t request(nSize + 1);
		void* pArray = malloc(nSize + 1);
		obj.SerializeToArray(pArray, nSize);
		//最后一位作为标志，0为同步，1为异步
		*((char*)pArray + nSize) = bAsyn ? 1 : 0;
		memcpy(request.data(), pArray, nSize + 1);

		m_socket.send(request);
		free(pArray);
	}

private:
	zmq::context_t m_context;
	zmq::socket_t m_socket;
	std::string m_sAddr;
};

#endif //_RPCCLIENT_H