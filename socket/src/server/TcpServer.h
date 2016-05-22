#ifndef __TCPSERVER_H__
#define __TCPSERVER_H__

#include <list>
#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>

#ifndef NET_printf
	#define NET_printf(fmt, args...) printf("%s %s [%d] " fmt " \n", __FILE__, __FUNCTION__, __LINE__, ##args);
#endif

enum StatusType
{
	ALL_OK = 0,
	ALL_Failed = 1,
	Param_Error = 2
};

class ClientConnection;

class CTcpServer
{
public:

	CTcpServer();

	~CTcpServer();

	int Init(int port);

	int Fini();

	int Run();

	void ServerThreadProc();

private:

	int InitSocket(int port);

private:

	typedef boost::shared_ptr<ClientConnection> 	ClientConnectionPtr;

	typedef boost::shared_ptr<boost::thread> 		ThreadPtr;

	 int 		m_port;
	 int 		m_ServerFd;

	 bool 		m_brun;

	 bool		m_binit;

	 ThreadPtr 	m_ServerThread;

	 boost::mutex 	m_ConnectionListMutex;
	 std::list<ClientConnection> m_ConnectionList; 
};

class ClientConnection
{

public:

	ClientConnection(int serverFd, int clientFd);
	~ClientConnection();

	int closeConnection();

//private:

	int 		m_ServerFd;
	int 		m_ClientFd;

	bool 		m_bvalid;

};


#endif