#include <sys/types.h>
#include <netinet/in.h>

#ifdef _WIN32
#include <stdint.h>
#else
#include <inttypes.h>
#endif

#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <errno.h>  

#include <memory>
#include <vector>
#include <string>

#include <scag/util/singleton/Singleton.h>
#include "StatisticsManager.h"

namespace scag { 
namespace stat {

bool  StatisticsManager::inited = false;
Mutex StatisticsManager::initLock;

using smsc::core::buffers::TmpBuf;
using namespace scag::util::singleton;
using smsc::core::buffers::File;
using scag::config::StatManConfig;

const uint16_t SCAG_STAT_DUMP_INTERVAL = 60; // in seconds
const uint16_t SCAG_STAT_VERSION_INFO  = 0x0001;
const char*    SCAG_STAT_HEADER_TEXT   = "SCAG.STAT";
const char*    SCAG_SMPP_STAT_DIR_NAME_FORMAT  = "SMPP/%04d-%02d";
const char*    SCAG_HTTP_STAT_DIR_NAME_FORMAT  = "HTTP/%04d-%02d";
const char*    SCAG_STAT_FILE_NAME_FORMAT = "%02d.rts";
RouteMap StatisticsManager::routeMap;
RouteMap StatisticsManager::httpRouteMap;

inline unsigned GetLongevity(StatisticsManager*) { return 5; }
//typedef SingletonHolder<StatisticsManager, CreateUsingNew, SingletonWithLongevity> SingleSM;
typedef SingletonHolder<StatisticsManager> SingleSM;

Statistics& Statistics::Instance()
{
    if (!StatisticsManager::inited) 
    {
        MutexGuard guard(StatisticsManager::initLock);
        if (!StatisticsManager::inited) 
            throw std::runtime_error("Statistics not inited!");
    }
    return SingleSM::Instance();
}
void StatisticsManager::init(const StatManConfig& statManConfig)
{
    if (!StatisticsManager::inited)
    {
        MutexGuard guard(StatisticsManager::initLock);
        if (!StatisticsManager::inited) {
            StatisticsManager& sm = SingleSM::Instance();
            sm.configure(statManConfig);
            sm.Start();
            StatisticsManager::inited = true;
        }
    }
}

void StatisticsManager::configure(const StatManConfig& statManConfig)
{
    location = statManConfig.getDir();
    traffloc = location;
    if( !location.length() )
        throw Exception("StatisticsManager, configure: Dirrectory has zero length.");

    // Deletes last slash if it exists
    int len = location.length();
    const char slash = location.c_str()[len-1];
    if(slash == '/'){
        char loc[1024];
        memcpy((void*)loc, (const void*)location.c_str(), len-1);
        loc[len-1] = 0;
        location = loc;
    }

    std::string smppLoc = location + "/SMPP";
    if (!createStorageDir(smppLoc)) 
        throw Exception("Can't open statistics directory: '%s'", smppLoc.c_str());

    std::string httpLoc = location + "/HTTP";
    if (!createStorageDir(httpLoc)) 
        throw Exception("Can't open statistics directory: '%s'", httpLoc.c_str());

    std::string perfHost = statManConfig.getPerfHost();
    if(!perfHost.length())
        throw Exception("StatisticsManager, configure: Performance host has zero length.");
    int perfGenPort = statManConfig.getPerfGenPort();
    int perfSvcPort = statManConfig.getPerfSvcPort();
    int perfScPort = statManConfig.getPerfScPort();
    printf("StatisticsManager, perfSvcPort: %d\n", perfSvcPort);

    sender.init((PerformanceListener*)this, (PerformanceServer*)this);
    sender.InitServer(perfHost, perfGenPort, perfSvcPort, perfScPort);

    initTraffic();     // Initializes traffic hash
    initHttpTraffic(); // Initializes http traffic hash
    logger = Logger::getInstance("statman");
}


StatisticsManager::StatisticsManager()
    : Statistics(), Thread(), 
      logger(Logger::getInstance("scag.stat.StatisticsManager")),
      currentIndex(0), bExternalFlush(false), isStarted(false) 
{
}
StatisticsManager::~StatisticsManager()
{
  Logger::Init();
  logger = Logger::getInstance("scag.stat.StatisticsManager");

  file.Close();
  Stop();
  WaitFor();
}

void StatisticsManager::incError(IntHash<int>& hash, int errcode)
{
    int* counter = hash.GetPtr(errcode);
    if (!counter) hash.Insert(errcode, 1);
    else (*counter)++;
}

void StatisticsManager::registerEvent(const SmppStatEvent& se)
{
  MutexGuard  switchGuard(switchLock);

  CommonStat* smeSt=0;
  CommonStat* routeSt=0;
  CommonStat* srvSt=0;

  using namespace Counters;

  genCounters.incSmpp(se.counter);

  if (se.smeId && se.smeId[0])
  {
    if(se.counter < cntBillingOk){
      
        if(se.internal){
            srvSt = srvStatBySmeId[currentIndex].GetPtr(se.smeId);
            if(!srvSt){
                CommonStat newStat;
                srvStatBySmeId[currentIndex].Insert(se.smeId, newStat);
                srvSt=srvStatBySmeId[currentIndex].GetPtr(se.smeId);
            }

            // run-time statistics
            printf("register sc, serviceId: %s\n", se.smeId);
            incScSmppCounter(se.smeId, indexByCounter(se.counter));
        }else{
            smeSt = statBySmeId[currentIndex].GetPtr(se.smeId);
            if(!smeSt)
            {
                CommonStat newStat;
                statBySmeId[currentIndex].Insert(se.smeId, newStat);
                smeSt=statBySmeId[currentIndex].GetPtr(se.smeId);
            }

            // run-time statistics
            incSvcSmppCounter(se.smeId, indexByCounter(se.counter));
        }
    }
  }

  if (se.routeId && se.routeId[0])
  {
    //smsc_log_debug(logger, "routeId: '%s'", se.routeId);
    routeSt = statByRouteId[currentIndex].GetPtr(se.routeId);
    if(!routeSt)
    {
      CommonStat newStat;
      statByRouteId[currentIndex].Insert(se.routeId, newStat);
      routeSt=statByRouteId[currentIndex].GetPtr(se.routeId);
    }
  }

  if(se.counter < cntBillingOk)
  {
    if(smeSt) incError(smeSt->errors, se.errCode);
    if(srvSt) incError(srvSt->errors, se.errCode);
    if(routeSt) incError(routeSt->errors, se.errCode);
  }

  if(smeSt && se.smeProviderId!=-1)smeSt->providerId=se.smeProviderId;
  if(srvSt && se.smeProviderId!=-1)srvSt->providerId=se.smeProviderId;
  if(routeSt && se.routeProviderId!=-1)routeSt->providerId=se.routeProviderId;

  int c;
  switch(se.counter)
  {
#define INC_STAT(cnt,field) case cnt:{\
      if(smeSt)smeSt->field++; \
      if(srvSt)srvSt->field++; \
      if(routeSt)routeSt->field++; \
      }break;

    INC_STAT(cntAccepted,accepted)
    INC_STAT(cntRejected,rejected)
    INC_STAT(cntDelivered,delivered)
    INC_STAT(cntGw_Rejected,gw_rejected)
    INC_STAT(cntFailed,failed)
    INC_STAT(cntBillingOk,billingOk)
    INC_STAT(cntBillingFailed,billingFailed)
    INC_STAT(cntRecieptOk,recieptOk)
    INC_STAT(cntRecieptFailed,recieptFailed)
                    
#undef INC_STAT

  }

