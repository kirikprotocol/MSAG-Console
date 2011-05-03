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

#include <string.h>
#include <util/BufferSerialization.hpp>
#include "StatisticsManager.h"
#include "scag/config/base/ConfigManager2.h"

// #include "scag/util/properties/Properties2.h"
// using namespace util::properties;

using namespace scag::stat::sacc;
using namespace smsc::core::buffers;

namespace scag2 {
namespace stat {

    namespace Counters
    {
        typedef enum 
        {
          cntAccepted,
          cntRejected,
          cntDelivered,
          cntGw_Rejected,
          cntFailed,

          cntRecieptOk,
          cntRecieptFailed
        } SmppStatCounter;

        typedef enum
        {
          httpRequest,
          httpRequestRejected,
          httpResponse,
          httpResponseRejected,
          httpDelivered,
          httpFailed
        } HttpStatCounter;
    }

using smsc::core::buffers::TmpBuf;
using smsc::core::buffers::File;
using scag::config::StatManConfig;

const uint16_t SCAG_STAT_DUMP_INTERVAL = 60; // in seconds
const uint16_t SCAG_STAT_VERSION_INFO  = 0x0001;
const char*    SCAG_STAT_HEADER_TEXT   = "SCAG.STAT";
const char*    SCAG_SMPP_STAT_DIR_NAME_FORMAT  = "SMPP/%04d-%02d";
const char*    SCAG_HTTP_STAT_DIR_NAME_FORMAT = "HTTP/%04d-%02d";
const char*    SCAG_STAT_FILE_NAME_FORMAT = "%02d.rts";

//typedef SingletonHolder<StatisticsManager, CreateUsingNew, SingletonWithLongevity> SingleSM;

/*
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
*/

void StatisticsManager::init( const StatManConfig& statManConfig )
{
    /*
    if (!StatisticsManager::inited)
    {
        MutexGuard guard(StatisticsManager::initLock);
        if (!StatisticsManager::inited) {
            StatisticsManager& sm = SingleSM::Instance();
     */
    configure(statManConfig);
    //            sm.Start();
    // StatisticsManager::inited = true;
    // }
    // }
}


void StatisticsManager::configChanged()
{
    MutexGuard  switchGuard(switchLock);
    Stop();
    configure( ConfigManager::Instance().getStatManConfig() );
    Start();
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


    smsc_log_debug(logger,"SACC configuration perfom to start");
    int saccPort = statManConfig.getSaccPort();
    std::string saccHost = statManConfig.getSaccHost();
    int reconnect_timeout = statManConfig.getReconnectTimeout();
    int queuelen = statManConfig.getMaxQueueLength();
    bool enabled = statManConfig.getEnabled();
    thrSaccSender.init(saccHost, saccPort, reconnect_timeout, queuelen, logger, enabled);
    
    smsc_log_debug(logger, "StatisticsManager, perfSvcPort: %d\n", perfSvcPort);

    sender.init((PerformanceListener*)this, (PerformanceServer*)this);
    sender.InitServer(perfHost, perfGenPort, perfSvcPort, perfScPort);

    initTraffic(smppTrafficByRouteId, traffloc + std::string("/SMPP/") + "traffic.dat");
    initTraffic(httpTrafficByRouteId, traffloc + std::string("/HTTP/") + "traffic.dat");
    statLogger.reset(new StatLogger(statManConfig.getDir(), statManConfig.getSaaDir(),
                                    statManConfig.getFilesPrefix(), statManConfig.getRollingInterval()));
    statRoller.reset(new StatRoller(statLogger.get()));
}


StatisticsManager::StatisticsManager() :
Statistics(), Thread(), ConfigListener(STATMAN_CFG),
logger(Logger::getInstance("statman")),
currentIndex(0), bExternalFlush(false), isStarted(false),
genStatSmpp(PERF_CNT_COUNT), genStatHttp(PERF_HTTP_COUNT)
{
    memset(&smppFileTM, 0, sizeof(smppFileTM));
    memset(&httpFileTM, 0, sizeof(httpFileTM));
}

StatisticsManager::~StatisticsManager()
{
  /*Logger::Init();
  logger = Logger::getInstance("scag.stat.StatisticsManager");
  sender.reinitPrfSrvLogger();*/

    Stop();
    WaitFor();
    smppFile.Close();
    httpFile.Close();

    deleteSockets( svcSocketsMutex, svcSockets );
    deleteSockets( scSocketsMutex, scSockets );
    deleteSockets( genSocketsMutex, genSockets );
    {
        MutexGuard mg(svcCountersLock);
        deleteCommonPerfCounters(svcSmppCounters);
        deleteCommonPerfCounters(svcWapCounters);
        deleteCommonPerfCounters(svcMmsCounters);
    }
    {
        MutexGuard mg(scCountersLock);
        deleteCommonPerfCounters(scSmppCounters);
        deleteCommonPerfCounters(scWapCounters);
        deleteCommonPerfCounters(scMmsCounters);
    }
    smsc_log_info( logger, "Statistics manager is released" );
}

void StatisticsManager::incError(IntHash<int>& hash, int errcode)
{
    if(errcode == -1) return;

    int* counter = hash.GetPtr(errcode);
    if (!counter) hash.Insert(errcode, 1);
    else (*counter)++;
}

CommonStat* StatisticsManager::getStat(const char* id, bool sc)
{
    CommonStat* st = NULL;
    Hash<CommonStat>* h = sc ? &srvStatBySmeId[currentIndex] : &statBySmeId[currentIndex];

    if(id && id[0])
    {
        st = h->GetPtr(id);
        if(!st)
        {
            CommonStat newStat;
            h->Insert(id, newStat);
            st = h->GetPtr(id);
        }
    }
    return st;
}

void StatisticsManager::deleteSockets( Mutex& mt, std::vector< Socket* >& socks )
{
    MutexGuard mg(mt);
    for ( std::vector< Socket* >::const_iterator i = socks.begin();
          i != socks.end();
          ++i ) {
        if ( *i ) {
            (*i)->Abort();
            delete *i;
        }
    }
    socks.clear();
}

void StatisticsManager::deleteCommonPerfCounters( Hash< CommonPerformanceCounter* >& counters )
{
    char* id;
    CommonPerformanceCounter* c;
    for ( Hash< CommonPerformanceCounter* >::Iterator i(&counters); i.Next(id,c); ) {
        delete c;
    }
    counters.Empty();
}

void StatisticsManager::registerEvent(const SmppStatEvent& se)
{
  MutexGuard  switchGuard(switchLock);

  CommonStat* srcSt=0;
  CommonStat* routeSt=0;
  CommonStat* dstSt=0;

  using namespace Counters;
  using namespace events::smpp;

  srcSt = getStat(se.srcId, se.srcType);
  dstSt = getStat(se.dstId, se.dstType);

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
  if(routeSt && se.routeProviderId!=-1)routeSt->providerId=se.routeProviderId;

#ifdef MSAG_FAKE_STAT
#define STAT_LOG_EVENT(ev) smsc_log_debug(logger, "StatisticsManager:events::smpp::"ev" %s(%d) %s(%d) %d", se.srcId, se.srcType, se.dstId, se.dstType, routeSt)
#else
#define STAT_LOG_EVENT(ev)
#endif

  switch(se.event)
  {
    case events::smpp::ACCEPTED:
        STAT_LOG_EVENT("ACCEPTED");

        if(srcSt) { srcSt->accepted++; incSmppCounter(se.srcId, se.srcType, cntAccepted); }
        if(routeSt) { routeSt->accepted++; }
        genStatSmpp.inc(cntAccepted);
        srcSt = 0;
        routeSt = 0;
        break;
    case events::smpp::REJECTED:
        STAT_LOG_EVENT("REJECTED");

        if(srcSt) { srcSt->rejected++; incSmppCounter(se.srcId, se.srcType, cntRejected); }
        if(dstSt) { dstSt->failed++; incSmppCounter(se.dstId, se.dstType, cntFailed); }
        if(routeSt) { routeSt->rejected++; routeSt->failed++; }
        genStatSmpp.inc(cntRejected); genStatSmpp.inc(cntFailed);
        break;
    case events::smpp::GW_REJECTED:
        STAT_LOG_EVENT("GW_REJECTED");

        if(srcSt) { srcSt->rejected++; incSmppCounter(se.srcId, se.srcType, cntRejected); 
                    srcSt->gw_rejected++; incSmppCounter(se.srcId, se.srcType, cntGw_Rejected);
                  }
	dstSt = 0; // null to skip error count
        if(routeSt) { routeSt->rejected++; routeSt->gw_rejected++; }
        genStatSmpp.inc(cntRejected); genStatSmpp.inc(cntGw_Rejected);
        break;
    case events::smpp::FAILED:
        STAT_LOG_EVENT("FAILED");
        if(dstSt) { dstSt->failed++; incSmppCounter(se.dstId, se.dstType, cntFailed); }		
        if(routeSt) { routeSt->failed++; }
        genStatSmpp.inc(cntFailed);       
        break;
    case events::smpp::RESP_OK:
        STAT_LOG_EVENT("RESP_OK");
        if(srcSt) { srcSt->delivered++; incSmppCounter(se.srcId, se.srcType, cntDelivered);}
        if(routeSt) { routeSt->delivered++; }

        if (se.routeId && se.routeId[0])
            incRouteTraffic(smppTrafficByRouteId, se.routeId);

        genStatSmpp.inc(cntDelivered);
        break;
    case events::smpp::RESP_FAILED:        
    case events::smpp::RESP_EXPIRED:
        STAT_LOG_EVENT("RESP_EXPIRED");
        if(srcSt) { srcSt->failed++; incSmppCounter(se.srcId, se.srcType, cntFailed);}
        if(dstSt) { dstSt->rejected++; incSmppCounter(se.dstId, se.dstType, cntRejected);}
        if(routeSt) { routeSt->failed++; }

        genStatSmpp.inc(cntFailed);
        break;
    case events::smpp::RESP_REJECTED:
        STAT_LOG_EVENT("RESP_REJECTED");
        if(srcSt) { srcSt->failed++; incSmppCounter(se.srcId, se.srcType, cntFailed);}
        if(dstSt) { dstSt->rejected++; incSmppCounter(se.dstId, se.dstType, cntRejected);}
        if(routeSt) { routeSt->rejected++; }

        genStatSmpp.inc(cntRejected);
        break;
    case events::smpp::RESP_GW_REJECTED:
        STAT_LOG_EVENT("RESP_GW_REJECTED");

        if(dstSt) { dstSt->rejected++; incSmppCounter(se.dstId, se.dstType, cntRejected);
                    dstSt->gw_rejected++; incSmppCounter(se.dstId, se.dstType, cntGw_Rejected);
                  }
        if(routeSt) { routeSt->rejected++; routeSt->gw_rejected++;}

        genStatSmpp.inc(cntRejected); genStatSmpp.inc(cntGw_Rejected);
        break;
    case events::smpp::RECEIPT_OK:
        if(srcSt) { srcSt->recieptOk++; incSmppCounter(se.srcId, se.srcType, cntRecieptOk); }
        if(dstSt) { dstSt->recieptOk++; incSmppCounter(se.dstId, se.dstType, cntRecieptOk); }
        if(routeSt) { routeSt->recieptOk++; }

        genStatSmpp.inc(cntRecieptOk);
        break;
    case events::smpp::RECEIPT_FAILED:
        if(srcSt) { srcSt->recieptFailed++; incSmppCounter(se.srcId, se.srcType, cntRecieptFailed);}
        if(dstSt) { dstSt->recieptFailed++; incSmppCounter(se.dstId, se.dstType, cntRecieptFailed);}
        if(routeSt) { routeSt->recieptFailed++; }

        genStatSmpp.inc(cntRecieptFailed);
        break;
  }
#undef STAT_LOG_EVENT  
  //thrSaccSender.Put(se.sacc_stat);

  if(srcSt) incError(srcSt->errors, se.errCode);
  if(dstSt) incError(dstSt->errors, se.errCode);
  if(routeSt) incError(routeSt->errors, se.errCode);

}

void StatisticsManager::registerEvent(const HttpStatEvent& se)
{
    MutexGuard  switchGuard(switchLock);
    
    HttpStat* routeSt=0;
    HttpStat* urlSt=0;
    
    using namespace Counters;
    
    genStatHttp.inc(indexByHttpCounter(se.event));
    
    if (se.routeId.length())
    {
 //       smsc_log_debug(logger, "routeId: '%s'", se.routeId.c_str());
        routeSt = httpStatByRouteId[currentIndex].GetPtr(se.routeId.c_str());
        if(!routeSt)
        {
            HttpStat newStat;
            httpStatByRouteId[currentIndex].Insert(se.routeId.c_str(), newStat);
            routeSt=httpStatByRouteId[currentIndex].GetPtr(se.routeId.c_str());
        }
    }

    if (se.url.length() > 0)
    {
//        smsc_log_debug(logger, "URL: '%s'", se.url.c_str());
        urlSt = httpStatByUrl[currentIndex].GetPtr(se.url.c_str());
        if(!urlSt)
        {
            HttpStat newStat;
            httpStatByUrl[currentIndex].Insert(se.url.c_str(), newStat);
            urlSt=httpStatByUrl[currentIndex].GetPtr(se.url.c_str());
        }
        incSvcWapCounter(se.site.c_str(), se.event);
    }
    
    if(se.errCode && (se.errCode < 200 || se.errCode > 299) && routeSt)     incError(routeSt->errors, se.errCode);
    if(se.errCode && (se.errCode < 200 || se.errCode > 299) && urlSt)       incError(urlSt->errors, se.errCode);
    
    switch(se.event)
    {
        #define INC_STAT(cnt,field) case cnt:{\
              if(routeSt)routeSt->field++; \
              if(urlSt)urlSt->field++; \
              }break;
    
        INC_STAT(events::http::REQUEST_OK,request)
        INC_STAT(events::http::REQUEST_FAILED,requestRejected)
        INC_STAT(events::http::RESPONSE_OK,response)
        INC_STAT(events::http::RESPONSE_FAILED,responseRejected)
        INC_STAT(events::http::DELIVERED,delivered)
        INC_STAT(events::http::FAILED,failed)

        #undef INC_STAT
    
    }
    
