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

    bool Fini();

    void printMsgQueue();

    void run();

private:

    //typedef boost::function<void (CBaseQueue, zhandle_t *zh, int type, int state, const char *path,void *watcherCtx)> zoo_watcher;

    bool reblance(zhandle_t *zh);

    bool synMsgFromLeader();  //Only Copy MsgQueue For Test

    bool Generate32UUID(std::string &uuid);

    bool GetNodeValue(const std::string path, std::string &value);

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

    bool            m_InitFlag;

    CBaseQueueThread m_threadProc;

};


#endif
























#include "BaseQueue.h"

const char* CBaseQueue::RootPath="/NodeTest";

CBaseQueue::CBaseQueue()
{
    m_zhandle = NULL;
    m_InitFlag = false;
    m_id = "0";
}

CBaseQueue::~CBaseQueue()
{
    Fini();
}

bool CBaseQueue::Generate32UUID(std::string &uuid)
{
    uuid_t uu;
    uuid_generate(uu);

    for(int i = 0; i < 16; ++i)
    {
        char szTemp[3]={0};
        sprintf(szTemp,"%02x",uu[i]);
        uuid += szTemp;
    }
    return true;
}

static void NodeExists(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx)
{
    NET_printf("From NodeExists , path %s", path)
}

bool CBaseQueue::Init(const std::string host, std::string port)
{
    if(host.empty() || std::atoi(port.c_str()) <= 0)
    {
        NET_printf("Host Empty Or Port empty, host:%s, port %s\n", host.c_str(), port.c_str());
        return false;
    }

    if(m_zhandle)
    {
        NET_printf("Base Queue Inited \n")
        return true;
    }

    Generate32UUID(m_id);
    m_port = port;

    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
    do
    {
        //m_zhandle = zookeeper_init(host.c_str(), CBaseQueue::zoo_init_watch, 1000, NULL, NULL, 0);
        m_zhandle = zookeeper_init(host.c_str(), NULL, 1000, NULL, NULL, 0);

        if(NULL == m_zhandle)
        {
            NET_printf("Zookeeper Init Failed")
            break;
        }

        struct Stat RootPathStat;
        std::string watcherCtx("RootPath_Exists");
        std::string path(RootPath);
        
        int ret = zoo_exists(m_zhandle, path.c_str(), 1, &RootPathStat);

        if(ret == ZOK)
        {
            NET_printf("Node %s Exists", path.c_str())
            
        }
        else if(ret == ZNONODE)
        {
            NET_printf("Zoo_exists Return : ZNONODE")
            std::string RootPath_value("RootPath_value, U Can Use Json"); //大小限制？
            char createPath[512];
            /*
            ZOOAPI int zoo_create(zhandle_t *zh, const char *path, const char *value,
            int valuelen, const struct ACL_vector *acl, int flags,
            char *path_buffer, int path_buffer_len);
            */
            
            int creat = zoo_create(m_zhandle, path.c_str(), RootPath_value.c_str(), RootPath_value.size(), &ZOO_OPEN_ACL_UNSAFE, 0, createPath, sizeof(createPath));
            if(creat != ZOK)
            {
                NET_printf("Create Node %s Failed , Error %d", path.c_str(), creat)
                break;
            }
            else
            {
                NET_printf("Node %s Created ", path.c_str())
            }

        }
        else if(ret == ZNOAUTH)
        {
            NET_printf("From Zoo_exists : ZNOAUTH")
            break;
        }
        else if(ret == ZBADARGUMENTS)
        {
            NET_printf("From Zoo_exists : ZBADARGUMENTS")
            break;
        }
        else if(ret == ZINVALIDSTATE)
        {
            NET_printf("From Zoo_exists : ZINVALIDSTATE")
            break;
        }
        else if(ret == ZMARSHALLINGERROR)
        {
            NET_printf("From Zoo_exists : ZMARSHALLINGERROR")
            break;
        }
        else
        {
            NET_printf("From Zoo_exists : Unknown %d", ret);
            break;
        }

        bool reb = reblance(m_zhandle);
        if(!reb)
        {
            NET_printf("Reblance Failed")
            break;
        }
        /*
        struct String_vector childNodes;
        int nGetChildrenRet = zoo_wget_children(m_zhandle, path.c_str(), &CBaseQueue::zoo_GetChildren_watch, this, &childNodes);

        deallocate_String_vector(&childNodes);
*/
        m_threadProc = CBaseQueueThread(new(std::nothrow) boost::thread(boost::bind(&CBaseQueue::threadProc, this)));
        if(m_threadProc == NULL)
        {
            NET_printf("Create Thread Proc Failed \n");
            break;
        }
        
        m_InitFlag = true;
        NET_printf("Init Done");
    }while(0);

    if(!m_InitFlag)
    {
        if(m_zhandle != NULL)
        {
            zookeeper_close(m_zhandle);
            m_zhandle = NULL;
        }
        return false;
    }

    NET_printf("Base Queue %s Inited Done \n", m_id.c_str());
    return true;
}