  if (se.routeId && se.routeId[0])
  {             
      if( se.counter == cntAccepted){
          int id, newRouteId;
          if(  (id = routeMap.regRoute(se.routeId, newRouteId)) == -1)
              id = newRouteId;

          time_t now = time(0);
          tm tmnow;   localtime_r(&now, &tmnow);
          TrafficRecord *tr = 0;
          tr = trafficByRouteId.GetPtr(id);
          if(tr){
                tr->inc(tmnow);
          }else{
              TrafficRecord tr(1, 1, 1, 1, 
                               tmnow.tm_year, tmnow.tm_mon, tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min);
              trafficByRouteId.Insert(id, tr);
          }
      }
  }
}

void StatisticsManager::registerEvent(const HttpStatEvent& se)
{
      // TODO: implement !!!
    MutexGuard  switchGuard(switchLock);
    
    HttpStat* providerSt=0;
    HttpStat* routeSt=0;
    HttpStat* srvSt=0;
    
    using namespace Counters;
    
    genCounters.incHttp(se.counter);
    
    if (se.serviceProviderId != -1)
    {
        if(se.counter < httpBillingOk){
                providerSt = httpStatByProviderId[currentIndex].GetPtr(se.serviceProviderId);                
    
                if(!providerSt)
                {
                    HttpStat newStat;
                    httpStatByProviderId[currentIndex].Insert(se.serviceProviderId, newStat);
                    providerSt=httpStatByProviderId[currentIndex].GetPtr(se.serviceProviderId);
                }        
        }
    }
    
    if (se.routeId.length())
    {
        //smsc_log_debug(logger, "routeId: '%s'", se.routeId);
        routeSt = httpStatByRouteId[currentIndex].GetPtr(se.routeId.c_str());
        if(!routeSt)
        {
            HttpStat newStat;
            httpStatByRouteId[currentIndex].Insert(se.routeId.c_str(), newStat);
            routeSt=httpStatByRouteId[currentIndex].GetPtr(se.routeId.c_str());
        }
    }

    if (se.serviceId.length())
    {
        if(se.counter < httpBillingOk){
            //smsc_log_debug(logger, "routeId: '%s'", se.routeId);
            srvSt = httpStatByServiceId[currentIndex].GetPtr(se.serviceId.c_str());
            if(!srvSt)
            {
                HttpStat newStat;
                httpStatByServiceId[currentIndex].Insert(se.serviceId.c_str(), newStat);
                srvSt=httpStatByServiceId[currentIndex].GetPtr(se.serviceId.c_str());
            }
            incSvcWapCounter(se.serviceId.c_str(), indexByHttpCounter(se.counter));
        }
    }
    
    if(se.counter < httpBillingOk)
    {
        if(providerSt)  incError(providerSt->errors, se.errCode);
        if(routeSt)     incError(routeSt->errors, se.errCode);
        if(srvSt)       incError(srvSt->errors, se.errCode);
    }
    
    if(providerSt && se.serviceProviderId != -1) providerSt->providerId = se.serviceProviderId;
    if(routeSt    && se.serviceProviderId != -1) routeSt->providerId = se.serviceProviderId;
    if(srvSt      && se.serviceProviderId != -1) srvSt->providerId = se.serviceProviderId;
    
    int c;
    switch(se.counter)
    {
    #define INC_STAT(cnt,field) case cnt:{\
          if(providerSt)providerSt->field++; \
          if(routeSt)routeSt->field++; \
          if(srvSt)routeSt->field++; \
          }break;
    
        INC_STAT(httpRequest,request)
        INC_STAT(httpRequestRejected,requestRejected)
        INC_STAT(httpResponse,response)
        INC_STAT(httpResponseRejected,responseRejected)
        INC_STAT(httpDelivered,delivered)
        INC_STAT(httpFailed,failed)
        INC_STAT(httpBillingOk,billingOk)
        INC_STAT(httpBillingFailed,billingFailed)

    #undef INC_STAT
    
    }
    
    if (se.routeId.length())
    {
        if( se.counter == httpRequest){
              int id, newRouteId;
              if(  (id = httpRouteMap.regRoute(se.routeId.c_str(), newRouteId)) == -1)
                  id = newRouteId;
    
              time_t now = time(0);
              tm tmnow;   localtime_r(&now, &tmnow);
              TrafficRecord *tr = 0;
              tr = httpTrafficByRouteId.GetPtr(id);
              if(tr){
                    tr->inc(tmnow);
              }else{
                  TrafficRecord tr(1, 1, 1, 1, 
                                   tmnow.tm_year, tmnow.tm_mon, tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min);
                  httpTrafficByRouteId.Insert(id, tr);
              }
        }
    }
}
    
    bool StatisticsManager::checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value)
    {
            MutexGuard mg(switchLock);
    
            int id;
            if(  (id = routeMap.getIntRouteId(routeId.c_str())) == -1)
                return false;
    
            TrafficRecord *tr = 0;
    
            switch(period){
            case checkMinPeriod:
                tr = trafficByRouteId.GetPtr(id);
                if(tr){
                    time_t now = time(0);
                    tm tmnow;   localtime_r(&now, &tmnow);
                    tr->reset(tmnow);
    
                    long mincnt_, hourcnt_, daycnt_, monthcnt_;
                    uint8_t year_, month_, day_, hour_, min_;
                    tr->getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                        year_, month_, day_, hour_, min_);
                    if(mincnt_ <= value)
                        return true;
                }
                break;
            case checkHourPeriod:
                tr = trafficByRouteId.GetPtr(id);
                if(tr){
                    time_t now = time(0);
                    tm tmnow;   localtime_r(&now, &tmnow);
                    tr->reset(tmnow);
    
                    long mincnt_, hourcnt_, daycnt_, monthcnt_;
                    uint8_t year_, month_, day_, hour_, min_;
                    tr->getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                        year_, month_, day_, hour_, min_);
                    if(hourcnt_ <= value)
                        return true;
                }
                break;
            case checkDayPeriod:
                tr = trafficByRouteId.GetPtr(id);
                if(tr){
                    time_t now = time(0);
                    tm tmnow;   localtime_r(&now, &tmnow);
                    tr->reset(tmnow);
    
                    long mincnt_, hourcnt_, daycnt_, monthcnt_;
                    uint8_t year_, month_, day_, hour_, min_;
                    tr->getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                        year_, month_, day_, hour_, min_);
                    if(daycnt_ <= value)
                        return true;
                }
                break;
            case checkMonthPeriod:
                tr = trafficByRouteId.GetPtr(id);
                if(tr){
                    time_t now = time(0);
                    tm tmnow;   localtime_r(&now, &tmnow);
                    tr->reset(tmnow);
    
                    long mincnt_, hourcnt_, daycnt_, monthcnt_;
                    uint8_t year_, month_, day_, hour_, min_;
                    tr->getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                        year_, month_, day_, hour_, min_);
                    //printf("%04d-%02d-%02d %02d:%02d (m, h, d, M): %d, %d, %d, %d\n", year_ + 1900, month_ + 1, day_, hour_, min_, mincnt_, hourcnt_, daycnt_, monthcnt_);
                    if(monthcnt_ <= value)
                        return true;
                }
                break;
            }
            return false;
}

int StatisticsManager::Execute()
{
    smsc_log_debug(logger, "PerformanceServer is starting...");
    sender.Start();
    smsc_log_debug(logger, "PerformanceServer is started");

    {
        MutexGuard mg(stopLock);
        isStarted = true; 
    }

    bExternalFlush = false;

    while (started())
    {
        int toSleep = calculateToSleep();
        smsc_log_debug(logger, "Execute() >> Start wait %d", toSleep);
        awakeEvent.Wait(toSleep); // Wait for next hour begins ...
        smsc_log_debug(logger, "Execute() >> End wait");

        int flushIndex = switchCounters();
        flushCounters(flushIndex);
        flushHttpCounters(flushIndex);

        flushTraffic();
        flushHttpTraffic();

        bExternalFlush = false;
        doneEvent.Signal();
        smsc_log_debug(logger, "Execute() >> Flushed");
    }

    {
        MutexGuard guard(stopLock);
        isStarted = false;
    }

    //exitEvent.Signal();

    smsc_log_debug(logger, "PerformanceServer is shutdowninig...");
    sender.Stop();
    smsc_log_debug(logger, "PerformanceServer is shutdowned");

    smsc_log_debug(logger, "Execute() exited");
    return 0;
}

void StatisticsManager::Stop()
{
    MutexGuard guard(stopLock);

    //smsc_log_debug(logger, "stop() called, started=%d", isStarted);
    if (isStarted)
    {
        isStarted = false;
        bExternalFlush = true;
        awakeEvent.Signal();
        //smsc_log_debug(logger, "stop() waiting finish ...");
        //exitEvent.Wait();
    }
    //smsc_log_debug(logger, "stop() exited");
}

bool StatisticsManager::started()
{
    bool isStart = false;
    {
        MutexGuard guard(stopLock);
        isStart = isStarted;
    }
    return isStart;
}

int StatisticsManager::switchCounters()
{
    MutexGuard  switchGuard(switchLock);

    tm tmDate;
    time_t date = time(0);
    localtime_r(&date, &tmDate);
    resetTraffic(tmDate);
    resetHttpTraffic(tmDate);

    int flushIndex = currentIndex;
    currentIndex ^= 1; //switch between 0 and 1
    return flushIndex;
}

