#include <ctime>
#include "logger/Logger.h"
#include "admin/util/SocketListener.h"
#include "admin/service/ServiceSocketListener.h"
#include "core/threads/ThreadedTask.hpp"

using namespace smsc::core::threads;
using namespace smsc::core::network;
using namespace smsc::logger;
using namespace smsc::admin::util;
using namespace smsc::admin::service;

struct PrivateDispatcher : public ThreadedTask
{
    PrivateDispatcher( Socket* s ) { delete s; }
    virtual const char* taskName() { return "pd"; }
    virtual int Execute() {
        return 0;
    }
    static void shutdown() {}
};

int main()
{
    Logger::initForTest(Logger::LEVEL_DEBUG);
    Logger* mainlog = Logger::getInstance("main");

    {
        smsc_log_debug(mainlog,"creating socket listener");
        ServiceSocketListener sl;
        sl.init("localhost",8886);

        smsc_log_debug(mainlog,"starting socket listener");
        sl.Start();
    
        smsc_log_debug(mainlog,"waiting until it starts");
        struct timespec wait = {1,0};
        nanosleep(&wait,0);
    
        smsc_log_debug(mainlog,"sending a shutdown");
        sl.shutdown();
        smsc_log_debug(mainlog,"exiting scope (dtor)");
    }
    smsc_log_debug(mainlog,"exiting main");
    return 0;
}
