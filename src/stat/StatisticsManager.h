#ifndef SMSC_STAT_STATISTICS_MANAGER
#define SMSC_STAT_STATISTICS_MANAGER

#include "Statistics.h"

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/Hash.hpp>

#include <core/threads/ThreadedTask.hpp>

#include <db/DataSource.h>
#include <logger/Logger.h>

namespace smsc { namespace stat 
{
    using namespace smsc::db;
    using namespace core::threads;
    using namespace core::synchronization;
    
    using core::buffers::IntHash;
    using core::buffers::Hash;
    
    using smsc::util::Logger;
    
    struct SmsStat
    {
        int accepted, rejected;                       
        int delivered, failed, rescheduled, temporal;
        IntHash<int> errors;
        
        SmsStat(int accepted = 0, int rejected = 0, 
                int delivered = 0, int failed = 0, 
                int recheduled = 0, int temporal = 0) 
            : accepted(accepted), rejected(rejected),
              delivered(delivered), failed(failed),
              rescheduled(rescheduled), temporal(temporal) 
            { errors.Empty(); };
        SmsStat(const SmsStat& stat) 
            : accepted(stat.accepted), rejected(stat.rejected), 
              delivered(stat.delivered), failed(stat.failed),
              rescheduled(stat.rescheduled), temporal(stat.temporal), 
              errors(stat.errors) {};
        
        SmsStat& operator =(const SmsStat& stat) {
            accepted = stat.accepted; rejected = stat.rejected;
            delivered = stat.delivered; failed = stat.failed;
            rescheduled = stat.rescheduled; temporal = stat.temporal;
            errors = stat.errors;
            return (*this);
        };

        inline void Empty() {
            accepted = 0; rejected = 0; delivered = 0; 
            failed = 0; rescheduled = 0; temporal = 0;
            errors.Empty();
        };
    };

    class StatisticsManager : public Statistics, public ThreadedTask
    {
    protected:
    
        log4cpp::Category       &logger;
        DataSource              &ds;
        
        SmsStat         statGeneral[2];
        Hash<SmsStat>   statBySmeId[2];
        Hash<SmsStat>   statByRoute[2];
        
        short   currentIndex;
        bool    bExternalFlush;
        
        Mutex   stopLock, switchLock, flushLock;
        Event   awakeEvent, exitEvent, doneEvent;
        bool    isStarted;
        
        short switchCounters();
        void  resetCounters(short index);
        void  flushCounters(short index);
        
        uint32_t calculatePeriod();
        int      calculateToSleep();

        void  addError(IntHash<int>& hash, int errcode);

    public:
        
        virtual const char* taskName() { return "StatisticsTask"; };
        virtual int Execute();
        virtual void stop();

        virtual void flushStatistics();

        virtual void updateAccepted (const char* srcSmeId, const char* routeId);
        virtual void updateRejected (const char* srcSmeId, const char* routeId, int errcode);
        virtual void updateTemporal (const char* dstSmeId, const char* routeId, int errcode);
        virtual void updateChanged  (const char* dstSmeId, const char* routeId, int errcode = 0);
        virtual void updateScheduled(const char* dstSmeId, const char* routeId);
        
        StatisticsManager(DataSource& ds);
        virtual ~StatisticsManager();
    };

}}

#endif //SMSC_STAT_STATISTICS_MANAGER
