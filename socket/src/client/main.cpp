
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>

#include <arpa/inet.h>
#include <sys/ioctl.h>

#include <string>
#include <list>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>

#ifndef NET_printf
	#define NET_printf(fmt, args...) printf("%s %s [%d] " fmt " \n", __FILE__, __FUNCTION__, __LINE__, ##args);
#endif

enum TestReturn
{
	All_Ok = 0,
	All_Failed = 1,
	Param_Error = 2
};

int onlineTest(std::string hostInfo, int timeOut = 3000)
{

	//参数合法性
	if(hostInfo.empty() || hostInfo.find(":")==std::string::npos || timeOut < 0)
	{
		NET_printf("Param Error, HostInfo %s, timeOut %d", hostInfo.c_str(), timeOut);
		return TestReturn::Param_Error;
	}

	std::string host = hostInfo.substr(0,hostInfo.find(":"));
	int port = std::atoi(hostInfo.substr(hostInfo.find(":")+1).c_str());

	if(host.empty() || port <= 0)
	{
		NET_printf("Param Error, Host %s, port %d", hostInfo.c_str(), port);
		return TestReturn::Param_Error;
	}

	TestReturn ret = TestReturn::All_Failed;
	int count = 0;
	do
	{
		//Init Socket	
		do 
		{
			int m_sockfd = 0;
			struct sockaddr_in client_addr;
		    bzero(&client_addr,sizeof(client_addr)); //把一段内存区的内容全部设置为0
		    client_addr.sin_family = AF_INET;    //internet协议族
		    client_addr.sin_addr.s_addr = htons(INADDR_ANY);//INADDR_ANY表示自动获取本机地址
		    client_addr.sin_port = htons(0);    //0表示让系统自动分配一个空闲端口
		    //创建用于internet的流协议(TCP)socket,用client_socket代表客户机socket

		    m_sockfd = socket(AF_INET, SOCK_STREAM, 0);
		    if(m_sockfd <= 0)
		    {
		        NET_printf("Create Error: Create Socket Failed ,%d\n", m_sockfd);
		        break;
		    }

		    if(bind(m_sockfd, (struct sockaddr*)&client_addr, sizeof(client_addr)))
		    {
		        NET_printf("Operator Error: Bind Socket [%d] Failed!\n", m_sockfd);
		        break;
		    }

		    struct sockaddr_in server_addr;
		    bzero((void *)&server_addr, sizeof(server_addr));
		    server_addr.sin_family = AF_INET;
		    server_addr.sin_port = htons(port);
		    if(inet_aton(host.c_str(), &server_addr.sin_addr) == 0)
		    {
		        NET_printf("Server IP Address Error\n");
		        break;
		    }

		    socklen_t server_addr_length = sizeof(server_addr);

		    if(connect(m_sockfd, (struct sockaddr*)&server_addr, server_addr_length) < 0)
		    {
		        NET_printf("Connect To Host [%s] Failed, Error %d\n", host.c_str(), errno);
		        perror("onnect To Host Failed");
		        break;
		    }

		    ret = TestReturn::All_Ok;
		    if(m_sockfd > 0)
		    {
			    close(m_sockfd);
			}

		}while(0);

		if(ret == TestReturn::All_Ok)
		{
			//NET_printf("Host %s Online", hostInfo.c_str())
			if(++count % 100 == 0)
			{
				NET_printf("Host %s Online", hostInfo.c_str())
			}
		}
		else
		{
			NET_printf("Host %s Offline, Error %d\n", hostInfo.c_str(), errno);
			perror("Host Offline ");
		}

		sleep(5);
	} while(1);

	return ret;
}

void multiThread(std::string hostInfo, int timeOut = 3000, int nThread=20)
{
	std::list<boost::shared_ptr<boost::thread>> threadList;

	for(int i = 0; i < nThread; ++i)
	{
		boost::shared_ptr<boost::thread> p1 = boost::shared_ptr<boost::thread>(new boost::thread(onlineTest, hostInfo, timeOut));
		threadList.push_back(p1);
	}

	for(auto itr = threadList.begin(); itr != threadList.end(); ++itr)
	{
		(*itr)->join();
	}
}

int main(int argc, char **argv)
{
	NET_printf("Online Test Start ...\n")
	if(argc < 2)
	{
		NET_printf("argc < 2");	
	}
	else if(argc == 2)
	{
		//onlineTest(std::string(argv[1]));
		multiThread(std::string(argv[1]));
	}
	else if(argc == 3)
	{
		//onlineTest(std::string(argv[1]), std::atoi(argv[2]));
		multiThread(std::string(argv[1]), std::atoi(argv[2]));
	}
	else
	{
		multiThread(std::string(argv[1]), std::atoi(argv[2]), std::atoi(argv[3]));
	}
}