    if (se.routeId.length() && se.event == events::http::REQUEST_OK)
        incRouteTraffic(httpTrafficByRouteId, se.routeId.c_str());

    //thrSaccSender.Put(se.sacc_stat);
}
    
bool StatisticsManager::checkTraffic(std::string routeId, CheckTrafficPeriod period, int64_t value)
{
    MutexGuard mg(switchLock);
    
    TrafficRecord *tr = smppTrafficByRouteId.GetPtr(routeId.c_str());
    if(tr)
    {
        time_t now = time(0);
        tm tmnow;
        localtime_r(&now, &tmnow);
        tr->reset(tmnow);
    
        long mincnt_, hourcnt_, daycnt_, monthcnt_;
        uint8_t year_, month_, day_, hour_, min_;

        tr->getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, year_, month_, day_, hour_, min_);

        switch(period)
        {
            case checkMinPeriod: return mincnt_ <= value;
            case checkHourPeriod: return hourcnt_ <= value;
            case checkDayPeriod: return daycnt_ <= value;
            case checkMonthPeriod: return monthcnt_ <= value;
        }
    }
    return false;
}

int StatisticsManager::Execute()
{
    isStarted = true;     

    while (isStarted)
    {
        int toSleep = calculateToSleep();
        smsc_log_debug(logger, "Execute() >> Start wait %d", toSleep);
        awakeEvent.Wait(toSleep); // Wait for next hour begins ...
        smsc_log_debug(logger, "Execute() >> End wait");

        int flushIndex = switchCounters();
        flushCounters(flushIndex);
        flushHttpCounters(flushIndex);

        dumpTraffic(smppTrafficByRouteId, traffloc + "/SMPP/");
        dumpTraffic(httpTrafficByRouteId, traffloc + "/HTTP/");

        if(bExternalFlush) isStarted = false;
        bExternalFlush = false;
        smsc_log_debug(logger, "Execute() >> Flushed");
    }

    smsc_log_debug(logger, "Execute() exited");
    return 0;
}

