#ifndef SMSC_SCAG_STAT_STATISTICS_MANAGER
#define SMSC_SCAG_STAT_STATISTICS_MANAGER

#include "scag/stat/Statistics.h"

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/TmpBuf.hpp>
#include <core/threads/ThreadedTask.hpp>

#include <db/DataSource.h>
#include <logger/Logger.h>

#include <util/timeslotcounter.hpp>
#include "util/config/Config.h"

namespace scag {

namespace stat {

    //using namespace smsc::db;
    using namespace smsc::core::threads;
    using namespace smsc::core::synchronization;

    using smsc::core::buffers::IntHash;
    //using core::buffers::TmpBuf;
    using smsc::core::buffers::Hash;

    using smsc::logger::Logger;
    //using smsc::util::TimeSlotCounter;

    using smsc::util::config::Config;

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

    class StatisticsManager : public Statistics, public ThreadedTask
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

        int   switchCounters();
        void  resetCounters(int index);
        void  flushCounters(int index);
        void  dumpCounters(const uint8_t* buff, int buffLen, const tm& flushTM);

        void  calculateTime(tm& flushTM);
        int   calculateToSleep();

        void  incError(IntHash<int>& hash, int errcode);

        //File storage
    private:

        std::string     location;
        bool            bFileTM;
        tm              fileTM;
        FILE*           file;
    
        void close();
        void write(const void* data, size_t size);

        static bool createDir(const std::string& dir);
        bool createStatDir();

    public:

        virtual const char* taskName() { return "StatisticsTask"; };
        virtual int Execute();
        virtual void stop();

        virtual void flushStatistics();

        virtual void registerCommand(SmppCommand cmd);
        virtual void registerCommand(WapCommand cmd);
        virtual void registerCommand(MmsCommand cmd);
        bool checkTraffic(string routeId, int period);

        StatisticsManager(Config config);
        virtual ~StatisticsManager();
    };

}//namespace stat
}//namespace scag

#endif //SMSC_STAT_STATISTICS_MANAGER
