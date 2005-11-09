#ifndef SMSC_SCAG_STAT_STATISTICS_MANAGER
#define SMSC_SCAG_STAT_STATISTICS_MANAGER

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/TmpBuf.hpp>
#include <core/threads/Thread.hpp>

#include <logger/Logger.h>

#include <util/timeslotcounter.hpp>
#include <util/config/Config.h>

#include "Statistics.h"
#include "routemap.h"
#include "TrafficRecord.h"
#include <core/buffers/File.hpp>

namespace scag {

namespace stat {

    using namespace smsc::core::threads;
    using namespace smsc::core::synchronization;
    using smsc::core::buffers::File;

    using smsc::core::buffers::IntHash;
    using smsc::core::buffers::Hash;

    using smsc::logger::Logger;
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

    class StatisticsManager : public Statistics, public Thread
    {
    friend class Statistics;
    private:

        static bool  inited;
        static Mutex initLock;

        void configure(const std::string& dir);

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
        File file;

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
        bool started();
        void Stop();

    public:

        static RouteMap routeMap;

        static void init(const std::string& dir);    
        
        virtual int Execute();

        virtual void registerEvent(const SmppStatEvent& si);
        bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);

        StatisticsManager();
        virtual ~StatisticsManager();
    };

}//namespace stat
}//namespace scag

#endif //SMSC_STAT_STATISTICS_MANAGER