void StatisticsManager::Stop()
{

    smsc_log_debug(logger, "PerformanceServer is shutdowninig...");
    sender.Stop();
    smsc_log_debug(logger, "PerformanceServer is shutdowned");

    statRoller->Stop();

    thrSaccSender.Stop();

    if (isStarted)
    {
        bExternalFlush = true;
        awakeEvent.Signal();
    }
}

void StatisticsManager::Start()
{
    smsc_log_debug(logger, "PerformanceServer is starting...");
    sender.Start();
    smsc_log_debug(logger, "PerformanceServer is started");

    thrSaccSender.Start();
    isStarted = true;

    Thread::Start();
}

int StatisticsManager::switchCounters()
{
    MutexGuard  switchGuard(switchLock);

    tm tmDate;
    time_t date = time(0);
    localtime_r(&date, &tmDate);

    resetTraffic(smppTrafficByRouteId, tmDate);
    resetTraffic(httpTrafficByRouteId, tmDate);

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
    time_t nextTime = currTime + SCAG_STAT_DUMP_INTERVAL;
    tm tmNT; localtime_r(&nextTime, &tmNT);
    tmNT.tm_sec = 0;
    nextTime = mktime(&tmNT);
    return (((nextTime-currTime)*1000)+1);
}

void StatisticsManager::SerializeSmppStat(Hash<CommonStat>& smppStat, SerializationBuffer& buf, bool add)
{
    buf.WriteNetInt32(smppStat.GetCount());

    smppStat.First();
    char* Id = 0;
    CommonStat* st = 0;
    while (smppStat.Next(Id, st))
    {
        if (!st || !Id || Id[0] == '\0') continue;
            
        uint16_t len = (uint16_t)strlen(Id);
        buf.WriteNetInt16(len);
        buf.Write(Id, len);

        buf.WriteNetInt32(st->accepted);
        buf.WriteNetInt32(st->rejected);
        buf.WriteNetInt32(st->gw_rejected);
        buf.WriteNetInt32(st->delivered);
        buf.WriteNetInt32(st->failed);
        if(add)
        {
            buf.WriteNetInt32(st->recieptOk);
            buf.WriteNetInt32(st->recieptFailed);
        }

        buf.WriteNetInt32(st->errors.Count());

        IntHash<int>::Iterator sit = st->errors.First();
        int secError, seCounter;
        while (sit.Next(secError, seCounter))
        {
            buf.WriteNetInt32(secError);
            buf.WriteNetInt32(seCounter);
        }
        st = 0;
    }
}

