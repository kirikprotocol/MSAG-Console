#ifndef SMSC_STAT_STATISTICS_MANAGER
#define SMSC_STAT_STATISTICS_MANAGER

#include "Statistics.h"

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/Hash.hpp>

#include <core/threads/ThreadedTask.hpp>

#include <db/DataSource.h>
#include <util/Logger.h>

namespace smsc { namespace stat 
{
    using namespace smsc::db;
    using namespace core::threads;
    using namespace core::synchronization;
    
    using core::buffers::IntHash;
    using core::buffers::Hash;
    
    using smsc::util::Logger;
    
    struct SmeStat
    {
        int sent; 
        int received;
        
        SmeStat() : sent(0), received(0) {};
        SmeStat(int s, int r) : sent(s), received(r) {};
        SmeStat(const SmeStat& stat) 
            : sent(stat.sent), received(stat.received) {};
        
        SmeStat& operator =(const SmeStat& stat) {
            sent = stat.sent; received = stat.received;
            return (*this);
        };
    };

    class StatisticsManager : public Statistics, public ThreadedTask
    {
    protected:
    
        log4cpp::Category       &log;
        DataSource              &ds;
        
        int     acceptedCount[2];
        int     finalizedCount[2];
        int     rescheduledCount[2];

        Hash<SmeStat>   statBySmeId[2];
        IntHash<int>    finalizedByError[2];
        Hash<int>       finalizedByRoute[2];
        
        short   currentIndex;
        
        Mutex   stopLock, switchLock, flushLock, smeStatLock;
        Mutex   incomingLock, outgoingLock, scheduleLock;

        Event   awakeEvent, exitEvent, doneEvent;
        bool    isStarted;
        
        short switchCounters();
        void  resetCounters(short index);
        void  flushCounters(short index);

        uint32_t calculatePeriod();
        int      calculateToSleep();

    public:
        
        virtual const char* taskName() { return "StatisticsTask"; };
        virtual int Execute();
        virtual void stop();

        virtual void flushStatistics();

        virtual void updateScheduled();
        virtual void updateAccepted(const char* srcSmeId);
        virtual void updateRejected(int errcode);
        virtual void updateTemporal(int errcode) {
            updateRejected(errcode);
        };
        virtual void updateChanged(const char* dstSmeId, 
            const char* routeId, int errcode = 0);
        
        StatisticsManager(DataSource& ds);
        virtual ~StatisticsManager();
    };

}}

#endif //SMSC_STAT_STATISTICS_MANAGER
