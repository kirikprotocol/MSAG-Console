#ifndef SMSC_INFO_SME_STATISTICS_MANAGER
#define SMSC_INFO_SME_STATISTICS_MANAGER

#include "Statistics.h"

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/Hash.hpp>

#include <core/threads/Thread.hpp>

#include <db/DataSource.h>
#include <logger/Logger.h>

namespace smsc { namespace infosme
{
    using namespace smsc::db;
    using namespace core::threads;
    using namespace core::synchronization;
    
    using core::buffers::IntHash;
    using core::buffers::Hash;
    
    using smsc::logger::Logger;
    
    class StatisticsManager : public Statistics, public Thread
    {
    protected:
    
        smsc::logger::Logger logger;
        Connection*         connection;
        
        Hash<TaskStat>      statistics[2];

        short   currentIndex;
        bool    bExternalFlush;
        
        Mutex   startLock, switchLock, flushLock;
        
        Event   awakeEvent, exitEvent, doneEvent;
        bool    bStarted, bNeedExit;
        
        short switchCounters();
        void  flushCounters(short index);

        uint32_t calculatePeriod();
        int      calculateToSleep();

    public:
        
        virtual int Execute();
        virtual void Start();
        virtual void Stop();

        virtual void flushStatistics();
        virtual bool getStatistics(std::string taskId, TaskStat& stat);
        virtual void delStatistics(std::string taskId);

        virtual void incGenerated(std::string taskId, unsigned inc=1);
        virtual void incDelivered(std::string taskId, unsigned inc=1);
        virtual void incRetried(std::string taskId, unsigned inc=1);
        virtual void incFailed(std::string taskId, unsigned inc=1);
        
        StatisticsManager(Connection* connection);
        virtual ~StatisticsManager();
    };

}}

#endif //SMSC_INFO_SME_STATISTICS_MANAGER
