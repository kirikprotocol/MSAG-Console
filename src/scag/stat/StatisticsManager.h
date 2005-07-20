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
#include "routemap.h"
#include "TrafficRecord.h"

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

    using scag::stat::SmppStatEvent;

    struct CommonStat
    {
      int providerId;
      int accepted;
      int rejected;
      int delivered;
      int gw_rejected;
      int failed;
      int billingOk;
      int billingFailed;
      int recieptOk;
      int recieptFailed;

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
        gw_rejected=0;
        failed=0;
        billingOk = 0;
        billingFailed = 0;
        recieptOk = 0;
        recieptFailed = 0;
        providerId=-1;
        errors.Empty();
      }
    };

    /*struct ServiceStat
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
    };*/

    class StatisticsManager : public Statistics, public ThreadedTask
    {
      protected:

        smsc::logger::Logger    *logger;

        Hash<CommonStat>    statBySmeId[2];
        Hash<CommonStat>   statByRouteId[2];
        Hash<CommonStat>   srvStatBySmeId[2];
        IntHash<TrafficRecord> trafficByRouteId;

        int     currentIndex;
        bool    bExternalFlush;

        Mutex   stopLock, switchLock, flushLock;
        Event   awakeEvent, exitEvent, doneEvent;
        bool    isStarted;

        int   switchCounters();
        void  resetCounters(int index);
        void  flushCounters(int index);
        void  dumpCounters(const uint8_t* buff, int buffLen, const tm& flushTM);

        void flushTraffic();
        void dumpTraffic(const IntHash<TrafficRecord>& traff);
        void resetTraffic(const tm& tmDate);
        void incRouteTraffic(const int routeId, const tm& tmDate);

        void  calculateTime(tm& flushTM);
        int   calculateToSleep();

        void  incError(IntHash<int>& hash, int errcode);

        //File storage
    private:

        std::string     location;
        bool            bFileTM;
        tm              fileTM;
        FILE*           file;
        FILE*           tfile;
    
        void close();
        void flush();
        void write(const void* data, size_t size);

        std::string traffloc;
        void Fopen(FILE* &cfPtr, const std::string loc);
        void Fseek(long offset, int whence, FILE* &cfPtr);
        void Fclose(FILE* &cfPtr);
        void Fflush(FILE* &cfPtr);
        void Fwrite(const void* data, size_t size, FILE* &cfPtr);
        size_t Fread(FILE* &cfPtr, void* data, size_t size);
        void initTraffic();

        static bool createDir(const std::string& dir);
        bool createStorageDir(const std::string loc);

    public:

        static RouteMap routeMap;
        virtual const char* taskName() { return "StatisticsTask"; };
        virtual int Execute();
        virtual void stop();

        virtual void registerEvent(const SmppStatEvent& si);
        bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);

        StatisticsManager(Config config);
        virtual ~StatisticsManager();
    };

}//namespace stat
}//namespace scag

#endif //SMSC_STAT_STATISTICS_MANAGER
