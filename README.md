# zmqrpc
RPC that use zmq+protobuf, contains sync call &amp; asyn call


Server Example code
////////////////////////////////////////////////
#include <iostream>
#include "RPCServer.h"
#include "hello.pb.h"


class MyServer :public RPCServer<tutorial::Person>
{
public:
	MyServer(std::string sAddr) :RPCServer(sAddr){}

	virtual bool OnCall(tutorial::Person param){ std::cout << "Recieve:" << param.name() << std::endl; return false; }
};

int main() 
{
	MyServer server("tcp://*:6666");
	std::cout << "Server listening..." << std::endl;

	while (true) {
		server.Loop();
		sleep(1);
	}

	return 0;
}

Client Example Code
/////////////////////////////////////////////////
#include "hello.pb.h"
#include "RPCClient.h"

int main ()
{

	tutorial::Person person;
	person.set_id(1);
	person.set_name("hehe");
	person.set_email("a@b.c");

	RPCClient<tutorial::Person> client("tcp://localhost:6666");
	bool bRet = client.CallSync(person);
	client.CallAsyn(person);
  return 0;
}
