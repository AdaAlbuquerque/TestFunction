#include "TcpServer.h"
#include <netinet/in.h>    // for sockaddr_in
#include <sys/types.h>    // for socket
#include <sys/socket.h>    // for socket
#include <stdlib.h>        // for exit
#include <string.h>        // for bzero

#define BACKLOG 10

CTcpServer::CTcpServer()
{
	m_port = -1;
	m_ServerFd = -1;
	m_brun = true;
	m_binit = false;
}

CTcpServer::~CTcpServer()
{}

int CTcpServer::Init(int port)
{
	if(port<=0)
	{
		NET_printf("Param Error, Port %d", port);
		return StatusType::Param_Error;
	}

	StatusType ret = StatusType::ALL_Failed;
	do
	{

		struct sockaddr_in server_addr;
	    bzero(&server_addr,sizeof(server_addr)); //把一段内存区的内容全部设置为0
	    server_addr.sin_family = AF_INET;
	    server_addr.sin_addr.s_addr = htons(INADDR_ANY);
	    server_addr.sin_port = htons(port);

	    //创建用于internet的流协议(TCP)socket,用server_socket代表服务器socket
	    m_ServerFd = socket(AF_INET,SOCK_STREAM,0);
	    if( m_ServerFd < 0)
	    {
	        NET_printf("Create Socket Failed!");
	        break;
	    }
	    else
	    {
	    	NET_printf("Create Socket %d", m_ServerFd)
	    }

		{
		   int opt =1;
		   setsockopt(m_ServerFd,SOL_SOCKET,SO_REUSEADDR,&opt,sizeof(opt));
		}
	     
	    //把socket和socket地址结构联系起来
	    if( bind(m_ServerFd,(struct sockaddr*)&server_addr,sizeof(server_addr)))
	    {
	        NET_printf("Server Bind Port : %d Failed!", port);
	        break;
	    }
	    else
	    {
	    	NET_printf("Bind Socket %d", m_ServerFd)
	    }
	 
	    //m_ServerFd用于监听
	    if ( listen(m_ServerFd, BACKLOG) == -1)
	    {
	        NET_printf("Server Listen Failed!");
	        break;
	    }
	    else
	    {
	    	NET_printf("Listen Socket %d", m_ServerFd)
	    }

	    m_ServerThread = ThreadPtr(new boost::thread(boost::bind(&CTcpServer::ServerThreadProc, this)));
	    if(m_ServerThread == NULL)
    	{
    		NET_printf("Create Server Thread Failed, Error %d", errno)
	    	break;
	    }

	    ret = StatusType::ALL_OK;
	}while(0);


	if(ret != StatusType::ALL_OK)
	{
		if(m_ServerFd != -1)
		{
			NET_printf("Close Server Socket Fd %d", m_ServerFd)
			close(m_ServerFd);
			m_ServerFd = -1;
		}
	}
	else
	{
		m_binit = true;
	}
	return ret;
}

int CTcpServer::Fini()
{
	if(m_ServerFd != -1)
	{
		close(m_ServerFd);
	}

	return StatusType::ALL_OK;
}

