#include <iostream>

#define __Zookeeper_Test__
#define MACRO_TEST

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

    //baseNode.run();
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

    //baseNode.Test();

	baseNode.run();
}

#endif


int main(int argc, char **argv)
{

NET_printf("Test Start ...\n");
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



