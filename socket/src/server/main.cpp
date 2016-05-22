#include "TcpServer.h"
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>

#ifndef NET_printf
	#define NET_printf(fmt, args...) printf("%s %s [%d] " fmt " \n", __FILE__, __FUNCTION__, __LINE__, ##args);
#endif

void Server(int port)
{
	boost::shared_ptr<CTcpServer > tcpServer = boost::shared_ptr<CTcpServer>(new CTcpServer());
	if(NULL == tcpServer)
	{
		NET_printf("Create CTcpServer Failed !!")
		return ;
	}
	do
	{
		if(tcpServer->Init(port) == StatusType::ALL_OK)
		{
			NET_printf("Init Server On Port %d", port)
		}
		else
		{
			NET_printf("Init Server On Port %d Failed", port)
			break;
		}

		tcpServer->Run();

	}while(0);

	NET_printf("Server End \n")
	tcpServer->Fini();
}

int main(int argc, char **argv)
{
	if(argc < 2)
	{
		NET_printf("Param Empty !");
	}
	else if(argc == 2)
	{
		Server(std::atoi(argv[1]));
	}
}