void CTcpServer::ServerThreadProc()
{
	fd_set readFd_set;
	struct timeval tv = {1, 0};
	char buffer[1024];
	while(m_brun)
	{
		if(m_ConnectionList.empty())
		{
			continue;
		}

		FD_ZERO(&readFd_set);
		int maxFd = -1;
		{
			boost::unique_lock<boost::mutex> lock(m_ConnectionListMutex);
			NET_printf("\nm_ConnectionList.size %d", (int)m_ConnectionList.size())
			for(std::list<ClientConnection>::iterator itr = m_ConnectionList.begin(); itr != m_ConnectionList.end(); )
			{
				NET_printf("m_ServerFd %d, m_ClientFd %d, m_bvalid %d\n", (*itr).m_ServerFd, (*itr).m_ClientFd, (*itr).m_bvalid)

				if( ((*itr).m_bvalid ) == false)
				{
					m_ConnectionList.erase(itr++);
					continue;
				}
				else
				{
					maxFd = ((*itr).m_ClientFd > maxFd)? (*itr).m_ClientFd : maxFd;
					FD_SET((*itr).m_ClientFd, &readFd_set);
					NET_printf("itr %p, maxFd %d", itr, maxFd);
					itr++;
				}


			}
		}
		if(m_ConnectionList.empty())
		{
			NET_printf("m_ConnectionList.empty(*)")
		}
		
		if(maxFd == -1)
		{
			continue;
		}

		int ret = select(maxFd+1, &readFd_set, NULL, NULL, &tv);
		if(ret == 0)
		{
			continue;
		}
		else if(ret < 0)
		{
			perror("Select ");
			continue;
		}
		{
			boost::unique_lock<boost::mutex> lock(m_ConnectionListMutex);
			for(auto itr = m_ConnectionList.begin(); itr != m_ConnectionList.end(); ++itr)
			{
				if(FD_ISSET((*itr).m_ClientFd, &readFd_set) )
				{
					memset(buffer, 0, sizeof(buffer));
					int nRecvRet = recv((*itr).m_ClientFd, buffer, sizeof(buffer), 0);
					if(nRecvRet == 0)
					{
						NET_printf("Clinet %d Send Zero Data, Close It", (*itr).m_ClientFd);
						close((*itr).m_ClientFd);
						(*itr).m_bvalid = false;
					}
					else if(nRecvRet > 0)
					{
						if(nRecvRet < (int)sizeof(buffer))
						{
							memset(&buffer[nRecvRet], '\0', 1);
							NET_printf("Get Data : %s\n", buffer);
						}
						else
						{
							NET_printf("Buffer Two Small, Get %d data : %s\n", nRecvRet, buffer)
						}
					}
					else
					{
						NET_printf("Recieve Data From %d Failed, Error %d\n", (*itr).m_ClientFd, errno);
						perror("Recieve Data Failed");
					}
				}
			}
		}
	}
}

int CTcpServer::Run()
{
	NET_printf("Enter Run Thread m_ServerFd %d", m_ServerFd)

	while (1) //服务器端要一直运行
    {

    	if(m_ServerFd < 0)
    	{
    		NET_printf("m_ServerFd %d < 0, m_binit %d", m_ServerFd, m_binit);
    		sleep(1);
    		continue;
    	}
        //定义客户端的socket地址结构client_addr
        struct sockaddr_in client_addr;
        socklen_t length = sizeof(client_addr);
 
        //接受一个到server_socket代表的socket的一个连接
        //如果没有连接请求,就等待到有连接请求--这是accept函数的特性
        //accept函数返回一个新的socket,这个socket(new_server_socket)用于同连接到的客户的通信
        //new_server_socket代表了服务器和客户端之间的一个通信通道
        //accept函数把连接到的客户端信息填写到客户端的socket地址结构client_addr中
        int new_server_socket = accept(m_ServerFd,(struct sockaddr*)&client_addr,&length);
        if ( new_server_socket < 0)
        {
            NET_printf("Server Accept Failed, Error %d\n", errno);
            perror("Server Accept Failed ");
            continue;
        }

        {
        	//ClientConnectionPtr s_ptr = ClientConnectionPtr(new ClientConnection(m_ServerFd, new_server_socket));
	        boost::unique_lock<boost::mutex> lock(m_ConnectionListMutex);
	        m_ConnectionList.push_back(ClientConnection(m_ServerFd, new_server_socket));
	    }
         /*
        char buffer[BUFFER_SIZE];
        bzero(buffer, BUFFER_SIZE);
        length = recv(new_server_socket,buffer,BUFFER_SIZE,0);
        if (length < 0)
        {
            NET_printf("Server Recieve Data Failed!\n");
            break;
        }

        //关闭与客户端的连接
        close(new_server_socket);
        */
    }

    return StatusType::ALL_OK;
}


ClientConnection::ClientConnection(int serverFd, int clientFd)
{
	m_ServerFd = serverFd;
	m_ClientFd = clientFd;
	m_bvalid = true;
}

ClientConnection::~ClientConnection()
{}
