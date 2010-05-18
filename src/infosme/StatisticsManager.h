#ifndef SMSC_INFO_SME_STATISTICS_MANAGER
#define SMSC_INFO_SME_STATISTICS_MANAGER

#include "Statistics.h"

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/File.hpp>
#include <core/threads/Thread.hpp>

#include <db/DataSource.h>
#include <logger/Logger.h>
#include <memory>

#include "InfoSmeAdmin.h"
#include "core/buffers/File.hpp"

namespace smsc { namespace infosme
{
    using namespace smsc::db;
    using namespace core::threads;
    using namespace core::synchronization;
    
    using core::buffers::IntHash;
    using core::buffers::Hash;
    using core::buffers::File;
    
    using smsc::logger::Logger;
    
    class StatisticsManager : public Statistics, public Thread
    {
    protected:
    
        smsc::logger::Logger *logger;
        
        std::auto_ptr<IntHash<TaskStat> > statistics[2];

        std::string storeLocation;

        InfoSmeAdmin* admin;

        File currentFile;
        int currentHour;

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
        virtual void Start(int) { Start(); }
        virtual void Stop();

        virtual void flushStatistics();
        virtual bool getStatistics(uint32_t taskId, TaskStat& stat);
        virtual void delStatistics(uint32_t taskId);

        virtual void incGenerated(uint32_t taskId, unsigned inc=1);
        virtual void incDelivered(uint32_t taskId, unsigned inc=1);
        virtual void incRetried(uint32_t taskId, unsigned inc=1);
        virtual void incFailed(uint32_t taskId, unsigned inc=1);
        
        StatisticsManager(const std::string& argLocation,InfoSmeAdmin* argAdmin);
        virtual ~StatisticsManager();
    };

}}

#endif //SMSC_INFO_SME_STATISTICS_MANAGER
