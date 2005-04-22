#ifndef SMSC_SMPPGW_STAT_STATISTICS_MANAGER
#define SMSC_SMPPGW_STAT_STATISTICS_MANAGER

#include "smppgw/stat/Statistics.h"

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/TmpBuf.hpp>
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
    using core::buffers::TmpBuf;
    using core::buffers::Hash;

    using smsc::logger::Logger;
    using smsc::util::TimeSlotCounter;

    struct CommonStat
    {
      int providerId;
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
        providerId=-1;
        errors.Empty();
      }
    };

    struct ServiceStat
    {
      int DeniedByBilling;
      int SmsTrOk;
      int SmsTrFailed;
      int SmsTrBilled;
      int UssdTrFromScOk;
      int UssdTrFromScFailed;
      int UssdTrFromScBilled;
      int UssdTrFromSmeOk;
      int UssdTrFromSmeFailed;
      int UssdTrFromSmeBilled;
      ServiceStat()
      {
        Reset();
      }
      void Reset()
      {
        memset(this,0,sizeof(*this));
      }
    };

    class StatStorage
    {
    private:
    
        smsc::logger::Logger    *logger;

        std::string     location;
        bool            bFileTM;
        tm              fileTM;
        FILE*           file;
    
        void close();
        void flush();
        void write(const void* data, size_t size);

        static bool createDir(const std::string& dir);
        bool createStatDir();
    
    public:
    
        StatStorage(const std::string& location);
        ~StatStorage();
        
        void dump(const uint8_t* buff, int buffLen, const tm& flushTM);
    };

    class GWStatisticsManager : public IStatistics, public ThreadedTask
    {
      protected:

        smsc::logger::Logger    *logger;

        struct TotalStat{
          CommonStat common;
          ServiceStat service;
        };

        Hash<TotalStat>    totalStatBySmeId[2];
        Hash<CommonStat>   commonStatByRoute[2];

        int     currentIndex;
        bool    bExternalFlush;

        Mutex   stopLock, switchLock, flushLock;
        Event   awakeEvent, exitEvent, doneEvent;
        bool    isStarted;

        StatStorage storage;

        int switchCounters();
        void  resetCounters(int index);
        void  flushCounters(int index);

        void  calculateTime(tm& flushTM);
        uint32_t calculatePeriod();
        int      calculateToSleep();

        void  incError(IntHash<int>& hash, int errcode);

    public:

        virtual const char* taskName() { return "StatisticsTask"; };
        virtual int Execute();
        virtual void stop();

        virtual void flushStatistics();

        virtual void updateCounter(int counter,const StatInfo& si,int errorCode);

        //GWStatisticsManager(DataSource& ds);
        GWStatisticsManager(std::string& location);
        virtual ~GWStatisticsManager();
    };

}//namespace stat
}//namespace smppgw
}//namespace smsc

#endif //SMSC_STAT_STATISTICS_MANAGER
