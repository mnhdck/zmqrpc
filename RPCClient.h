/**************************************************************************************
*    < Copyright by HAXC. Co. >
*    File		  :	RPC.h
*	 Description  : Remote Process Call Interface
*	 System	      : 
*	 Module	      : 
*    Implement    : 
*--------------------------------------------------------------------------------------
*    Update Date	   Update Reason		         Ver.	   Author     
*    (2017/09/13)(Create                          )(0.1    )(mengnaihong) 
**************************************************************************************/
#ifndef RPC_H
#define RPC_H

#include "zmq.hpp"
#include <string>

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

#endif //RPC_H