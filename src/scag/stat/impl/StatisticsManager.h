#ifndef SMSC_SCAG_STAT_IMPL_STATISTICS_MANAGER
#define SMSC_SCAG_STAT_IMPL_STATISTICS_MANAGER

#include <core/synchronization/Mutex.hpp>
#include <core/synchronization/Event.hpp>
#include <core/buffers/IntHash.hpp>
#include <core/buffers/Hash.hpp>
#include <core/buffers/TmpBuf.hpp>
#include <core/buffers/Array.hpp>
#include <core/threads/Thread.hpp>

#include <logger/Logger.h>

#include <util/timeslotcounter.hpp>
#include "scag/config/base/Config.h"

#include "scag/stat/base/Statistics2.h"
#include "TrafficRecord.h"
#include <core/buffers/File.hpp>
#include "scag/config/stat/StatManConfig.h"
#include "scag/config/base/ConfigListener2.h"
#include "util/timeslotcounter.hpp"
#include "core/network/Socket.hpp"
#include "Sender.h"
#include "Performance.h"
#include "scag/stat/impl/SACC_EventSender.h"

#include <util/BufferSerialization.hpp>

namespace scag2 {
namespace stat {

    using namespace smsc::core::threads;
    using namespace smsc::core::synchronization;
    using smsc::core::buffers::File;
    using smsc::core::buffers::Array;
    using namespace config;

    using smsc::core::buffers::IntHash;
    using smsc::core::buffers::Hash;

    using smsc::logger::Logger;
    // using smsc::util::config::Config;
    using stat::SmppStatEvent;

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

    class StatisticsManager : public Statistics, public PerformanceListener, public PerformanceServer, public Thread, public ConfigListener
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
        sacc::EventSender thrSaccSender;

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

        std::vector<Socket*> svcSockets;
        Mutex svcSocketsMutex;

        Mutex                             scCountersLock;
        Hash   <CommonPerformanceCounter*>  scSmppCounters;
        Hash   <CommonPerformanceCounter*>  scWapCounters;
        Hash   <CommonPerformanceCounter*>  scMmsCounters;

        std::vector<Socket*> scSockets;
        Mutex scSocketsMutex;

        std::vector<Socket*> genSockets;
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
        void initTraffic(Hash<TrafficRecord>& h, const std::string loc);

        static bool createDir(const std::string& dir);
        bool createStorageDir(const std::string loc);

        inline TimeSlotCounter<int>* newSlotCounter() {
            return new TimeSlotCounter<int>(3600, 1000);
        }
        void incSmppCounter(const char* systemId, bool sc, int index);
//        void incSvcSmppCounter(const char* systemId, int index);
        void incSvcWapCounter(const char*  systemId, int index);
        void incSvcMmsCounter(const char*  systemId, int index);
        void dumpSvcCounters(SerializationBuffer& buf);

//        void incScSmppCounter(const char* systemId, int index);
        void incScWapCounter(const char*  systemId, int index);
        void incScMmsCounter(const char*  systemId, int index);
        void dumpScCounters(SerializationBuffer& buf);

        int indexByCounter(int counter);
        int indexByHttpCounter(int event);

        void SerializeSmppStat(Hash<CommonStat>& smppStat, SerializationBuffer& buf, bool add);
        void SerializeHttpStat(Hash<HttpStat>& httpStat, SerializationBuffer& buf);
        void incSvcScCounter(const char* systemId, int index, int max_cnt, Hash<CommonPerformanceCounter*>& svcCounters, Mutex& mt);
        void dumpPerfCounters(SerializationBuffer& buf, Hash<CommonPerformanceCounter*>& h);
        void reportPerformance(bool t, Mutex& mt, std::vector<Socket*>& socks);
        void incRouteTraffic(Hash<TrafficRecord>& h,  const char* routeId);
        void dumpTrafficHash(Hash<TrafficRecord>& traff, SerializationBuffer& buf);
        CommonStat* getStat(const char* id, bool sc);
        void deleteSockets( Mutex& mt, std::vector< Socket* >& socks );
        void deleteCommonPerfCounters( Hash< CommonPerformanceCounter* >& counters );
    public:
        void init( const StatManConfig& statManCfg );

        void Stop();                        
        void Start();
        
        void configChanged();
        
        virtual int Execute();

        virtual void registerEvent(const SmppStatEvent& se);
        virtual void registerEvent(const HttpStatEvent& se);
        virtual bool checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value);

        virtual void registerSaccEvent( stat::SaccTrafficInfoEvent* ev );
        virtual void registerSaccEvent( stat::SaccBillingInfoEvent* ev );
        virtual void registerSaccEvent( stat::SaccAlarmEvent* ev );
        virtual void registerSaccEvent( stat::SaccAlarmMessageEvent* ev );

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

    private:
        // to make compiler happy
        void Start( int );
    };

}//namespace stat
}//namespace scag

#endif //SMSC_STAT_STATISTICS_MANAGER
