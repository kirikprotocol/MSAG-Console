#include <sys/types.h>
#include <netinet/in.h>

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>  

#include <memory>
#include <vector>
#include <string>

#include <scag/util/singleton/Singleton.h>
#include "tstStatisticsManager.h"

namespace scag { 
namespace statx {

using namespace scag::stat;

bool  StatisticsManager::inited = false;
using namespace scag::util::singleton;
using scag::config::StatManConfig;


typedef SingletonHolder<StatisticsManager> SingleSM;

Statistics& Statistics::Instance()
{
    if (!StatisticsManager::inited) 
    {
        MutexGuard guard(StatisticsManager::initLock);
        if (!StatisticsManager::inited) 
            throw std::runtime_error("Statistics not inited!");
    }
    return SingleSM::Instance();
}
void StatisticsManager::init(const StatManConfig& statManConfig)
{
    if (!StatisticsManager::inited)
    {
        MutexGuard guard(StatisticsManager::initLock);
        if (!StatisticsManager::inited) {
            StatisticsManager& sm = SingleSM::Instance();

            sm.Start();
            StatisticsManager::inited = true;
        }
    }
}



StatisticsManager::StatisticsManager()
    : Statistics(), Thread(), 
      logger(Logger::getInstance("scag.stat.StatisticsManager")),
      currentIndex(0), bExternalFlush(false), isStarted(false) 
{
}
StatisticsManager::~StatisticsManager()
{
  Stop();
  WaitFor();

}
 
void StatisticsManager::incError(IntHash<int>& hash, int errcode){}

void StatisticsManager::registerEvent(const SmppStatEvent& se){}

void StatisticsManager::registerEvent(const HttpStatEvent& se){}
    
bool StatisticsManager::checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value)
{
      
    return false;
}

int StatisticsManager::Execute()
{

    
    isStarted = true;     

    while (isStarted)
    {
        
        smsc_log_debug(logger, "Execute() >> Start wait %d", toSleep);
        awakeEvent.Wait(100); // Wait for next hour begins ...
        smsc_log_debug(logger, "Execute() >> End wait");


        if(bExternalFlush) isStarted = false;
        bExternalFlush = false;
        smsc_log_debug(logger, "Execute() >> Flushed");
    }

    smsc_log_debug(logger, "Execute() exited");
    return 0;
}

void StatisticsManager::Stop()
{

    //smsc_log_debug(logger, "PerformanceServer is shutdowninig...");
    //sender.Stop();
    smsc_log_debug(logger, "PerformanceServer is shutdowned");

    if (isStarted)
    {
        bExternalFlush = true;
        awakeEvent.Signal();
    }
}

void StatisticsManager::Start()
{
    
    //smsc_log_debug(logger, "PerformanceServer is starting...");
    //sender.Start();
    smsc_log_debug(logger, "PerformanceServer is started");

    isStarted = true;
    Thread::Start();
}




void StatisticsManager::reportGenPerformance(PerformanceData * data){}
void StatisticsManager::getSmppPerfData(uint64_t *cnt){}
void StatisticsManager::reportSvcPerformance(){}
void StatisticsManager::reportScPerformance(){}
void StatisticsManager::addSvcSocket(Socket * socket){}
void StatisticsManager::addScSocket(Socket * socket){}
void StatisticsManager::addGenSocket(Socket * socket){}



}//namespace statx
}//namespace scag
