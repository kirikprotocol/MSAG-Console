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
#include "TrafficRecord.h"
#include <core/buffers/File.hpp>
#include "scag/config/stat/StatManConfig.h"

#include "util/timeslotcounter.hpp"
#include "core/network/Socket.hpp"
#include "Sender.h"
#include "Performance.h"

#include "sacc/SACC_EventSender.h"

#include <util/BufferSerialization.hpp>


using namespace scag::stat::sacc;

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


    using namespace scag::stat::sacc;


    struct CommonStat
    {
      int providerId;
      int accepted;
      int rejected;
      int delivered;
      int gw_rejected;
      int failed;
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
        recieptOk = 0;
        recieptFailed = 0;
        providerId=-1;
        errors.Empty();
      }
    };

    struct HttpStat
    {
      int request;
      int requestRejected;
      int response;
      int responseRejected;
      int delivered;
      int failed;

      IntHash<int> errors;

      HttpStat()
      {
        Reset();
      }

      void Reset()
      {
        request = 0;
        requestRejected = 0;
        response = 0;
        responseRejected = 0;
        delivered = 0;
        failed = 0;

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
        Hash<TrafficRecord>  smppTrafficByRouteId;

        Hash<HttpStat>          httpStatByRouteId[2];
        Hash<HttpStat>          httpStatByUrl[2];
        Hash<TrafficRecord>  httpTrafficByRouteId;

        IntHash<std::string>       saccEventFiler;
        EventSender thrSaccSender;

        int     currentIndex;
        bool    bExternalFlush;

        Mutex   stopLock, switchLock, flushLock;
        Event   awakeEvent, exitEvent;
        bool    isStarted;

        int   switchCounters();
        void  resetCounters(int index);
        void  flushCounters(int index);
        void  dumpCounters(const uint8_t* buff, int buffLen, const tm& flushTM, tm& fileTM, const char *dirNameFmt, File& file);

        void  resetHttpCounters(int index);
        void  flushHttpCounters(int index);
        void  dumpHttpCounters(const uint8_t* buff, int buffLen, const tm& flushTM);

        void flushTraffic();
        void dumpTraffic(Hash<TrafficRecord>& traff, const std::string& path);
        void resetTraffic(Hash<TrafficRecord>& h, const tm& tmDate);
        void incSmppRouteTraffic(const char* routeId);

        void flushHttpTraffic();
        void incHttpRouteTraffic(const char* routeId);

        void  calculateTime(tm& flushTM);
        int   calculateToSleep();

        void  incError(IntHash<int>& hash, int errcode);

        Sender sender;

        Mutex                            svcCountersLock;
        Hash   <CommonPerformanceCounter*>  svcSmppCounters;
        Hash   <CommonPerformanceCounter*>  svcWapCounters;
        Hash   <CommonPerformanceCounter*>  svcMmsCounters;

        Array<Socket*> svcSockets;
        Mutex svcSocketsMutex;

        Mutex                             scCountersLock;
        Hash   <CommonPerformanceCounter*>  scSmppCounters;
        Hash   <CommonPerformanceCounter*>  scWapCounters;
        Hash   <CommonPerformanceCounter*>  scMmsCounters;

        Array<Socket*> scSockets;
        Mutex scSocketsMutex;

        Array<Socket*> genSockets;
        Mutex genSocketsMutex;

        GenStatistics genStatSmpp;
        GenStatistics genStatHttp;

        //File storage
    private:

        std::string     location;
        tm              smppFileTM;
        File smppFile;

        tm              httpFileTM;
        File httpFile;

        std::string traffloc;
        void Fopen(FILE* &cfPtr, const std::string loc);
        void Fseek(long offset, int whence, FILE* &cfPtr);
        void Fclose(FILE* &cfPtr);
        void Fflush(FILE* &cfPtr);
        void Fwrite(const void* data, size_t size, FILE* &cfPtr);
        size_t Fread(FILE* &cfPtr, void* data, size_t size);
        void initTraffic(Hash<TrafficRecord>& h, const std::string loc);

        static bool createDir(const std::string& dir);
        bool createStorageDir(const std::string loc);
        void Stop();
        void Start();

        inline TimeSlotCounter<int>* newSlotCounter() {
            return new TimeSlotCounter<int>(3600, 1000);
        }
        void incSmppCounter(const char* systemId, bool sc, int index);
//        void incSvcSmppCounter(const char* systemId, int index);
        void incSvcWapCounter(const char*  systemId, int index);
        void incSvcMmsCounter(const char*  systemId, int index);
        void StatisticsManager::dumpSvcCounters(SerializationBuffer& buf);

//        void incScSmppCounter(const char* systemId, int index);
        void incScWapCounter(const char*  systemId, int index);
        void incScMmsCounter(const char*  systemId, int index);
        void StatisticsManager::dumpScCounters(SerializationBuffer& buf);

        int indexByCounter(int counter);
        int indexByHttpCounter(int event);

        void SerializeSmppStat(Hash<CommonStat>& smppStat, SerializationBuffer& buf, bool add);
        void SerializeHttpStat(Hash<HttpStat>& httpStat, SerializationBuffer& buf);
        void incSvcScCounter(const char* systemId, int index, int max_cnt, Hash<CommonPerformanceCounter*>& svcCounters, Mutex& mt);
        void dumpPerfCounters(SerializationBuffer& buf, Hash<CommonPerformanceCounter*>& h);
        void reportPerformance(bool t, Mutex& mt, Array<Socket*>& socks);
        void incRouteTraffic(Hash<TrafficRecord>& h,  const char* routeId);
        void dumpTrafficHash(Hash<TrafficRecord>& traff, SerializationBuffer& buf);
        CommonStat* getStat(const char* id, bool sc);
    public:
        static void init(const StatManConfig& statManCfg);    
        
        virtual int Execute();

        virtual void registerEvent(const SmppStatEvent& se);
        virtual void registerEvent(const HttpStatEvent& se);
        virtual bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);

        virtual void registerSaccEvent(const scag::stat::SACC_TRAFFIC_INFO_EVENT_t& ev);
        virtual void registerSaccEvent(const scag::stat::SACC_BILLING_INFO_EVENT_t& ev);
//      virtual void registerSaccEvent(const scag::stat::SACC_OPERATOR_NOT_FOUND_ALARM_t& ev);
//      virtual void registerSaccEvent(const scag::stat::SACC_SESSION_EXPIRATION_TIME_ALARM_t& ev);
        virtual void registerSaccEvent(const scag::stat::SACC_ALARM_MESSAGE_t& ev);
        virtual void registerSaccEvent(const scag::stat::SACC_ALARM_t& ev);


        virtual void getSmppPerfData(uint64_t *cnt);
        virtual void getHttpPerfData(uint64_t *cnt);

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
