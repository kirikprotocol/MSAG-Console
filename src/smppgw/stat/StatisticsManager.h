#ifndef SMSC_SMPPGW_STAT_STATISTICS_MANAGER
#define SMSC_SMPPGW_STAT_STATISTICS_MANAGER

#include "smppgw/stat/Statistics.h"

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/Hash.hpp>

#include <core/threads/ThreadedTask.hpp>

#include <db/DataSource.h>
#include <logger/Logger.h>

#include <util/timeslotcounter.hpp>

namespace smsc {

namespace smppgw {

namespace stat {

    using namespace smsc::db;
    using namespace core::threads;
    using namespace core::synchronization;

    using core::buffers::IntHash;
    using core::buffers::Hash;

    using smsc::logger::Logger;
    using smsc::util::TimeSlotCounter;

    struct CommonStat
    {
      int accepted;
      int rejected;
      int delivered;
      int temperror;
      int permerror;

      IntHash<int> errors;

      CommonStat()
      {
        Reset();
      }

      void Reset()
      {
        accepted=0;
        rejected=0;
        delivered=0;
        temperror=0;
        permerror=0;
        errors.Empty();
      }
    };

    struct ServiceStat
    {
      int SmsTrOk;
      int SmsTrFailed;
      int SmsTrBilled;
      int UssdTrFromScOk;
      int UssdTrFromScFailed;
      int UssdTrFromScFailedBilled;
      int UssdTrFromSmeOk;
      int UssdTrFromSmeFailed;
      int UssdTrFromSmeFailedBilled;
      ServiceStat()
      {
        Reset();
      }
      void Reset()
      {
        memset(this,0,sizeof(*this));
      }
    };

    class GWStatisticsManager : public IStatistics, public ThreadedTask
    {
      protected:

        smsc::logger::Logger    *logger;
        DataSource              &ds;

        CommonStat      statCommon[2];

        struct TotalStat{
          CommonStat common;
          ServiceStat service;
        };

        Hash<TotalStat>    totalStatBySmeId[2];
        Hash<CommonStat>   commonStatByRoute[2];

        int   currentIndex;
        bool    bExternalFlush;

        Mutex   stopLock, switchLock, flushLock;
        Event   awakeEvent, exitEvent, doneEvent;
        bool    isStarted;

        int switchCounters();
        void  resetCounters(int index);
        void  flushCounters(int index);

        uint32_t calculatePeriod();
        int      calculateToSleep();

        void  incError(IntHash<int>& hash, int errcode);

    public:

        virtual const char* taskName() { return "StatisticsTask"; };
        virtual int Execute();
        virtual void stop();

        virtual void flushStatistics();

        virtual void updateCounter(int counter,const char* srcSmeId, const char* routeId,int errorCode);

        GWStatisticsManager(DataSource& ds);
        virtual ~GWStatisticsManager();
    };

}//namespace stat
}//namespace smppgw
}//namespace smsc

#endif //SMSC_STAT_STATISTICS_MANAGER