bool CBaseQueue::Fini()
{
    if(m_threadProc != NULL)
    {
        m_threadProc->join();
    }

    if(m_zhandle != NULL)
    {
        zookeeper_close(m_zhandle);
        m_zhandle = NULL;
    }
}

void CBaseQueue::zoo_init_watch(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx)
{

    if(type == ZOO_CREATED_EVENT )
    {
        NET_printf("From Init Watch ZooHandle %p, Type: ZOO_CREATED_EVENT", zh);
        
    }
    else if(type == ZOO_DELETED_EVENT)
    {
        NET_printf("From Init Watch ZooHandle %p, Type: ZOO_DELETED_EVENT", zh);
        
    }
    else if(type == ZOO_CHANGED_EVENT)
    {
        NET_printf("From Init Watch ZooHandle %p, Type: ZOO_CHANGED_EVENT", zh);
        
    }
    else if(type == ZOO_CHILD_EVENT)
    {
        NET_printf("From Init Watch ZooHandle %p, Type: ZOO_CHILD_EVENT, Reblance!", zh);
    }
    else if(type == ZOO_SESSION_EVENT)
    {
        NET_printf("From Init Watch ZooHandle %p, Type: ZOO_SESSION_EVENT", zh);
        
    }
    else if(type == ZOO_NOTWATCHING_EVENT)
    {
        NET_printf("From Init Watch ZooHandle %p, Type: ZOO_NOTWATCHING_EVENT", zh);
        
    }
    else
    {
        NET_printf("From Init Watch ZooHandle %p, Default type %d", zh, type);
    }

    if(state == ZOO_EXPIRED_SESSION_STATE )
    {
        NET_printf("From Init Watch ZooHandle State : ZOO_EXPIRED_SESSION_STATE");
        
    }
    if (state == ZOO_AUTH_FAILED_STATE)
    {
        NET_printf("From Init Watch ZooHandle State : ZOO_AUTH_FAILED_STATE");
        
    }
    if (state == ZOO_CONNECTING_STATE)
    {
        NET_printf("From Init Watch ZooHandle State : ZOO_CONNECTING_STATE");
        
    }
    if (state == ZOO_ASSOCIATING_STATE)
    {
        NET_printf("From Init Watch ZooHandle State : ZOO_ASSOCIATING_STATE");
        
    }
    if (state == ZOO_CONNECTED_STATE)
    {
        NET_printf("From Init Watch ZooHandle State : ZOO_CONNECTED_STATE");
        
    }

    if(path != NULL)
    {
        NET_printf("From Init Watch ZooHandle %p, Path %s", zh, path)
    }
    else
    {
        NET_printf("From Init Watch ZooHandle %p, Path Empty", zh)
    }

    if(watcherCtx != NULL)
    {
        NET_printf("From Init Watch ZooHandle %p, watcherCtx Not Empty", zh);
    }
    else
    {
        NET_printf("From Init Watch ZooHandle %p, watcherCtx Empty", zh);
    }

}

void CBaseQueue::zoo_GetChildren_watch(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx)
{
    CBaseQueue *bq = NULL;
    if(watcherCtx != NULL)
    {
        bq = static_cast<CBaseQueue *> (watcherCtx);
    }
    else
    {
        NET_printf("watcherCtx Null ");
        return;
    }
    
    if(type == ZOO_CHILD_EVENT)
    {
        NET_printf("From GetChildren Watch ZooHandle %p, Type: ZOO_CHILD_EVENT, Reblance!", zh);
        bq->reblance(zh);
    }
    else 
    {
        NET_printf("From GetChildren Watch ZooHandle %p, Type: %d", zh, type);
        return ;
    }

}