void StatisticsManager::SerializeHttpStat(Hash<HttpStat>& httpStat, SerializationBuffer& buf)
{
    buf.WriteNetInt32(httpStat.GetCount());

    httpStat.First();
    char* Id = 0;
    HttpStat* st = 0;
    while (httpStat.Next(Id, st))
    {
        if (!st || !Id || Id[0] == '\0') continue;
          
//        smsc_log_debug(logger, "name: %s, req: %d, req_r: %d, res: %d, res_r: %d, d: %d, f: %d\n", Id, st->request, st->requestRejected, st->response, st->responseRejected, st->delivered, st->failed);

        uint16_t len = (uint16_t)strlen(Id);
        buf.WriteNetInt16(len);
        buf.Write(Id, len);

        buf.WriteNetInt32(st->request);
        buf.WriteNetInt32(st->requestRejected);
        buf.WriteNetInt32(st->response);
        buf.WriteNetInt32(st->responseRejected);
        buf.WriteNetInt32(st->delivered);
        buf.WriteNetInt32(st->failed);

        buf.WriteNetInt32(st->errors.Count());

        IntHash<int>::Iterator sit = st->errors.First();
        int secError, seCounter;
        while (sit.Next(secError, seCounter))
        {
            buf.WriteNetInt32(secError);
            buf.WriteNetInt32(seCounter);
        }

        st = 0;
    }
}