void StatisticsManager::calculateTime(tm& flushTM)
{
    time_t flushTime = time(0);
    if (!bExternalFlush) flushTime -= SCAG_STAT_DUMP_INTERVAL;
    gmtime_r(&flushTime, &flushTM); flushTM.tm_sec = 0;
}

int StatisticsManager::calculateToSleep() // returns msecs to next hour
{
    time_t currTime = time(0);
    time_t nextTime = currTime + 60;
    tm tmNT; localtime_r(&nextTime, &tmNT);
    tmNT.tm_sec = 0;
    nextTime = mktime(&tmNT);
    return (((nextTime-currTime)*1000)+1);
}

void StatisticsManager::flushCounters(int index)
{

    tm flushTM; calculateTime(flushTM);
    smsc_log_debug(logger, "Flushing SMPP statistics for %02d.%02d.%04d %02d:%02d:%02d GMT",
                   flushTM.tm_mday, flushTM.tm_mon+1, flushTM.tm_year+1900,
                   flushTM.tm_hour, flushTM.tm_min, flushTM.tm_sec);

    try
    {
        TmpBuf<uint8_t, 4096> buff(4096);

        // Head of record
        uint8_t value8 = 0;
        value8 = (uint8_t)(flushTM.tm_hour); buff.Append((uint8_t *)&value8, sizeof(value8));
        value8 = (uint8_t)(flushTM.tm_min);  buff.Append((uint8_t *)&value8, sizeof(value8));

        // Route statistic
        int32_t value32 = 0;
        value32 = statByRouteId[index].GetCount(); 
        value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

        statByRouteId[index].First();
        char* routeId = 0;
        CommonStat* routeStat = 0;
        while (statByRouteId[index].Next(routeId, routeStat))
        {
            if (!routeStat || !routeId || routeId[0] == '\0') continue;
          
            __trace2__("routeid=%s",routeId);
            // Writes length of routeId and routId
            uint8_t routIdLen = (uint8_t)strlen(routeId);
            buff.Append((uint8_t *)&routIdLen, sizeof(routIdLen));
            buff.Append((uint8_t *)routeId, routIdLen);
            // Writes rout statistics for this routId
            value32 = htonl(routeStat->providerId);         buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->accepted);           buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->rejected);           buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->delivered);          buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->gw_rejected);        buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->failed);             buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->billingOk);          buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->billingFailed);      buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->recieptOk);          buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->recieptFailed);      buff.Append((uint8_t *)&value32, sizeof(value32));
            printf("p: %d, a: %d, r: %d, d: %d, gw_r: %d, f: %d, bo: %d, bf:%d, ro: %d, rf: %d\n", routeStat->providerId,
                   routeStat->accepted, routeStat->rejected, routeStat->delivered, routeStat->gw_rejected, routeStat->failed,
                   routeStat->billingOk, routeStat->billingFailed, routeStat->recieptOk, routeStat->recieptFailed);

            // Writes route errors count.
            value32 = routeStat->errors.Count(); 
            value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));
            IntHash<int>::Iterator rit = routeStat->errors.First();
            int recError, reCounter;
            while (rit.Next(recError, reCounter))
            {
                // Statistics for this errors
                value32 = htonl(recError);  buff.Append((uint8_t *)&value32, sizeof(value32));
                value32 = htonl(reCounter); buff.Append((uint8_t *)&value32, sizeof(value32));
                //printf("e: %d, c: %d\n", recError, reCounter);
            }
            routeStat = 0;
        }

        // Sme statistics
        value32 = statBySmeId[index].GetCount(); 
        value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

        statBySmeId[index].First();
        char* smeId = 0;
        CommonStat* smeStat = 0;
        while (statBySmeId[index].Next(smeId, smeStat))
        {
            if (!smeStat || !smeId || smeId[0] == '\0') continue;
            
            int cnt=2;
            // Writes length of smeId and smeId
            uint8_t smeIdLen = (uint8_t)strlen(smeId);
            buff.Append((uint8_t *)&smeIdLen, sizeof(smeIdLen));
            buff.Append((uint8_t *)smeId, smeIdLen);
            // Writes sme statistics for this smeId
            value32 = htonl(smeStat->providerId);           buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->accepted);             buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->rejected);             buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->delivered);            buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->gw_rejected);          buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->failed);               buff.Append((uint8_t *)&value32, sizeof(value32));

            // Writes sme common errors count.
            value32 = smeStat->errors.Count(); 
            value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

            IntHash<int>::Iterator sit = smeStat->errors.First();
            int secError, seCounter;
            while (sit.Next(secError, seCounter))
            {
                // Statistics for this errors
                value32 = htonl(secError);  buff.Append((uint8_t *)&value32, sizeof(value32));
                value32 = htonl(seCounter); buff.Append((uint8_t *)&value32, sizeof(value32));
            }
            smeStat = 0;
        }

        // Smsc statistics
        value32 = srvStatBySmeId[index].GetCount(); 
        value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

        srvStatBySmeId[index].First();
        smeId = 0;
        CommonStat* srvStat = 0;
        while (srvStatBySmeId[index].Next(smeId, srvStat))
        {
            if (!srvStat || !smeId || smeId[0] == '\0') continue;
            
            int cnt=2;
            // Writes length of smeId and smeId
            uint8_t smeIdLen = (uint8_t)strlen(smeId);
            buff.Append((uint8_t *)&smeIdLen, sizeof(smeIdLen));
            buff.Append((uint8_t *)smeId, smeIdLen);
            // Writes sme statistics for this smeId
            value32 = htonl(srvStat->providerId);           buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->accepted);             buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->rejected);             buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->delivered);            buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->gw_rejected);          buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->failed);               buff.Append((uint8_t *)&value32, sizeof(value32));

            // Writes sme common errors count.
            value32 = srvStat->errors.Count(); 
            value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

            IntHash<int>::Iterator sit = srvStat->errors.First();
            int secError, seCounter;
            while (sit.Next(secError, seCounter))
            {
                // Statistics for this errors
                value32 = htonl(secError);  buff.Append((uint8_t *)&value32, sizeof(value32));
                value32 = htonl(seCounter); buff.Append((uint8_t *)&value32, sizeof(value32));
            }
            srvStat = 0;
        }

        dumpCounters(buff, buff.GetPos(), flushTM);

    }
    catch (Exception& exc)
    {
        smsc_log_error(logger, "Statistics flush failed. Cause: %s", exc.what());
    }

    resetCounters(index);

}