void CBaseQueue::zoo_RootPath_watch(zhandle_t *zh, int type, int state, const char *path,void *watcherCtx)
{

    if(type == ZOO_CREATED_EVENT )
    {
        NET_printf("From RootPath Watch ZooHandle %p, Type: ZOO_CREATED_EVENT", zh);
        
    }
    else if(type == ZOO_DELETED_EVENT)
    {
        NET_printf("From RootPath Watch ZooHandle %p, Type: ZOO_DELETED_EVENT", zh);
        
    }
    else if(type == ZOO_CHANGED_EVENT)
    {
        NET_printf("From RootPath Watch ZooHandle %p, Type: ZOO_CHANGED_EVENT", zh);
        
    }
    else if(type == ZOO_CHILD_EVENT)
    {
        NET_printf("From RootPath Watch ZooHandle %p, Type: ZOO_CHILD_EVENT", zh);
        
    }
    else 
    {
        NET_printf("From RootPath Watch ZooHandle %p, Type: %d", zh, type);
    }

    if(path != NULL)
    {
        NET_printf("From RootPath Watch ZooHandle %p, Path %s", zh, path)
    }
    else
    {
        NET_printf("From RootPath Watch ZooHandle %p, Path Empty", zh)
    }

    if(watcherCtx != NULL)
    {
        NET_printf("From RootPath Watch ZooHandle %p, watcherCtx Not Empty", zh);
    }
    else
    {
        NET_printf("From RootPath Watch ZooHandle %p, watcherCtx Empty", zh);
    }

}

bool CBaseQueue::GetNodeValue(const std::string path, std::string &value)
{
    if(NULL == m_zhandle || path.empty())
    {
        NET_printf("m_zhandle NULL Or Path Empty, GetNodeValue Failed");
        return false;
    }

    /*
    ZOOAPI int zoo_get(zhandle_t *zh, const char *path, int watch, char *buffer,   
                   int* buffer_len, struct Stat *stat);
    */
    char value_buffer[255];
    int buflen = sizeof(value_buffer);
    struct Stat stat;
    int retZooGet = zoo_get(m_zhandle, path.c_str(), 0, value_buffer, &buflen, &stat);
    if(retZooGet != ZOK)
    {
        NET_printf("zoo_get %s Failed, Error %d", path.c_str(), retZooGet);
        return false;
    }

    NET_printf("Get NodeValue %s", value_buffer);
    value = value_buffer;

    return true;
}

bool CBaseQueue::reblance(zhandle_t *zh)
{
   
   if(NULL == m_zhandle)
    {
        NET_printf("m_zhandle NULL, Reblance Failed");
        return true;
    }

    if(m_Seq.empty())
    {
        int cretFlag = 0;
        char path_buffer[255];
        std::string curPath(RootPath);
        curPath+="/";
        cretFlag = zoo_create(m_zhandle, curPath.c_str(), m_port.c_str(), m_port.size(), &ZOO_OPEN_ACL_UNSAFE, ZOO_EPHEMERAL|ZOO_SEQUENCE, path_buffer, sizeof(path_buffer));
        if(cretFlag != ZOK)
        {
            NET_printf("Create ZOO_EPHEMERAL|ZOO_SEQUENCE Node Failed, Error Code %d\n", cretFlag);
            return false;
        }

        NET_printf("Create ZOO_EPHEMERAL|ZOO_SEQUENCE Node Done Path : %s", path_buffer);
        std::string curNode(path_buffer);

        m_Seq = curNode.substr(curNode.find_last_of("/")+1);
    }

    int seq = std::atoi(m_Seq.c_str());

    struct String_vector childNodes;
    do
    {
        char path_buffer[255];
        int buffer_len = sizeof(path_buffer);
        int retGetChild = zoo_wget_children(m_zhandle, RootPath, &CBaseQueue::zoo_GetChildren_watch, this, &childNodes);
        //int retGetChild = zoo_wget_children(m_zhandle, RootPath, NULL, NULL, &childNodes);
        if(retGetChild != ZOK)
        {
            NET_printf("Get %s childNodes Failed , Error %d", RootPath, retGetChild);
            break;
        }

        //NET_printf("Get Children : %s", childNodes.data[0]);

        int min = seq;
        std::string leaderId;
        for(int i = 0; i < childNodes.count; ++i)
        {
            if(childNodes.data[i] != NULL)
            {
                int nodeSeq = std::atoi(childNodes.data[i]);
                if(nodeSeq < min)
                {
                    min = nodeSeq;
                    std::string path(RootPath);
                    path+="/";
                    path+=childNodes.data[i];

                    if(!GetNodeValue(path, leaderId))
                    {
                        NET_printf("Get Node Value Failed, May Lose Leader Info");
                    }
                }
            }
        }

        if(min == seq)
        {
            NET_printf("Base Queue %s:%p Is The Leader", m_port.c_str(), m_zhandle);
        }
        else
        {
            NET_printf("From Base Queue %s, %s Is The Leader", m_port.c_str(), leaderId.c_str());
        }

    }while(0);

    deallocate_String_vector(&childNodes);

    return true;
}

