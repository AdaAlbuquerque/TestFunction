#include "BaseQueue.h"

const char* CBaseQueue::RootPath="/NodeTest";

CBaseQueue::CBaseQueue()
{
    m_zhandle = NULL;
    m_InitFlag = false;
    m_id = "0";
    m_bRun = false;
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

bool CBaseQueue::InitZookeeper()
{
    bool Ret = false;

    zoo_set_debug_level(ZOO_LOG_LEVEL_ERROR);
    do
    {
        //m_zhandle = zookeeper_init(host.c_str(), CBaseQueue::zoo_init_watch, 1000, NULL, NULL, 0);
        m_zhandle = zookeeper_init(m_host.c_str(), NULL, 1000, NULL, NULL, 0);

        if(NULL == m_zhandle)
        {
            NET_printf("Zookeeper Init Failed")
            break;
        }

        struct Stat RootPathStat;
        std::string watcherCtx("RootPath_Exists");
        std::string path(RootPath);
        
        int ret = zoo_exists(m_zhandle, path.c_str(), 0, &RootPathStat);

        if(ret == ZOK)
        {
            NET_printf("Node %s Exists", path.c_str())
        }
        else if(ret == ZNONODE)
        {
            NET_printf("Zoo_exists Return : ZNONODE")
            std::string RootPath_value("RootPath_value, U Can Use Json"); 
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
        
        Ret = true;
        m_bRun = true;
        NET_printf("Init Zookeeper %s Done", m_host.c_str());
    }while(0);

    return Ret;
}

bool CBaseQueue::Init()
{
    do
    {
        if(!InitZookeeper())
        {
            NET_printf("Init Zookeeper Failed !")
            break;
        }

        m_InitFlag = true;
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

bool CBaseQueue::Init(const std::string host, std::string port)
{
    if(host.empty() || std::atoi(port.c_str()) <= 0)
    {
        NET_printf("Host Empty Or Port empty, host:%s, port %s\n", host.c_str(), port.c_str());
        return false;
    }

    m_port = port;
    m_host = host;

    do
    {
        if(!InitZookeeper())
        {
            NET_printf("Init Zookeeper Failed !")
            break;
        }

        m_threadProc = CBaseQueueThread(new(std::nothrow) boost::thread(boost::bind(&CBaseQueue::threadProc, this)));
        
        if(m_threadProc == NULL)
        {
            NET_printf("Create Thread Proc Failed \n");
            break;
        }

        m_InitFlag = true;
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
    NET_printf("Enter Fini Proc")
    m_InitFlag = false;

	
	m_bRun = false;
    if(m_threadProc != NULL)
    {
        NET_printf("Fini Proc 0")
        m_threadProc->join();
    }
  

    if(m_zhandle != NULL)
    {
        NET_printf("Fini Proc 0")
        zookeeper_close(m_zhandle);
        m_zhandle = NULL;
    }

    NET_printf("Fini Done\n");

    return true;
}

bool CBaseQueue::FiniZookeeper(bool watcher)
{
    NET_printf("Enter Fini Proc")
    m_InitFlag = false;
	
    if(m_zhandle != NULL)
    {
        NET_printf("Fini Proc 0")
        zookeeper_close(m_zhandle);
        m_zhandle = NULL;
    }

    NET_printf("Fini Done\n");

    return true;
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
    else if(type == ZCONNECTIONLOSS || type == ZSYSTEMERROR)
    {
        NET_printf("From GetChildren Watch ZooHandle %p, Type: %d, ZCONNECTIONLOSS Or ZSYSTEMERROR", zh, type);
        if(bq->FiniZookeeper(true))
        {
            NET_printf("Fini Done");
        }
        else
        {
            NET_printf("Fini Failed")
        }

        if(bq->Init())
        {
            NET_printf("Init Done")
        }
        else
        {
            NET_printf("Init Failed ")
        }
        NET_printf("Reinit Zookeeper Done")
        return ;
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

    value_buffer[buflen]='\0';
    NET_printf("Get NodeValue %s", value_buffer);
    value = value_buffer;

    return true;
}

bool CBaseQueue::CheckOnline()
{
    bool OnlineRet = false;

    struct String_vector childNodes;
    do
    {
        char path_buffer[255];
        int buflen = sizeof(path_buffer);
        int retGetChild = zoo_wget_children(m_zhandle, RootPath, NULL, NULL, &childNodes);
        if(retGetChild != ZOK)
        {
            NET_printf("Get Node %s Children Failed, Error %d", RootPath, retGetChild);
            break;
        }

        for(int i = 0; i < childNodes.count; ++i)
        {
            if(childNodes.data[i] != NULL)
            {
                if(m_Seq == std::string(childNodes.data[i]))
                 {
                    OnlineRet = true;
                    break;
                }
            }
        }

    }while(0);

    deallocate_String_vector(&childNodes);

    return OnlineRet;
}

bool CBaseQueue::reblance(zhandle_t *zh)
{
   
   if(NULL == m_zhandle)
    {
        NET_printf("m_zhandle NULL, Reblance Failed");
        return true;
    }

    if(!CheckOnline())
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
        m_id = curNode.substr(curNode.find_last_of("/")+1);
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
            NET_printf("Base Queue %s:%p Is The Leader\n", m_port.c_str(), m_zhandle);
        }
        else
        {
            NET_printf("From Base Queue %s, %s Is The Leader\n", m_port.c_str(), leaderId.c_str());
        }

    }while(0);

    deallocate_String_vector(&childNodes);

    return true;
}

void CBaseQueue::Test()
{
    char path_buffer[255];

    std::string curPath("/NodeACLTest");
    std::string NodeValue("Node For ACL Test");
    struct ACL_vector acl_vector;
    acl_vector.count = 1;

    char scheme[10]="digest";
    char usr_pa[100]="test:V28q/NynI4JI3Rk54h0r8O5kMug=";

    struct ACL acl0;
    acl0.perms = ZOO_PERM_ALL;
    acl0.id.scheme = scheme;
    acl0.id.id = usr_pa;

    acl_vector.data = &acl0;

    int cretFlag = zoo_create(m_zhandle, curPath.c_str(), NodeValue.c_str(), NodeValue.size(), &acl_vector, 0, path_buffer, sizeof(path_buffer)-1);


}


void CBaseQueue::threadProc()
{
    while(m_bRun)
    {
        sleep(1);
    }
}

bool CBaseQueue::synMsgFromLeader()  //Only Copy MsgQueue For Test
{

}

void CBaseQueue::run()
{
	if(m_threadProc != NULL)
	{
		m_threadProc->join();
	}
}