void StatisticsManager::flushHttpCounters(int index)
{

    tm flushTM; calculateTime(flushTM);
    smsc_log_debug(logger, "Flushing HTTP Statistics for %02d.%02d.%04d %02d:%02d:%02d GMT",
                   flushTM.tm_mday, flushTM.tm_mon+1, flushTM.tm_year+1900,
                   flushTM.tm_hour, flushTM.tm_min, flushTM.tm_sec);

    try
    {
        TmpBuf<uint8_t, 4096> buff(4096);

        // Head of record
        uint8_t value8 = 0;
        value8 = (uint8_t)(flushTM.tm_hour); buff.Append((uint8_t *)&value8, sizeof(value8));
        value8 = (uint8_t)(flushTM.tm_min);  buff.Append((uint8_t *)&value8, sizeof(value8));

        // Route statistic
        int32_t value32 = 0;
        value32 = httpStatByRouteId[index].GetCount(); 
        value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

        httpStatByRouteId[index].First();
        char* routeId = 0;
        HttpStat* routeStat = 0;
        while (httpStatByRouteId[index].Next(routeId, routeStat))
        {
            if (!routeStat || !routeId || routeId[0] == '\0') continue;
          
            __trace2__("routeid=%s",routeId);
            // Writes length of routeId and routId
            uint8_t routIdLen = (uint8_t)strlen(routeId);
            buff.Append((uint8_t *)&routIdLen, sizeof(routIdLen));
            buff.Append((uint8_t *)routeId, routIdLen);
            // Writes rout statistics for this routId
            value32 = htonl(routeStat->providerId);         buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->request);            buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->requestRejected);    buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->response);           buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->responseRejected);   buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->delivered);          buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->failed);             buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->billingOk);          buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->billingFailed);      buff.Append((uint8_t *)&value32, sizeof(value32));                                                      
            /*printf("p: %d, req: %d, req_r: %d, res: %d, res_r: %d, d: %d, f: %d, bo:%d, bf: %d\n", routeStat->providerId,
                   routeStat->request, routeStat->requestRejected, routeStat->response, routeStat->responseRejected, routeStat->delivered,
                   routeStat->failed, routeStat->billingOk, routeStat->billingFailed);*/

            // Writes route errors count.
            value32 = routeStat->errors.Count(); 
            value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));
            IntHash<int>::Iterator rit = routeStat->errors.First();
            int recError, reCounter;
            while (rit.Next(recError, reCounter))
            {
                // Statistics for this errors
                value32 = htonl(recError);  buff.Append((uint8_t *)&value32, sizeof(value32));
                value32 = htonl(reCounter); buff.Append((uint8_t *)&value32, sizeof(value32));
                //printf("e: %d, c: %d\n", recError, reCounter);
            }
            routeStat = 0;
        }

        // Service statistics
        value32 = httpStatByServiceId[index].GetCount(); 
        value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

        httpStatByServiceId[index].First();
        char* srvId = 0;
        HttpStat* srvStat = 0;
        while (httpStatByServiceId[index].Next(srvId, srvStat))
        {
            if (!srvStat || !srvId || srvId[0] == '\0') continue;
            
            int cnt=2;
            // Writes length of smeId and smeId
            uint8_t srvIdLen = (uint8_t)strlen(srvId);
            buff.Append((uint8_t *)&srvIdLen, sizeof(srvIdLen));
            buff.Append((uint8_t *)srvId, srvIdLen);
            // Writes sme statistics for this smeId
            value32 = htonl(srvStat->providerId);         buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->request);            buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->requestRejected);    buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->response);           buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->responseRejected);   buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->delivered);          buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->failed);             buff.Append((uint8_t *)&value32, sizeof(value32));

            // Writes sme common errors count.
            value32 = srvStat->errors.Count(); 
            value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

            IntHash<int>::Iterator sit = srvStat->errors.First();
            int secError, seCounter;
            while (sit.Next(secError, seCounter))
            {
                // Statistics for this errors
                value32 = htonl(secError);  buff.Append((uint8_t *)&value32, sizeof(value32));
                value32 = htonl(seCounter); buff.Append((uint8_t *)&value32, sizeof(value32));
            }
            srvStat = 0;
        }

        // Provider statistics
        value32 = httpStatByProviderId[index].Count(); 
        value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

        IntHash<HttpStat>::Iterator stIter = httpStatByProviderId[index].First();
        int pId = -1;
        HttpStat pStat;
        while (stIter.Next(pId, pStat))
        {
            if (pId != -1) continue;
            
            int cnt=2;
            // Writes length of smeId and smeId
            value32 = htonl(pId); buff.Append((uint8_t *)&value32, sizeof(value32));
            // Writes sme statistics for this smeId
            value32 = htonl(pStat.request);            buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(pStat.requestRejected);    buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(pStat.response);           buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(pStat.responseRejected);   buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(pStat.delivered);          buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(pStat.failed);             buff.Append((uint8_t *)&value32, sizeof(value32));

            // Writes sme common errors count.
            value32 = pStat.errors.Count(); 
            value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

            IntHash<int>::Iterator sit = pStat.errors.First();
            int secError, seCounter;
            while (sit.Next(secError, seCounter))
            {
                // Statistics for this errors
                value32 = htonl(secError);  buff.Append((uint8_t *)&value32, sizeof(value32));
                value32 = htonl(seCounter); buff.Append((uint8_t *)&value32, sizeof(value32));
            }
        }

        dumpHttpCounters(buff, buff.GetPos(), flushTM);

    }
    catch (Exception& exc)
    {
        smsc_log_error(logger, "Http Statistics flush failed. Cause: %s", exc.what());
    }

    resetHttpCounters(index);

}

void StatisticsManager::resetCounters(int index)
{
  statBySmeId[index].Empty();
  statByRouteId[index].Empty();
  srvStatBySmeId[index].Empty();
}

void StatisticsManager::resetHttpCounters(int index)
{
  httpStatByRouteId[index].Empty();
  httpStatByServiceId[index].Empty();
  httpStatByProviderId[index].Empty();
}

void StatisticsManager::Fseek(long offset, int whence, FILE* &cfPtr)
{
    if (cfPtr && fseek(cfPtr, offset, whence)) {
        int error = ferror(cfPtr);
        Exception exc("Failed to seek file. Details: %s", strerror(error));
        Fclose(cfPtr); throw exc;
    }
}

void StatisticsManager::Fclose(FILE* &cfPtr)
{
    if (cfPtr) { 
        fclose(cfPtr); cfPtr = 0;
    }
}

void StatisticsManager::Fopen(FILE* &cfPtr, const std::string loc)
{
    cfPtr = fopen(loc.c_str(), "ab+"); // open or create for update
    if (!cfPtr)
        throw Exception("Failed to create/open file '%s'. Details: %s", 
                            loc.c_str(), strerror(errno));
}

void StatisticsManager::Fflush(FILE* &cfPtr)
{
    if (cfPtr && fflush(cfPtr)) {
        int error = ferror(cfPtr);
        Exception exc("Failed to flush file. Details: %s", strerror(error));
        Fclose(cfPtr); throw exc;
    }
}

size_t StatisticsManager::Fread(FILE* &cfPtr, void* data, size_t size)
{
    int read_size = fread(data, size, 1, cfPtr);

    if( (read_size != 1) && (read_size != 0)){
        int err = ferror(cfPtr);
        Fclose(cfPtr);
        throw Exception("Can't read route from file. Details: %s", strerror(err));
    }

    return read_size;
}

void StatisticsManager::Fwrite(const void* data, size_t size, FILE* &cfPtr)
{
    if (cfPtr && fwrite(data, size, 1, cfPtr) != 1) {
        int error = ferror(cfPtr);
        Exception exc("Failed to write file. Details: %s", strerror(error));
        Fclose(cfPtr); throw exc;
    }
}