void CBaseQueue::threadProc()
{
    while(1)
    {
        sleep(1);
    }
}

void CBaseQueue::run()
{

}

bool CBaseQueue::synMsgFromLeader()  //Only Copy MsgQueue For Test
{

}






















#include <iostream>
#include "boost_test.h"

#ifdef MACRO_TEST

#include <stdio.h>
#include <sys/syscall.h> 
#define gettid() syscall(__NR_gettid)  

#ifndef NET_printf
    #define NET_printf(fmt, args...) printf("%s [0x%x] [%d] %s : " fmt "\n", __FILE__, gettid(), __LINE__, __FUNCTION__,  ##args);
#endif
#endif

#ifdef __Zookeeper_Test__

#include "BaseQueue.h"

void test1()
{
    CBaseQueue baseNode;
    baseNode.Init("localhost:2181", "10000");

    baseNode.run();
}

void multiThreadTest()
{
    int nTimes = 1000;
    int nn = nTimes;

    std::vector<boost::shared_ptr<boost::thread>> threads;

    do
    {
        boost::shared_ptr<boost::thread> s_ptr = boost::shared_ptr<boost::thread> (new boost::thread(test1));
        if(s_ptr != NULL)
        {
            threads.push_back(s_ptr);
            NET_printf("Create %dth Thread", nn-nTimes);
        }

    }while(--nTimes);

    for(auto itr = threads.begin(); itr != threads.end(); ++itr)
    {
        (*itr)->join();
    }

}

void testZookeeper(std::string port)
{
    CBaseQueue baseNode;
    baseNode.Init("localhost:2181", port);

    baseNode.run();
}

#endif


int main(int argc, char **argv)
{

#ifdef __ARGC__

    for(int i = 0; i < argc; ++i)
    {
        NET_printf("agrv [%d] : %s", i, argv[i]);
    }

    int opt = -1;
    zhandle_t *zt = NULL;
    std::string host;
    std::string nodePath;

    bool d = false;
    bool l = false;
    bool t = false;
    while((opt = getopt(argc,argv,"z:d:l:t")) != -1)
    {
        switch(opt)
        {
            case 'z':
                host = optarg;
                break;
            case 'd':
                nodePath = optarg;
                d = true;
                break;
            case 'l':
                nodePath = optarg;
                l = true;
                break;
            case 't':
                nodePath = optarg;
                t = true;
            default:
                goto usage;
        }
    }

    if(optind != argc || host.size()== 0 || /*(!d && !l) ||*/ host.find(":") == (std::string::size_type)-1)
    {
        usage:
        fprintf(stderr, 
            "\nUsage:\n    zooTool -z <host:port> [-d <node> | -l <node>]\n\n"
            "Example (Check Broker Nodes):\n    ./zooTool -z localhost:2181 -l /brokers/ids\n\n"
            "Tips:\n"
            "    Broker Nodes Path: /brokers/ids\n"
            "    Topic Nodes Path: /brokers/topics\n"
            "    Magic Nodes Path: /\n\n");
        exit(0);
    }

#endif

#ifdef __Zookeeper_Test__

    if(argc > 1)
    {
        testZookeeper(std::string(argv[1]));
    }
    else
    {
        NET_printf("argc < 1, testZookeeper Start Failed")
    }
#endif
}



