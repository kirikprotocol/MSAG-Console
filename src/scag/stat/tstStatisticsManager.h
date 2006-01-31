#ifndef SMSC_SCAG_STATX_STATISTICS_MANAGER
#define SMSC_SCAG_STATX_STATISTICS_MANAGER

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

    class StatisticsManager : public Statistics, public PerformanceListener, public PerformanceServer, public Thread
    {
    friend class Statistics;
    private:
 
        static bool  inited;
        static Mutex initLock;
 
    protected:

        smsc::logger::Logger    *logger;

        bool    isStarted;
        Event   awakeEvent, exitEvent;
        bool    bExternalFlush;
    private:


        void Stop();
        void Start();

    public:

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

}//namespace statx
}//namespace scag

#endif //SMSC_STATX_STATISTICS_MANAGER