bool StatisticsManager::createDir(const std::string& dir)
{
    if (mkdir(dir.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0) {
        if (errno == EEXIST) return false;
        throw Exception("Failed to create directory '%s'. Details: %s", 
                        dir.c_str(), strerror(errno));
    }
    return true;
}

bool StatisticsManager::createStorageDir(const std::string loc)
{
    const char * dir_ = loc.c_str();

    int len = strlen(dir_);
    if(len == 0)
        return false;

    if(strcmp(dir_, "/") == 0)
        return true;

    ++len;

    TmpBuf<char, 512> tmpBuff(len);
    char * buff =tmpBuff.get();
    memcpy(buff, dir_, len);

    if(buff[len-2] == '/'){
       buff[len-2] = 0;
       if(len > 2){
          if(buff[len-3] == '/'){
              return false;
           }
       }
    }

    std::vector<std::string> dirs(0);

    char* p1 = buff+1;
    int dirlen = 0;
    char* p2 = strchr(p1, '/');
    int pos = p2 - buff;
    while(p2){
       int len = p2 - p1;
       dirlen += len + 1;
       if(len == 0)
           return false;

       int direclen = dirlen + 1;
       TmpBuf<char, 512> tmpBuff(direclen);
       char * dir = tmpBuff.get();
       memcpy(dir, buff, dirlen);
       dir[dirlen] = 0;
       dirs.push_back(std::string(dir));

       p1 = p1 + len + 1;
       p2 = strchr(p1, '/');
    }
    dirs.push_back(std::string(buff));

    for(std::vector<std::string>::iterator it = dirs.begin(); it != dirs.end(); it++){

        DIR* dirp = opendir( (*it).c_str() );
        if(dirp){
            closedir(dirp);            
        }else{
            try{
                createDir(std::string( (*it).c_str() ));
            }catch(...){
                return false;
            }
        }
    }
    
    return true;
}

void StatisticsManager::dumpCounters(const uint8_t* buff, int buffLen, const tm& flushTM)
{
    smsc_log_debug(logger, "Statistics dump called for %02d:%02d GMT", 
                   flushTM.tm_hour, flushTM.tm_min);

    try {

    char dirName[128]; bool hasDir = false;

    if (!bFileTM || fileTM.tm_mon != flushTM.tm_mon || fileTM.tm_year != flushTM.tm_year)
    {
        sprintf(dirName, SCAG_SMPP_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
        smsc_log_debug(logger, "dirName: %s", dirName);
        createDir(location + "/" + dirName); bFileTM = false; hasDir = true;
        smsc_log_debug(logger, "New dir '%s' created", dirName);
    }

    if (!bFileTM || fileTM.tm_mday != flushTM.tm_mday)
    {
        char fileName[128]; 
        std::string fullPath = location;
        if (!hasDir) sprintf(dirName, SCAG_SMPP_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
        sprintf(fileName, SCAG_STAT_FILE_NAME_FORMAT, flushTM.tm_mday);
        fullPath += '/'; fullPath += (const char*)dirName; 
        fullPath += '/'; fullPath += (const char*)fileName; 
        const char* fullPathStr = fullPath.c_str();

        if (file.isOpened()) file.Close();
            
        bool needHeader = true;
        if (File::Exists(fullPathStr)) { 
            needHeader = false;
            file.WOpen(fullPathStr);
        } else {
            file.RWCreate(fullPathStr);
        }

        if (needHeader) { // create header (if new file created)
            file.Write(SCAG_STAT_HEADER_TEXT, strlen(SCAG_STAT_HEADER_TEXT));
            uint16_t version = htons(SCAG_STAT_VERSION_INFO);
            file.Write(&version, sizeof(version));
            file.Flush();
        }
        fileTM = flushTM; bFileTM = true;
        smsc_log_debug(logger, "%s file '%s' %s", (needHeader) ? "New":"Existed",
                       fileName, (needHeader) ? "created":"opened");
    }
    
    smsc_log_debug(logger, "Statistics data dump...");
    uint32_t value32 = htonl(buffLen);
    file.Write((const void *)&value32, sizeof(value32));
    file.Write((const void *)buff, buffLen); // write dump to it
    file.Write((const void *)&value32, sizeof(value32));
    file.Flush();
    smsc_log_debug(logger, "Statistics data dumped.");

    }catch(std::exception & exc){
        if (file.isOpened()) file.Close();
        bFileTM = false;
        throw exc;
    }
}

void StatisticsManager::dumpHttpCounters(const uint8_t* buff, int buffLen, const tm& flushTM)
{
    smsc_log_debug(logger, "Statistics dump called for %02d:%02d GMT", 
                   flushTM.tm_hour, flushTM.tm_min);

    try {

    char dirName[128]; bool hasDir = false;

    if (!httpIsFileTM || httpFileTM.tm_mon != flushTM.tm_mon || httpFileTM.tm_year != flushTM.tm_year)
    {
        sprintf(dirName, SCAG_HTTP_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
        createDir(location + "/" + dirName); httpIsFileTM = false; hasDir = true;
        smsc_log_debug(logger, "New dir '%s' created", dirName);
    }

    if (!httpIsFileTM || httpFileTM.tm_mday != flushTM.tm_mday)
    {
        char fileName[128]; 
        std::string fullPath = location;
        if (!hasDir) sprintf(dirName, SCAG_HTTP_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
        sprintf(fileName, SCAG_STAT_FILE_NAME_FORMAT, flushTM.tm_mday);
        fullPath += '/'; fullPath += (const char*)dirName; 
        fullPath += '/'; fullPath += (const char*)fileName; 
        const char* fullPathStr = fullPath.c_str();

        if (httpFile.isOpened()) httpFile.Close();
            
        bool needHeader = true;
        if (File::Exists(fullPathStr)) { 
            needHeader = false;
            httpFile.WOpen(fullPathStr);
        } else {
            httpFile.RWCreate(fullPathStr);
        }

        if (needHeader) { // create header (if new file created)
            httpFile.Write(SCAG_STAT_HEADER_TEXT, strlen(SCAG_STAT_HEADER_TEXT));
            uint16_t version = htons(SCAG_STAT_VERSION_INFO);
            httpFile.Write(&version, sizeof(version));
            httpFile.Flush();
        }
        httpFileTM = flushTM; httpIsFileTM = true;
        smsc_log_debug(logger, "%s file '%s' %s", (needHeader) ? "New":"Existed",
                       fileName, (needHeader) ? "created":"opened");
    }
    
    smsc_log_debug(logger, "Http Statistics data dump...");
    uint32_t value32 = htonl(buffLen);
    httpFile.Write((const void *)&value32, sizeof(value32));
    httpFile.Write((const void *)buff, buffLen); // write dump to it
    httpFile.Write((const void *)&value32, sizeof(value32));
    httpFile.Flush();
    smsc_log_debug(logger, "Http Statistics data dumped.");

    }catch(std::exception & exc){
        if (httpFile.isOpened()) httpFile.Close();
        httpIsFileTM = false;
        throw exc;
    }
}

void StatisticsManager::flushTraffic()
{
    IntHash<TrafficRecord> traff;
    {
        MutexGuard mg(switchLock);
        traff = trafficByRouteId;
    }

    std::string path = traffloc + std::string("/SMPP/"); 
    dumpTraffic(traff, path);
}

void StatisticsManager::flushHttpTraffic()
{
    IntHash<TrafficRecord> traff;
    {
        MutexGuard mg(switchLock);
        traff = trafficByRouteId;
    }

    std::string path = traffloc + std::string("/HTTP/"); 
    dumpTraffic(traff, path);
}

void StatisticsManager::dumpTraffic(const IntHash<TrafficRecord>& traff, const std::string& path)
{
    const int pos = smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH + 1;
    const int sz = pos + 21;
    char buff[sz];
    TmpBuf<uint8_t, 27648> fbuff(27648);
    long mincnt_, hourcnt_, daycnt_, monthcnt_; 
    uint8_t year_, month_, day_, hour_, min_;

    IntHash<TrafficRecord>::Iterator traffit = traff.First();
    int routeId = 0;
    TrafficRecord routeTraff;
    while (traffit.Next(routeId, routeTraff))
    {
        // Copies routeId
        const char * id = routeMap.getStrRouteId(routeId);
        memcpy((void*)buff, (const void*)(id), pos);
        routeTraff.getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);

        // Copies counters
        uint32_t lval = mincnt_;
        uint32_t val = htonl(lval);
        memcpy((void*)(buff + pos), (const void*)(&val), 4);
        lval = hourcnt_;
        val = htonl(lval);
        memcpy((void*)(buff + pos + 4), (const void*)(&val), 4);
        lval = daycnt_;
        val = htonl(lval);
        memcpy((void*)(buff + pos + 8), (const void*)(&val), 4);
        lval = monthcnt_;
        val = htonl(lval);
        memcpy((void*)(buff + pos + 12), (const void*)(&val), 4);

        // Copies date
        memcpy((void*)(buff + pos + 16), (const void*)(&year_), 1);
        memcpy((void*)(buff + pos + 17), (const void*)(&month_), 1);
        memcpy((void*)(buff + pos + 18), (const void*)(&day_), 1);
        memcpy((void*)(buff + pos + 19), (const void*)(&hour_), 1);
        memcpy((void*)(buff + pos + 20), (const void*)(&min_), 1);

        fbuff.Append((uint8_t *)buff, sz);
    }

    if(fbuff.GetPos() > 0){
        try {
            std::string loc = path + "traffic.tmp";     
            File tfile;
            tfile.WOpen(loc.c_str());
            tfile.Write(fbuff, fbuff.GetPos());
            tfile.Close();
            std::string traffpath = path + "traffic.dat";
            rename(loc.c_str(), traffpath.c_str());
        }catch(FileException & e){
            smsc_log_warn(logger, "Failed to dump traffic. Detailes: %s", e.what());
        }catch(...){
            smsc_log_warn(logger, "Failed to dump traffic. Unknown error.");
        }
    }else{
        std::string traffpath = path + "traffic.dat";
        remove(traffpath.c_str());
    }
    

}

void StatisticsManager::initTraffic()
{

    const std::string loc = traffloc + std::string("/SMPP/") + "traffic.dat";

    File tfile;

    try {

    tfile.ROpen(loc.c_str());

    const int pos = smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH + 1;
    const int sz = pos + 21;
    char buff[sz];
    char routeId[pos];
    uint32_t val;
    uint8_t year, month, day, hour, min;
    int id;
    long mincnt, hourcnt, daycnt, monthcnt;
    time_t now = time(0);
    tm tmnow;  localtime_r(&now, &tmnow);

    int read_size = -1;
    while( (read_size = tfile.Read(buff, sz)) == 1){

        // Copies routeId
        memcpy((void*)routeId, (const void*)buff, pos);
        if(routeMap.regRoute(routeId, id) != -1)
            continue;

        // Coies counters
        memcpy((void*)&val, (const void*)(buff + pos), 4);
        mincnt = ntohl(val);
        memcpy((void*)&val, (const void*)(buff + pos + 4), 4);
        hourcnt = ntohl(val);
        memcpy((void*)&val, (const void*)(buff + pos + 8), 4);
        daycnt = ntohl(val);
        memcpy((void*)&val, (const void*)(buff + pos + 12), 4);
        monthcnt = ntohl(val);

        // Copies date
        memcpy((void*)&year,    (const void*)(buff + pos + 16), 1);
        memcpy((void*)&month,   (const void*)(buff + pos + 17), 1);
        memcpy((void*)&day,     (const void*)(buff + pos + 18), 1);
        memcpy((void*)&hour,    (const void*)(buff + pos + 19), 1);
        memcpy((void*)&min,     (const void*)(buff + pos + 20), 1);

        TrafficRecord tr(mincnt, hourcnt, daycnt, monthcnt, 
                         year, month, day, hour, min);

        if(tmnow.tm_year != year)
            continue;
        if(tmnow.tm_mon != month)
            continue;

        /*long mincnt_, hourcnt_, daycnt_, monthcnt_; 
        uint8_t year_, month_, day_, hour_, min_;

        tr.getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);
        printf("%04d-%02d-%02d %02d:%02d (m, h, d, M): %d, %d, %d, %d\n", year_ + 1900, month_ + 1, day_, hour_, min_, mincnt_, hourcnt_, daycnt_, monthcnt_);*/

        tr.reset(tmnow);

        /*tr.getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);
        printf("%04d-%02d-%02d %02d:%02d (m, h, d, M): %d, %d, %d, %d\n\n", year_ + 1900, month_ + 1, day_, hour_, min_, mincnt_, hourcnt_, daycnt_, monthcnt_);*/

        trafficByRouteId.Insert(id, tr);

    }

    //Fclose(cfPtr);
    tfile.Close();

    }catch(FileException & e)
    {
        smsc_log_warn(logger, "Failed to init traffic. Detailes: %s", e.what());
    }catch(...)
    {
        smsc_log_warn(logger, "Failed to init traffic. Unknown error");
    }
}

void StatisticsManager::initHttpTraffic()
{

    const std::string loc = traffloc + std::string("/HTTP/") + "traffic.dat";

    File tfile;

    try {

    tfile.ROpen(loc.c_str());

    const int pos = smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH + 1;
    const int sz = pos + 21;
    char buff[sz];
    char routeId[pos];
    uint32_t val;
    uint8_t year, month, day, hour, min;
    int id;
    long mincnt, hourcnt, daycnt, monthcnt;
    time_t now = time(0);
    tm tmnow;  localtime_r(&now, &tmnow);

    int read_size = -1;
    while( (read_size = tfile.Read(buff, sz)) == 1){

        // Copies routeId
        memcpy((void*)routeId, (const void*)buff, pos);
        if(routeMap.regRoute(routeId, id) != -1)
            continue;

        // Coies counters
        memcpy((void*)&val, (const void*)(buff + pos), 4);
        mincnt = ntohl(val);
        memcpy((void*)&val, (const void*)(buff + pos + 4), 4);
        hourcnt = ntohl(val);
        memcpy((void*)&val, (const void*)(buff + pos + 8), 4);
        daycnt = ntohl(val);
        memcpy((void*)&val, (const void*)(buff + pos + 12), 4);
        monthcnt = ntohl(val);

        // Copies date
        memcpy((void*)&year,    (const void*)(buff + pos + 16), 1);
        memcpy((void*)&month,   (const void*)(buff + pos + 17), 1);
        memcpy((void*)&day,     (const void*)(buff + pos + 18), 1);
        memcpy((void*)&hour,    (const void*)(buff + pos + 19), 1);
        memcpy((void*)&min,     (const void*)(buff + pos + 20), 1);

        TrafficRecord tr(mincnt, hourcnt, daycnt, monthcnt, 
                         year, month, day, hour, min);

        if(tmnow.tm_year != year)
            continue;
        if(tmnow.tm_mon != month)
            continue;

        /*long mincnt_, hourcnt_, daycnt_, monthcnt_; 
        uint8_t year_, month_, day_, hour_, min_;

        tr.getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);
        printf("%04d-%02d-%02d %02d:%02d (m, h, d, M): %d, %d, %d, %d\n", year_ + 1900, month_ + 1, day_, hour_, min_, mincnt_, hourcnt_, daycnt_, monthcnt_);*/

        tr.reset(tmnow);

        /*tr.getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);
        printf("%04d-%02d-%02d %02d:%02d (m, h, d, M): %d, %d, %d, %d\n\n", year_ + 1900, month_ + 1, day_, hour_, min_, mincnt_, hourcnt_, daycnt_, monthcnt_);*/

        httpTrafficByRouteId.Insert(id, tr);

    }

    tfile.Close();

    }catch(FileException & e)
    {
        smsc_log_warn(logger, "Failed to init http traffic. Detailes: %s", e.what());
    }catch(...)
    {
        smsc_log_warn(logger, "Failed to init http traffic. Unknown error");
    }
}

void StatisticsManager::resetTraffic(const tm& tmDate)
{
    IntHash<TrafficRecord>::Iterator traffit = trafficByRouteId.First();
    int routeId = 0;
    TrafficRecord *routeTraff;
    while (traffit.Next(routeId, routeTraff))
    {
        /*long mincnt_, hourcnt_, daycnt_, monthcnt_; 
        uint8_t year_, month_, day_, hour_, min_;
        printf("\nresetTraffic routeId: %d\n", routeId);
        printf("%04d-%02d-%02d %02d:%02d\n", tmDate.tm_year + 1900, tmDate.tm_mon + 1, tmDate.tm_mday, tmDate.tm_hour, tmDate.tm_min);
        routeTraff->getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);
        printf("%04d-%02d-%02d %02d:%02d (m, h, d, M): %d, %d, %d, %d\n", year_ + 1900, month_ + 1, day_, hour_, min_, mincnt_, hourcnt_, daycnt_, monthcnt_);*/

        routeTraff->reset(tmDate);

        /*routeTraff->getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);
        printf("%04d-%02d-%02d %02d:%02d (m, h, d, M): %d, %d, %d, %d\n\n", year_ + 1900, month_ + 1, day_, hour_, min_, mincnt_, hourcnt_, daycnt_, monthcnt_);*/
    }
}

void StatisticsManager::resetHttpTraffic(const tm& tmDate)
{
    IntHash<TrafficRecord>::Iterator traffit = httpTrafficByRouteId.First();
    int routeId = 0;
    TrafficRecord *routeTraff;
    while (traffit.Next(routeId, routeTraff))
    {
        /*long mincnt_, hourcnt_, daycnt_, monthcnt_; 
        uint8_t year_, month_, day_, hour_, min_;
        printf("\nresetTraffic routeId: %d\n", routeId);
        printf("%04d-%02d-%02d %02d:%02d\n", tmDate.tm_year + 1900, tmDate.tm_mon + 1, tmDate.tm_mday, tmDate.tm_hour, tmDate.tm_min);
        routeTraff->getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);
        printf("%04d-%02d-%02d %02d:%02d (m, h, d, M): %d, %d, %d, %d\n", year_ + 1900, month_ + 1, day_, hour_, min_, mincnt_, hourcnt_, daycnt_, monthcnt_);*/

        routeTraff->reset(tmDate);

        /*routeTraff->getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);
        printf("%04d-%02d-%02d %02d:%02d (m, h, d, M): %d, %d, %d, %d\n\n", year_ + 1900, month_ + 1, day_, hour_, min_, mincnt_, hourcnt_, daycnt_, monthcnt_);*/
    }
}

void StatisticsManager::incRouteTraffic(const int routeId, const tm& tmDate)
{
    TrafficRecord *tr = 0;
    tr = trafficByRouteId.GetPtr(routeId);
    if(tr){
        tr->inc(tmDate);
    }
}

void StatisticsManager::incHttpRouteTraffic(const int routeId, const tm& tmDate)
{
    TrafficRecord *tr = 0;
    tr = httpTrafficByRouteId.GetPtr(routeId);
    if(tr){
        tr->inc(tmDate);
    }
}

void StatisticsManager::incSvcSmppCounter(const char* systemId, int index)
{
    if (!systemId || !systemId[0] || index<0 || index>=PERF_CNT_COUNT) return;

    MutexGuard guard(svcCountersLock);

    SmppPerformanceCounter*  counter = 0;
    SmppPerformanceCounter** smppCounter = svcSmppCounters.GetPtr(systemId);

    if (!smppCounter) counter = new SmppPerformanceCounter();
    else if (*smppCounter) counter = *smppCounter;
    else {
        counter = new SmppPerformanceCounter();
        smppCounter = 0; svcSmppCounters.Delete(systemId);
    }

    counter->counters[index]++;
    if (!counter->slots[index]) counter->slots[index] = newSlotCounter();
    counter->slots[index]->Inc();

    if (!smppCounter) svcSmppCounters.Insert(systemId, counter);
}

void StatisticsManager::incSvcWapCounter(const char* systemId, int index)
{
    if (!systemId || !systemId[0] || index<0 || index>=PERF_HTTP_COUNT) return;

    MutexGuard guard(svcCountersLock);

    HttpPerformanceCounter*  counter = 0;
    HttpPerformanceCounter** httpCounter = svcWapCounters.GetPtr(systemId);

    if (!httpCounter) counter = new HttpPerformanceCounter();
    else if (*httpCounter) counter = *httpCounter;
    else {
        counter = new HttpPerformanceCounter();
        httpCounter = 0; svcWapCounters.Delete(systemId);
    }

    counter->counters[index]++;
    if (!counter->slots[index]) counter->slots[index] = newSlotCounter();
    counter->slots[index]->Inc();

    if (!httpCounter) svcWapCounters.Insert(systemId, counter);
}

void StatisticsManager::incSvcMmsCounter(const char* systemId, int index)
{
    if (!systemId || !systemId[0] || index<0 || index>=PERF_CNT_COUNT) return;

    MutexGuard guard(svcCountersLock);

    SmppPerformanceCounter*  counter = 0;
    SmppPerformanceCounter** smeCounter = svcMmsCounters.GetPtr(systemId);

    if (!smeCounter) counter = new SmppPerformanceCounter();
    else if (*smeCounter) counter = *smeCounter;
    else {
        counter = new SmppPerformanceCounter();
        smeCounter = 0; svcMmsCounters.Delete(systemId);
    }

    counter->counters[index]++;
    if (!counter->slots[index]) counter->slots[index] = newSlotCounter();
    counter->slots[index]->Inc();

    if (!smeCounter) svcMmsCounters.Insert(systemId, counter);
}

void StatisticsManager::incScSmppCounter(const char* systemId, int index)
{
    if (!systemId || !systemId[0] || index<0 || index>=PERF_CNT_COUNT) return;

    MutexGuard guard(scCountersLock);

    SmppPerformanceCounter*  counter = 0;
    SmppPerformanceCounter** smppCounter = scSmppCounters.GetPtr(systemId);

    if (!smppCounter) counter = new SmppPerformanceCounter();
    else if (*smppCounter) counter = *smppCounter;
    else {
        counter = new SmppPerformanceCounter();
        smppCounter = 0; scSmppCounters.Delete(systemId);
    }

    counter->counters[index]++;
    if (!counter->slots[index]) counter->slots[index] = newSlotCounter();
    counter->slots[index]->Inc();

    if (!smppCounter) scSmppCounters.Insert(systemId, counter);
}

void StatisticsManager::incScWapCounter(const char* systemId, int index)
{
    if (!systemId || !systemId[0] || index<0 || index>=PERF_HTTP_COUNT) return;

    MutexGuard guard(scCountersLock);

    HttpPerformanceCounter*  counter = 0;
    HttpPerformanceCounter** httpCounter = scWapCounters.GetPtr(systemId);

    if (!httpCounter) counter = new HttpPerformanceCounter();
    else if (*httpCounter) counter = *httpCounter;
    else {
        counter = new HttpPerformanceCounter();
        httpCounter = 0; scWapCounters.Delete(systemId);
    }

    counter->counters[index]++;
    if (!counter->slots[index]) counter->slots[index] = newSlotCounter();
    counter->slots[index]->Inc();

    if (!httpCounter) scWapCounters.Insert(systemId, counter);
}

void StatisticsManager::incScMmsCounter(const char* systemId, int index)
{
    if (!systemId || !systemId[0] || index<0 || index>=PERF_CNT_COUNT) return;

    MutexGuard guard(scCountersLock);

    SmppPerformanceCounter*  counter = 0;
    SmppPerformanceCounter** smeCounter = scMmsCounters.GetPtr(systemId);

    if (!smeCounter) counter = new SmppPerformanceCounter();
    else if (*smeCounter) counter = *smeCounter;
    else {
        counter = new SmppPerformanceCounter();
        smeCounter = 0; scMmsCounters.Delete(systemId);
    }

    counter->counters[index]++;
    if (!counter->slots[index]) counter->slots[index] = newSlotCounter();
    counter->slots[index]->Inc();

    if (!smeCounter) scMmsCounters.Insert(systemId, counter);
}

void StatisticsManager::reportGenPerformance(PerformanceData * data)
{
    MutexGuard g(genSocketsMutex);
    PerformanceData ld=*data;
    int high,low;

    //ld.size=htonl(sizeof(ld));
    int size = 100;
    ld.size=htonl(size);
    ld.smppCountersNumber=htonl(ld.smppCountersNumber);


    for(int i=0;i<PERF_CNT_COUNT;i++)
    {
      ld.smppCounters[i].lastSecond=htonl(ld.smppCounters[i].lastSecond);
      ld.smppCounters[i].average=htonl(ld.smppCounters[i].average);

      uint64_t tmp=ld.smppCounters[i].total;
      unsigned char *ptr=(unsigned char *)&ld.smppCounters[i].total;
      ptr[0]=(unsigned char)(tmp>>56);
      ptr[1]=(unsigned char)(tmp>>48)&0xFF;
      ptr[2]=(unsigned char)(tmp>>40)&0xFF;
      ptr[3]=(unsigned char)(tmp>>32)&0xFF;
      ptr[4]=(unsigned char)(tmp>>24)&0xFF;
      ptr[5]=(unsigned char)(tmp>>16)&0xFF;
      ptr[6]=(unsigned char)(tmp>>8)&0xFF;
      ptr[7]=(unsigned char)(tmp&0xFF);
    }

    /*ld.httpCountersNumber=htonl(ld.httpCountersNumber);

    for(int i=0;i<PERF_HTTP_COUNT;i++)
    {
      ld.httpCounters[i].lastSecond=htonl(ld.httpCounters[i].lastSecond);
      ld.httpCounters[i].average=htonl(ld.httpCounters[i].average);

      uint64_t tmp=ld.httpCounters[i].total;
      unsigned char *ptr=(unsigned char *)&ld.httpCounters[i].total;
      ptr[0]=(unsigned char)(tmp>>56);
      ptr[1]=(unsigned char)(tmp>>48)&0xFF;
      ptr[2]=(unsigned char)(tmp>>40)&0xFF;
      ptr[3]=(unsigned char)(tmp>>32)&0xFF;
      ptr[4]=(unsigned char)(tmp>>24)&0xFF;
      ptr[5]=(unsigned char)(tmp>>16)&0xFF;
      ptr[6]=(unsigned char)(tmp>>8)&0xFF;
      ptr[7]=(unsigned char)(tmp&0xFF);
    }*/

    ld.uptime=htonl(ld.uptime);
    ld.now=htonl(ld.now);
    ld.sessionCount=htonl(ld.sessionCount);
    //ld.inProcessingCount=htonl(ld.inProcessingCount);
    //ld.inScheduler=htonl(ld.inScheduler);

    for(int i=0;i<genSockets.Count();i++)
    {

      int wr=genSockets[i]->WriteAll((char*)&ld, 100);

      if(wr!=100)
      {
        genSockets[i]->Abort();
        delete genSockets[i];
        genSockets.Delete(i);
        i--;
      }
    }
}

void StatisticsManager::getSmppPerfData(uint64_t *cnt)
{
    genCounters.getSmppCounters(cnt);
}

/*void StatisticsManager::getHttpPerfData(uint64_t *cnt)
{
    genCounters.getHttpCounters(cnt);
}*/

void StatisticsManager::reportSvcPerformance()
{
    uint8_t* data = 0; uint32_t size = 0;
    data = dumpSvcCounters(size);

    if (!data || size < sizeof(uint32_t)) {
        smsc_log_warn(logger, "invalid service performance data (size=%d)", size);
        return;
    }

    MutexGuard g(svcSocketsMutex);
    
    for(int i=0;i<svcSockets.Count();i++)
    {
      int wr=svcSockets[i]->WriteAll((char*)data, size);

      if(wr!=size)
      {
        svcSockets[i]->Abort();
        delete svcSockets[i];
        svcSockets.Delete(i);
        i--;
      }
    }
}

uint8_t* StatisticsManager::dumpSvcCounters(uint32_t& smePerfDataSize)
    {
        MutexGuard guard(svcCountersLock);

        uint32_t smppReclen = sizeof(char)*(smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1)+sizeof(uint16_t)*2*PERF_CNT_COUNT;
        uint32_t httpReclen = sizeof(char)*(smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1)+sizeof(uint16_t)*2*PERF_HTTP_COUNT;
        smePerfDataSize = sizeof(uint32_t)+sizeof(uint32_t)+3*sizeof(uint16_t)+ 
             smppReclen*svcSmppCounters.GetCount()+
             httpReclen*svcWapCounters.GetCount()+
             smppReclen*svcMmsCounters.GetCount();

        uint8_t* data = new uint8_t[smePerfDataSize];
        uint8_t* packet = data; memset(packet, 0, smePerfDataSize);

        // uint32_t     Total packet size
        *((uint32_t*)packet) = htonl(smePerfDataSize-sizeof(uint32_t)); packet += sizeof(uint32_t);
        // queue size
        *((uint32_t*)packet) = htonl(0); packet += sizeof(uint32_t);
        // uint16_t     Services count
        *((uint16_t*)packet) = htons((uint16_t)svcSmppCounters.GetCount()); packet += sizeof(uint16_t);
        
        svcSmppCounters.First();
        char* systemId = 0; SmppPerformanceCounter* smppCounter = 0;
        while (svcSmppCounters.Next(systemId, smppCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (systemId) strncpy((char *)packet, systemId, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;
            
            for (int i=0; i<PERF_CNT_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (smppCounter) ? htons(smppCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (smppCounter && smppCounter->slots[i]) ? smppCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (smppCounter) smppCounter->clear();
        }

        *((uint16_t*)packet) = htons((uint16_t)svcWapCounters.GetCount()); packet += sizeof(uint16_t);

        svcWapCounters.First();
        systemId = 0; HttpPerformanceCounter * httpCounter = 0;
        while (svcWapCounters.Next(systemId, httpCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (systemId) strncpy((char *)packet, systemId, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;
            
            for (int i=0; i<PERF_HTTP_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (httpCounter) ? htons(httpCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (httpCounter && httpCounter->slots[i]) ? httpCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (httpCounter) httpCounter->clear();
        }

        *((uint16_t*)packet) = htons((uint16_t)svcMmsCounters.GetCount()); packet += sizeof(uint16_t);

        svcMmsCounters.First();
        systemId = 0; smppCounter = 0;
        while (svcMmsCounters.Next(systemId, smppCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (systemId) strncpy((char *)packet, systemId, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;
            
            for (int i=0; i<PERF_CNT_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (smppCounter) ? htons(smppCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (smppCounter && smppCounter->slots[i]) ? smppCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (smppCounter) smppCounter->clear();
        }
        
        return data;
};

void StatisticsManager::reportScPerformance()
{
    uint8_t* data = 0; uint32_t size = 0;
    data = dumpScCounters(size);

    if (!data || size < sizeof(uint32_t)) {
        smsc_log_warn(logger, "invalid service performance data (size=%d)", size);
        return;
    }

    MutexGuard g(scSocketsMutex);
    
    for(int i=0;i<scSockets.Count();i++)
    {
      int wr=scSockets[i]->WriteAll((char*)data, size);

      if(wr!=size)
      {
        scSockets[i]->Abort();
        delete scSockets[i];
        scSockets.Delete(i);
        i--;
      }
    }
}

uint8_t* StatisticsManager::dumpScCounters(uint32_t& smePerfDataSize)
    {
        MutexGuard guard(scCountersLock);

        uint32_t smppReclen = sizeof(char)*(smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1)+sizeof(uint16_t)*2*PERF_CNT_COUNT;
        uint32_t httpReclen = sizeof(char)*(smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1)+sizeof(uint16_t)*2*PERF_HTTP_COUNT;
        smePerfDataSize = sizeof(uint32_t)+sizeof(uint32_t)+3*sizeof(uint16_t)+ 
             smppReclen*scSmppCounters.GetCount()+
             httpReclen*scWapCounters.GetCount()+
             smppReclen*scMmsCounters.GetCount();

        uint8_t* data = new uint8_t[smePerfDataSize];
        uint8_t* packet = data; memset(packet, 0, smePerfDataSize);

        // uint32_t     Total packet size
        *((uint32_t*)packet) = htonl(smePerfDataSize-sizeof(uint32_t)); packet += sizeof(uint32_t);
        // queue size
        *((uint32_t*)packet) = htonl(0); packet += sizeof(uint32_t);
        // uint16_t     Smpp count
        *((uint16_t*)packet) = htons((uint16_t)scSmppCounters.GetCount()); packet += sizeof(uint16_t);
        
        scSmppCounters.First();
        char* systemId = 0; SmppPerformanceCounter* smppCounter = 0;
        while (scSmppCounters.Next(systemId, smppCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (systemId) strncpy((char *)packet, systemId, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;
            
            for (int i=0; i<PERF_CNT_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (smppCounter) ? htons(smppCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (smppCounter && smppCounter->slots[i]) ? smppCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (smppCounter) smppCounter->clear();
        }

        *((uint16_t*)packet) = htons((uint16_t)scWapCounters.GetCount()); packet += sizeof(uint16_t);

        scWapCounters.First();
        systemId = 0; HttpPerformanceCounter * httpCounter = 0;
        while (scWapCounters.Next(systemId, httpCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (systemId) strncpy((char *)packet, systemId, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;
            
            for (int i=0; i<PERF_HTTP_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (httpCounter) ? htons(httpCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (httpCounter && httpCounter->slots[i]) ? httpCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (httpCounter) httpCounter->clear();
        }

        *((uint16_t*)packet) = htons((uint16_t)scMmsCounters.GetCount()); packet += sizeof(uint16_t);

        scMmsCounters.First();
        systemId = 0; smppCounter = 0;
        while (scMmsCounters.Next(systemId, smppCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (systemId) strncpy((char *)packet, systemId, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;
            
            for (int i=0; i<PERF_CNT_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (smppCounter) ? htons(smppCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (smppCounter && smppCounter->slots[i]) ? smppCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (smppCounter) smppCounter->clear();
        }
        
        return data;
};

void StatisticsManager::addSvcSocket(Socket * socket)
{
    MutexGuard g(svcSocketsMutex);
    socket->setNonBlocking(1);
    char buf[32];
    socket->GetPeer(buf);
    smsc_log_info(logger, "performance::add connect from %s", buf);
    svcSockets.Push(socket);
}

void StatisticsManager::addScSocket(Socket * socket)
{
    MutexGuard g(scSocketsMutex);
    socket->setNonBlocking(1);
    char buf[32];
    socket->GetPeer(buf);
    smsc_log_info(logger, "performance::add connect from %s", buf);
    scSockets.Push(socket);
}

void StatisticsManager::addGenSocket(Socket * socket)
{
    MutexGuard g(genSocketsMutex);
    socket->setNonBlocking(1);
    char buf[32];
    socket->GetPeer(buf);
    smsc_log_info(logger, "performance::add connect from %s", buf);
    genSockets.Push(socket);
}

int StatisticsManager::indexByCounter(int counter)
{

  using namespace Counters;

  switch(counter)
  {
  case cntAccepted:
      return PERF_CNT_ACCEPTED;
  case cntRejected:
      return PERF_CNT_REJECTED;
  case cntDelivered:
      return PERF_CNT_DELIVERED;
  case cntGw_Rejected:
      return PERF_CNT_GW_REJECTED;
  case cntFailed:
      return PERF_CNT_FAILED;
  }

  return -1;
}

int StatisticsManager::indexByHttpCounter(int counter)
{

  using namespace Counters;

  switch(counter)
  {
  case httpRequest:
      return PERF_HTTP_REQUEST;
  case httpRequestRejected:
      return PERF_HTTP_REQUEST_REJECTED;
  case httpResponse:
      return PERF_HTTP_RESPONSE;
  case httpResponseRejected:
      return PERF_HTTP_RESPONSE_REJECTED;
  case httpDelivered:
      return PERF_HTTP_DELIVERED;
  case httpFailed:
      return PERF_HTTP_FAILED;
  }

  return -1;
}

}//namespace stat
}//namespace scag