void StatisticsManager::flushCounters(int index)
{

    tm flushTM;
    calculateTime(flushTM);
    smsc_log_debug(logger, "Flushing SMPP statistics for %02d.%02d.%04d %02d:%02d:%02d GMT",
                   flushTM.tm_mday, flushTM.tm_mon+1, flushTM.tm_year+1900,
                   flushTM.tm_hour, flushTM.tm_min, flushTM.tm_sec);
    try
    {
        SerializationBuffer buf(4096);

        buf.WriteByte(flushTM.tm_hour);
        buf.WriteByte(flushTM.tm_min);

        SerializeSmppStat(statByRouteId[index], buf, true);
        SerializeSmppStat(srvStatBySmeId[index], buf, false);
        SerializeSmppStat(statBySmeId[index], buf, false);

        dumpCounters((const unsigned char*)buf.getBuffer(), buf.getPos(), flushTM, smppFileTM, SCAG_SMPP_STAT_DIR_NAME_FORMAT, smppFile);
    }
    catch (Exception& exc)
    {
        smsc_log_error(logger, "Statistics flush failed. Cause: %s", exc.what());
    }
    catch ( std::exception& e ) {
        smsc_log_error(logger,"Statistics flush failed. Cause: %s", e.what());
        std::terminate();
    }

    resetCounters(index);
}

