#ifndef SMSC_SCAG_STAT_STATISTICS_MANAGER
#define SMSC_SCAG_STAT_STATISTICS_MANAGER

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/TmpBuf.hpp>
#include <core/buffers/Array.hpp>
#include <core/threads/Thread.hpp>

#include <logger/Logger.h>

#include <util/timeslotcounter.hpp>
#include <util/config/Config.h>

#include "Statistics.h"
#include "routemap.h"
#include "TrafficRecord.h"
#include <core/buffers/File.hpp>
#include "scag/config/stat/StatManConfig.h"

#include "util/timeslotcounter.hpp"
#include "core/network/Socket.hpp"
#include "Sender.h"
#include "Performance.h"

namespace scag {

namespace stat {

    using namespace smsc::core::threads;
    using namespace smsc::core::synchronization;
    using smsc::core::buffers::File;
    using smsc::core::buffers::Array;
    using scag::config::StatManConfig;

    using smsc::core::buffers::IntHash;
    using smsc::core::buffers::Hash;

    using smsc::logger::Logger;
    using smsc::util::config::Config;
    using scag::stat::SmppStatEvent;

    using smsc::util::TimeSlotCounter;
    using smsc::core::network::Socket;

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

    struct HttpStat
    {
      int providerId;
      int request;
      int requestRejected;
      int response;
      int responseRejected;
      int delivered;
      int failed;

      int billingOk;
      int billingFailed;

      IntHash<int> errors;

      HttpStat()
      {
        Reset();
      }

      void Reset()
      {
        providerId = -1;
        request = 0;
        requestRejected = 0;
        response = 0;
        responseRejected = 0;
        delivered = 0;
        failed = 0;

        billingOk = 0;
        billingFailed = 0;
        errors.Empty();
      }
    };

    class StatisticsManager : public Statistics, public PerformanceListener, public PerformanceServer, public Thread
    {
    friend class Statistics;
    private:

        static bool  inited;
        static Mutex initLock;

        void configure(const StatManConfig& statManCfg);

    protected:

        smsc::logger::Logger    *logger;

        Hash<CommonStat>        statBySmeId[2];
        Hash<CommonStat>        statByRouteId[2];
        Hash<CommonStat>        srvStatBySmeId[2];
        IntHash<TrafficRecord>  trafficByRouteId;

        Hash<HttpStat>          httpStatByRouteId[2];
        Hash<HttpStat>          httpStatByServiceId[2];
        IntHash<HttpStat>       httpStatByProviderId[2];
        IntHash<TrafficRecord>  httpTrafficByRouteId;

        int     currentIndex;
        bool    bExternalFlush;

        Mutex   stopLock, switchLock, flushLock;
        Event   awakeEvent, exitEvent;
        bool    isStarted;

        int   switchCounters();
        void  resetCounters(int index);
        void  flushCounters(int index);
        void  dumpCounters(const uint8_t* buff, int buffLen, const tm& flushTM);

        void  resetHttpCounters(int index);
        void  flushHttpCounters(int index);
        void  dumpHttpCounters(const uint8_t* buff, int buffLen, const tm& flushTM);

        void flushTraffic();
        void dumpTraffic(const IntHash<TrafficRecord>& traff, const std::string& path);
        void resetTraffic(const tm& tmDate);
        void incRouteTraffic(const int routeId, const tm& tmDate);

        void flushHttpTraffic();
        void resetHttpTraffic(const tm& tmDate);
        void incHttpRouteTraffic(const int routeId, const tm& tmDate);

        void  calculateTime(tm& flushTM);
        int   calculateToSleep();

        void  incError(IntHash<int>& hash, int errcode);

        Sender sender;

        Mutex                            svcCountersLock;
        Hash   <SmppPerformanceCounter*>  svcSmppCounters;
        Hash   <HttpPerformanceCounter*>  svcWapCounters;
        Hash   <SmppPerformanceCounter*>  svcMmsCounters;

        Array<Socket*> svcSockets;
        Mutex svcSocketsMutex;

        Mutex                             scCountersLock;
        Hash   <SmppPerformanceCounter*>  scSmppCounters;
        Hash   <HttpPerformanceCounter*>  scWapCounters;
        Hash   <SmppPerformanceCounter*>  scMmsCounters;

        Array<Socket*> scSockets;
        Mutex scSocketsMutex;

        Array<Socket*> genSockets;
        Mutex genSocketsMutex;

        GenStatistics genCounters;

        //File storage
    private:

        std::string     location;
        bool            bFileTM;
        tm              fileTM;
        File file;

        bool            httpIsFileTM;
        tm              httpFileTM;
        File httpFile;

        std::string traffloc;
        void Fopen(FILE* &cfPtr, const std::string loc);
        void Fseek(long offset, int whence, FILE* &cfPtr);
        void Fclose(FILE* &cfPtr);
        void Fflush(FILE* &cfPtr);
        void Fwrite(const void* data, size_t size, FILE* &cfPtr);
        size_t Fread(FILE* &cfPtr, void* data, size_t size);
        void initTraffic();
        void initHttpTraffic();

        static bool createDir(const std::string& dir);
        bool createStorageDir(const std::string loc);
        bool started();
        void Stop();
        void Start();

        inline TimeSlotCounter<int>* newSlotCounter() {
            return new TimeSlotCounter<int>(3600, 1000);
        }
        void incSvcSmppCounter(const char* systemId, int index);
        void incSvcWapCounter(const char*  systemId, int index);
        void incSvcMmsCounter(const char*  systemId, int index);
        uint8_t* StatisticsManager::dumpSvcCounters(uint32_t& smePerfDataSize);

        void incScSmppCounter(const char* systemId, int index);
        void incScWapCounter(const char*  systemId, int index);
        void incScMmsCounter(const char*  systemId, int index);
        uint8_t* StatisticsManager::dumpScCounters(uint32_t& smePerfDataSize);

        int indexByCounter(int counter);
        int indexByHttpCounter(int counter);
    public:

        static RouteMap routeMap;
        static RouteMap httpRouteMap;

        static void init(const StatManConfig& statManCfg);    
        
        virtual int Execute();

        virtual void registerEvent(const SmppStatEvent& se);
        virtual void registerEvent(const HttpStatEvent& se);

        bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);

        virtual void getSmppPerfData(uint64_t *cnt);
        //virtual void getHttpPerfData(uint64_t *cnt);
        virtual void reportGenPerformance(PerformanceData * data);
        virtual void reportSvcPerformance();
        virtual void reportScPerformance();
        virtual void addSvcSocket(Socket * socket);
        virtual void addScSocket(Socket * socket);
        virtual void addGenSocket(Socket * socket);

        StatisticsManager();
        virtual ~StatisticsManager();
    };

}//namespace stat
}//namespace scag

#endif //SMSC_STAT_STATISTICS_MANAGER
