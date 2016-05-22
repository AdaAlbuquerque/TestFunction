#ifndef __CBaseQueue_H__
#define __CBaseQueue_H__

#include <boost/smart_ptr.hpp>
#include <boost/thread.hpp>
#include <uuid/uuid.h>
#include <boost/function.hpp>
#include <zookeeper/zookeeper.h>

#include <sys/syscall.h> 
#define gettid() syscall(__NR_gettid)
#ifndef NET_printf
    #define NET_printf(fmt, args...) printf("%s [0x%x] [%d] %s : " fmt "\n", __FILE__, gettid(), __LINE__, __FUNCTION__,  ##args);
#endif

class CBaseQueue;

struct follower
{
    typedef boost::shared_ptr<CBaseQueue> CBaseQueue_ptr;
    CBaseQueue_ptr               Leader;
    std::list<CBaseQueue_ptr>    Followers;
};

struct Msg
{
    int iMsgNum;
};

class CBaseQueue
{

public:

    CBaseQueue();

    ~CBaseQueue();

    bool Init(const std::string zookeeperHost, std::string serverPort);

    bool Init();

    bool InitZookeeper();

	bool FiniZookeeper(bool watcher);
	
    bool Fini();

    void printMsgQueue();

    void Test();

    void run();

private:

    //typedef boost::function<void (CBaseQueue, zhandle_t *zh, int type, int state, const char *path,void *watcherCtx)> zoo_watcher;

    bool reblance(zhandle_t *zh);

    bool synMsgFromLeader();  //Only Copy MsgQueue For Test

    bool Generate32UUID(std::string &uuid);

    bool GetNodeValue(const std::string path, std::string &value);

    bool CheckOnline();

    static void zoo_init_watch(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx);
    static void zoo_GetChildren_watch(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx);
    static void zoo_RootPath_watch(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx);

    void threadProc();

private:

    static const char * RootPath;

    typedef boost::shared_ptr<CBaseQueue> CBaseQueue_ptr;

    typedef boost::shared_ptr<boost::thread> CBaseQueueThread;

    zhandle_t       *m_zhandle;

    CBaseQueue_ptr   m_leader;

    std::list<Msg>  m_msgQueue;

    std::string     m_Seq;

    std::string     m_id;

    std::string     m_port;

    std::string 	m_host;

    bool 			m_bRun;

    bool            m_InitFlag;

    CBaseQueueThread m_threadProc;

};


#endif