void StatisticsManager::flushHttpCounters(int index)
{

    tm flushTM;
    calculateTime(flushTM);
    smsc_log_debug(logger, "Flushing HTTP Statistics for %02d.%02d.%04d %02d:%02d:%02d GMT",
                   flushTM.tm_mday, flushTM.tm_mon+1, flushTM.tm_year+1900,
                   flushTM.tm_hour, flushTM.tm_min, flushTM.tm_sec);
    try
    {
        SerializationBuffer buf(4096);

        buf.WriteByte(flushTM.tm_hour);
        buf.WriteByte(flushTM.tm_min);

        SerializeHttpStat(httpStatByRouteId[index], buf);
        SerializeHttpStat(httpStatByUrl[index], buf);

        dumpCounters((const unsigned char*)buf.getBuffer(), buf.getPos(), flushTM, httpFileTM, SCAG_HTTP_STAT_DIR_NAME_FORMAT, httpFile);
    }
    catch (Exception& exc)
    {
        smsc_log_error(logger, "Http Statistics flush failed. Cause: %s", exc.what());
    } catch ( std::exception& e ) {
        smsc_log_error(logger, "Http Statistics flush failed. Cause: %s", e.what());
        std::terminate();
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
  httpStatByUrl[index].Empty();
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
    // int pos = p2 - buff;
    while(p2){
       int plen = p2 - p1;
       dirlen += plen + 1;
       if(plen == 0)
           return false;

       int direclen = dirlen + 1;
       TmpBuf<char, 512> ptmpBuff(direclen);
       char * dir = ptmpBuff.get();
       memcpy(dir, buff, dirlen);
       dir[dirlen] = 0;
       dirs.push_back(std::string(dir));

       p1 = p1 + plen + 1;
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

void StatisticsManager::dumpCounters(const uint8_t* buff, int buffLen, const tm& flushTM, tm& fileTM, const char *dirNameFmt, File& file)
{
    smsc_log_debug(logger, "Statistics dump called for %02d:%02d GMT", 
                   flushTM.tm_hour, flushTM.tm_min);
    try {

    char dirName[128]; bool hasDir = false;

    if (fileTM.tm_mon != flushTM.tm_mon || fileTM.tm_year != flushTM.tm_year)
    {
        snprintf(dirName, sizeof(dirName), dirNameFmt, flushTM.tm_year+1900, flushTM.tm_mon+1);
        createDir(location + "/" + dirName); hasDir = true;
        smsc_log_info(logger, "New dir '%s' created", dirName);
    }

    if (fileTM.tm_mday != flushTM.tm_mday)
    {
        char fileName[128]; 
        std::string fullPath = location;
        if (!hasDir)
            snprintf(dirName, sizeof(dirName), dirNameFmt, flushTM.tm_year+1900, flushTM.tm_mon+1);
        snprintf(fileName, sizeof(fileName), SCAG_STAT_FILE_NAME_FORMAT, flushTM.tm_mday);
        fullPath += '/'; fullPath += (const char*)dirName; 
        fullPath += '/'; fullPath += (const char*)fileName; 
        const char* fullPathStr = fullPath.c_str();

        if (file.isOpened()) file.Close();
            
        if (File::Exists(fullPathStr)) { 
            file.WOpen(fullPathStr);
            file.SeekEnd(0);
            smsc_log_debug(logger, "Existed file '%s' opened", fileName);
        } else {
            file.RWCreate(fullPathStr);
            file.Write(SCAG_STAT_HEADER_TEXT, strlen(SCAG_STAT_HEADER_TEXT));
            uint16_t version = htons(SCAG_STAT_VERSION_INFO);
            file.Write(&version, sizeof(version));
            file.Flush();
            smsc_log_info(logger, "New file '%s' created", fileName);
        }

        fileTM = flushTM;
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
        fileTM.tm_year = 0;
        throw;
    }
}

void StatisticsManager::dumpTrafficHash(Hash<TrafficRecord>& traff, SerializationBuffer& buf)
{
    long mincnt_, hourcnt_, daycnt_, monthcnt_; 
        uint8_t year_, month_, day_, hour_, min_;

    MutexGuard mg(switchLock);

    buf.WriteNetInt16(traff.GetCount());

    traff.First();
    char* routeId = 0;
    TrafficRecord routeTraff;
    while(traff.Next(routeId, routeTraff))
    {
        uint16_t len = strlen(routeId);
        buf.WriteNetInt16(len);
        buf.Write(routeId, len);

        routeTraff.getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);

        buf.WriteNetInt32(mincnt_);
        buf.WriteNetInt32(hourcnt_);
        buf.WriteNetInt32(daycnt_);
        buf.WriteNetInt32(monthcnt_);

        buf.WriteByte(year_);
        buf.WriteByte(month_);
        buf.WriteByte(day_);
        buf.WriteByte(hour_);
        buf.WriteByte(min_);
    }
}

void StatisticsManager::dumpTraffic(Hash<TrafficRecord>& traff, const std::string& path)
{
    SerializationBuffer buf(30000);

    dumpTrafficHash(traff, buf);

    try {
        std::string loc = path + "traffic.tmp";     
        File tfile;
        tfile.WOpen(loc.c_str());
        tfile.Write(buf.getBuffer(), buf.getPos());
        tfile.Close();
        std::string traffpath = path + "traffic.dat";
        File::Rename(loc.c_str(), traffpath.c_str());
    }catch(FileException & e){
        smsc_log_warn(logger, "Failed to dump traffic. Detailes: %s", e.what());
    }catch(...){
        smsc_log_warn(logger, "Failed to dump traffic. Unknown error.");
    }
}

void StatisticsManager::initTraffic(Hash<TrafficRecord>& h, const std::string loc)
{
    File tfile;

    try{
        tfile.ROpen(loc.c_str());

        uint8_t year, month, day, hour, min;
        long mincnt, hourcnt, daycnt, monthcnt;

        time_t now = time(0);
        tm tmnow;
        localtime_r(&now, &tmnow);

        uint16_t cnt = tfile.ReadNetInt16();
        while(cnt--)
        {
            uint16_t len = tfile.ReadNetInt16();
            TmpBuf<char, 512> id(len + 1);
//            char id[len + 1];
            tfile.Read(id.get(), len);
            *(id.get() + len) = 0;
//            id[len] = 0;

            mincnt = tfile.ReadNetInt32();
            hourcnt = tfile.ReadNetInt32();
            daycnt = tfile.ReadNetInt32();
            monthcnt = tfile.ReadNetInt32();

            year = tfile.ReadByte();
            month = tfile.ReadByte();
            day = tfile.ReadByte();
            hour = tfile.ReadByte();
            min = tfile.ReadByte();

            if(tmnow.tm_year != year || tmnow.tm_mon != month)
                continue;

            TrafficRecord tr(mincnt, hourcnt, daycnt, monthcnt, year, month, day, hour, min);
            tr.reset(tmnow);
            h.Insert(id.get(), tr);
        }

        tfile.Close();

    }catch(FileException & e)
    {
        smsc_log_info(logger, "No stored traffic data. Detailes: %s", e.what());
    }catch(...)
    {
        smsc_log_warn(logger, "Failed to init traffic. Unknown error");
    }
}

void StatisticsManager::resetTraffic(Hash<TrafficRecord>& h, const tm& tmDate)
{
    h.First();
    char* routeId = 0;
    TrafficRecord *routeTraff;
    while (h.Next(routeId, routeTraff))
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

void StatisticsManager::incRouteTraffic(Hash<TrafficRecord>& h,  const char* routeId)
{
    time_t now = time(0);
    tm tmnow;   localtime_r(&now, &tmnow);
    TrafficRecord *tr = 0;
    tr = h.GetPtr(routeId);
    if(tr)
        tr->inc(tmnow);
    else
    {
        TrafficRecord ntr(1, 1, 1, 1, tmnow.tm_year, tmnow.tm_mon, tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min);
        h.Insert(routeId, ntr);
    }
}

void StatisticsManager::incSvcScCounter(const char* systemId, int index, int max_cnt, Hash<CommonPerformanceCounter*>& svcCounters, Mutex& mt)
{
    if (!systemId || !systemId[0] || index < 0 || index >= max_cnt) return;

    MutexGuard guard(mt);
    CommonPerformanceCounter*  counter = 0;
    {
        CommonPerformanceCounter** pCounter = svcCounters.GetPtr(systemId);
        if (!pCounter) {
            counter = new CommonPerformanceCounter(max_cnt);
            svcCounters.Insert( systemId, counter );
        } else if ( *pCounter ) {
            counter = *pCounter;
        } else {
            counter = new CommonPerformanceCounter(max_cnt);
            *pCounter = counter;
        }
    }

    counter->counters[index]++;
    if (!counter->slots[index]) counter->slots[index] = newSlotCounter();
    counter->slots[index]->Inc();
}

void StatisticsManager::incSmppCounter(const char* systemId, bool sc, int index)
{
    if(sc)
        incSvcScCounter(systemId, index, PERF_CNT_COUNT, scSmppCounters, scCountersLock);
    else
    {
        index = indexByCounter(index);
        incSvcScCounter(systemId, index, PERF_CNT_COUNT, svcSmppCounters, svcCountersLock);
    }
}

/*void StatisticsManager::incSvcSmppCounter(const char* systemId, int index)
{
    index = indexByCounter(index);
    incSvcScCounter(systemId, index, PERF_CNT_COUNT, svcSmppCounters, svcCountersLock);
}*/

void StatisticsManager::incSvcWapCounter(const char* systemId, int index)
{
    index = indexByHttpCounter(index);
    incSvcScCounter(systemId, index, PERF_HTTP_COUNT, svcWapCounters, svcCountersLock);
}

void StatisticsManager::incSvcMmsCounter(const char* systemId, int index)
{
    incSvcScCounter(systemId, index, PERF_CNT_COUNT, svcMmsCounters, svcCountersLock);
}

/*void StatisticsManager::incScSmppCounter(const char* systemId, int index)
{
    incSvcScCounter(systemId, index, PERF_CNT_COUNT, scSmppCounters, scCountersLock);
}*/

void StatisticsManager::incScWapCounter(const char* systemId, int index)
{
    index = indexByHttpCounter(index);
    incSvcScCounter(systemId, index, PERF_HTTP_COUNT, scWapCounters, scCountersLock);
}

void StatisticsManager::incScMmsCounter(const char* systemId, int index)
{
    incSvcScCounter(systemId, index, PERF_CNT_COUNT, scMmsCounters, scCountersLock);
}

void StatisticsManager::reportGenPerformance(PerformanceData * data)
{
    SerializationBuffer buf(256);

    uint32_t size = 48 + (PERF_CNT_COUNT + PERF_HTTP_COUNT) * 16;
    buf.WriteNetInt32(size);
    buf.WriteNetInt32(data->sessionCount);
    buf.WriteNetInt32(data->sessionLoadedCount);
    buf.WriteNetInt32(data->sessionLockedCount);    
    buf.WriteNetInt32(data->smppReqQueueLen);
    buf.WriteNetInt32(data->smppRespQueueLen);
    buf.WriteNetInt32(data->smppLCMQueueLen);
    buf.WriteNetInt32(data->httpReqQueueLen);
    buf.WriteNetInt32(data->httpRespQueueLen);
    buf.WriteNetInt32(data->httpLCMQueueLen);
    buf.WriteNetInt32(data->uptime);
    buf.WriteNetInt32(data->now);

    for(int i=0;i<PERF_CNT_COUNT;i++)
    {
        buf.WriteNetInt32(data->smppCounters[i].lastSecond);
        buf.WriteNetInt32(data->smppCounters[i].average);
        buf.WriteNetInt64(data->smppCounters[i].total);
    }

    for(int i=0;i<PERF_HTTP_COUNT;i++)
    {
        buf.WriteNetInt32(data->httpCounters[i].lastSecond);
        buf.WriteNetInt32(data->httpCounters[i].average);
        buf.WriteNetInt64(data->httpCounters[i].total);
    }

    //ld.inProcessingCount=htonl(ld.inProcessingCount);
    //ld.inScheduler=htonl(ld.inScheduler);

    MutexGuard g(genSocketsMutex);
    for( size_t i = 0; i < genSockets.size(); i++ )
    {

      int wr = genSockets[i]->WriteAll((char*)buf.getBuffer(), buf.getPos());

      if(uint32_t(wr) != size)
      {
        smsc_log_warn(logger, "Error writing gen performance to socket");
        genSockets[i]->Abort();
        delete genSockets[i];
        genSockets.erase( genSockets.begin() + i );
        i--;
      }
    }
}

void StatisticsManager::getSmppPerfData(uint64_t *cnt)
{
    genStatSmpp.getCounters(cnt);
}

void StatisticsManager::getHttpPerfData(uint64_t *cnt)
{
    genStatHttp.getCounters(cnt);
}

void StatisticsManager::dumpPerfCounters(SerializationBuffer& buf, Hash<CommonPerformanceCounter*>& h)
{
    buf.WriteNetInt16(h.GetCount());

    h.First();
    char* sysId = 0; CommonPerformanceCounter * counter = 0;

    while (h.Next(sysId, counter))
    {
        if (sysId)
        {
            uint16_t l = strlen(sysId);
            buf.WriteNetInt16(l);
            buf.Write(sysId, l);
        }

        for (uint32_t i = 0; i < counter->count; i++)
        {
                 //uint16_t(2)  xxx counter + avg (hour)
            buf.WriteNetInt16((counter) ? counter->counters[i]:0);
            TimeSlotCounter<int>* cnt = (counter && counter->slots[i]) ? counter->slots[i] : 0;
            buf.WriteNetInt16((cnt) ? (uint16_t)cnt->Avg():0);
        }

        if (counter) counter->clear();
    }
}

void StatisticsManager::dumpSvcCounters(SerializationBuffer& buf)
{
    MutexGuard guard(svcCountersLock);

    dumpPerfCounters(buf, svcSmppCounters);
    dumpPerfCounters(buf, svcWapCounters);
    dumpPerfCounters(buf, svcMmsCounters);
}

void StatisticsManager::dumpScCounters(SerializationBuffer& buf)
{
    MutexGuard guard(scCountersLock);

    dumpPerfCounters(buf, scSmppCounters);
    dumpPerfCounters(buf, scWapCounters);
    dumpPerfCounters(buf, scMmsCounters);
}

void StatisticsManager::reportPerformance(bool t, Mutex& mt, std::vector<Socket*>& socks)
{
    SerializationBuffer buf(4096);

    if(t)
        dumpSvcCounters(buf);
    else
        dumpScCounters(buf);

    uint32_t nsize, size = buf.getPos();

    if (size < sizeof(uint32_t)) {
        smsc_log_warn(logger, "invalid service performance data (size=%d)", size);
        return;
    }

    MutexGuard g(mt);
    
    for( size_t i = 0; i < socks.size(); i++ )
    {
        size += 4;
        nsize = htonl(size);
        int wr = socks[i]->WriteAll((char*)&nsize, 4);
        wr += socks[i]->WriteAll((char*)buf.getBuffer(), size - 4);

      if(uint32_t(wr) != size)
      {
        socks[i]->Abort();
        delete socks[i];
        socks.erase(socks.begin() + i);
        i--;
      }
    }
}

void StatisticsManager::reportSvcPerformance()
{
    reportPerformance(1, svcSocketsMutex, svcSockets);
}

void StatisticsManager::reportScPerformance()
{
    reportPerformance(0, scSocketsMutex, scSockets);
}

void StatisticsManager::addSvcSocket(Socket * socket)
{
    MutexGuard g(svcSocketsMutex);
    socket->setNonBlocking(1);
    char buf[32];
    socket->GetPeer(buf);
    smsc_log_info(logger, "performance::add connect from %s", buf);
    svcSockets.push_back(socket);
}

void StatisticsManager::addScSocket(Socket * socket)
{
    MutexGuard g(scSocketsMutex);
    socket->setNonBlocking(1);
    char buf[32];
    socket->GetPeer(buf);
    smsc_log_info(logger, "performance::add connect from %s", buf);
    scSockets.push_back(socket);
}

void StatisticsManager::addGenSocket(Socket * socket)
{
    MutexGuard g(genSocketsMutex);
    socket->setNonBlocking(1);
    char buf[32];
    socket->GetPeer(buf);
    smsc_log_info(logger, "performance::add connect from %s", buf);
    genSockets.push_back(socket);
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

int StatisticsManager::indexByHttpCounter(int event)
{
  using namespace Counters;

  switch(event)
  {
      case events::http::REQUEST_OK:
          return PERF_HTTP_REQUEST;
      case events::http::REQUEST_FAILED:
          return PERF_HTTP_REQUEST_REJECTED;
      case events::http::RESPONSE_OK:
          return PERF_HTTP_RESPONSE;
      case events::http::RESPONSE_FAILED:
          return PERF_HTTP_RESPONSE_REJECTED;
      case events::http::DELIVERED:
          return PERF_HTTP_DELIVERED;
      case events::http::FAILED:
          return PERF_HTTP_FAILED;
  }
  return -1;
}

void StatisticsManager::registerSaccEvent(stat::SaccTrafficInfoEvent* ev)
{
    if (statLogger.get() && statRoller.get() && !statRoller->isStopped()) {
      statLogger->logEvent(ev);
    }
    if (ev) {
      delete ev;
    }
}
void StatisticsManager::registerSaccEvent(stat::SaccBillingInfoEvent* ev)
{
    if (statLogger.get() && statRoller.get() && !statRoller->isStopped()) {
      statLogger->logEvent(ev);
    }
    if (ev) {
      delete ev;
    }
}
void StatisticsManager::registerSaccEvent(stat::SaccAlarmMessageEvent* ev)
{
    thrSaccSender.PushEvent(ev);
}
void StatisticsManager::registerSaccEvent(stat::SaccAlarmEvent* ev)
{
    thrSaccSender.PushEvent(ev);
}

}}
