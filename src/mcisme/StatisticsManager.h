#ifndef SMSC_MCI_SME_STATISTICS_MANAGER
#define SMSC_MCI_SME_STATISTICS_MANAGER

#include "Statistics.h"

#include <logger/Logger.h>
#include <db/DataSource.h>

#include <core/threads/Thread.hpp>

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>

namespace smsc { namespace mcisme
{
    using namespace smsc::db;
    using namespace core::threads;
    using namespace core::synchronization;
    
    using smsc::logger::Logger;
    
    class StatisticsManager : public Statistics, public Thread
    {
    protected:
    
        smsc::logger::Logger*   logger;
        Connection*             connection;
        
        EventsStat              statistics[2];

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
        virtual EventsStat getStatistics();

        virtual void incMissed   (unsigned inc=1);
        virtual void incDelivered(unsigned inc=1);
        virtual void incFailed   (unsigned inc=1);
        virtual void incNotified (unsigned inc=1);
        
        StatisticsManager(Connection* connection);
        virtual ~StatisticsManager();
    };

}}

#endif //SMSC_MCI_SME_STATISTICS_MANAGER
