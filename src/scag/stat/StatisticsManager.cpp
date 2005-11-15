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

const uint16_t SMSC_STAT_DUMP_INTERVAL = 60; // in seconds
const uint16_t SMSC_STAT_VERSION_INFO  = 0x0001;
const char*    SMSC_STAT_HEADER_TEXT   = "SCAG.STAT";
const char*    SMSC_STAT_DIR_NAME_FORMAT  = "%04d-%02d";
const char*    SMSC_STAT_FILE_NAME_FORMAT = "%02d.rts";
RouteMap StatisticsManager::routeMap;

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

    if (!createStorageDir(location)) 
        throw Exception("Can't open statistics directory: '%s'", location.c_str());

    if (!createStorageDir(traffloc)) 
        throw Exception("Can't open traffic directory: '%s'", traffloc.c_str());

    std::string perfHost = statManConfig.getPerfHost();
    if(!perfHost.length())
        throw Exception("StatisticsManager, configure: Dirrectory has zero length.");
    int perfGenPort = statManConfig.getPerfGenPort();
    int perfSvcPort = statManConfig.getPerfSvcPort();
    int perfScPort = statManConfig.getPerfScPort();

    sender.init((PerformanceListener*)this, (PerformanceServer*)this);
    sender.InitServer(perfHost, perfGenPort, perfSvcPort, perfScPort);

    initTraffic(); // Initializes traffic hash
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

void StatisticsManager::registerEvent(const SmppStatEvent& si)
{
  MutexGuard  switchGuard(switchLock);

  CommonStat* smeSt=0;
  CommonStat* routeSt=0;
  CommonStat* srvSt=0;

  using namespace Counters;

  genCounters.inc(si.counter);

  if (si.smeId && si.smeId[0])
  {
    if(si.counter < cntBillingOk){
        smeSt = statBySmeId[currentIndex].GetPtr(si.smeId);

        /*int cnt = si.counter < 0x1000 ? si.counter : si.counter - 0x1000 + 5;
        smsc_log_debug(logger, "\nsmeId: '%s', %s\n", si.smeId, cnt_[cnt]);*/

        if(!smeSt)
        {
            CommonStat newStat;
            statBySmeId[currentIndex].Insert(si.smeId, newStat);
            smeSt=statBySmeId[currentIndex].GetPtr(si.smeId);
        }

        if(si.internal){
            srvSt = srvStatBySmeId[currentIndex].GetPtr(si.smeId);
            if(!srvSt){
                CommonStat newStat;
                srvStatBySmeId[currentIndex].Insert(si.smeId, newStat);
                srvSt=srvStatBySmeId[currentIndex].GetPtr(si.smeId);
            }

            // run-time statistics
            incScSmppCounter(si.smeId, indexByCounter(si.counter));
        }else
            // run-time statistics
            incSvcSmppCounter(si.smeId, indexByCounter(si.counter));
    }
  }

  if (si.routeId && si.routeId[0])
  {
    //smsc_log_debug(logger, "routeId: '%s'", si.routeId);
    routeSt = statByRouteId[currentIndex].GetPtr(si.routeId);
    if(!routeSt)
    {
      CommonStat newStat;
      statByRouteId[currentIndex].Insert(si.routeId, newStat);
      routeSt=statByRouteId[currentIndex].GetPtr(si.routeId);
    }
  }

  if(si.counter < cntBillingOk)
  {
    if(smeSt) incError(smeSt->errors, si.errCode);
    if(srvSt) incError(srvSt->errors, si.errCode);
    if(routeSt) incError(routeSt->errors, si.errCode);
  }

  if(smeSt && si.smeProviderId!=-1)smeSt->providerId=si.smeProviderId;
  if(srvSt && si.smeProviderId!=-1)srvSt->providerId=si.smeProviderId;
  if(routeSt && si.routeProviderId!=-1)routeSt->providerId=si.routeProviderId;

  int c;
  switch(si.counter)
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

  if (si.routeId && si.routeId[0])
  {
      if( si.counter == cntAccepted){
          int id, newRouteId;
          if(  (id = routeMap.regRoute(si.routeId, newRouteId)) == -1)
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

        flushCounters(switchCounters());

        flushTraffic();

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

    int flushIndex = currentIndex;
    currentIndex ^= 1; //switch between 0 and 1
    return flushIndex;
}

void StatisticsManager::calculateTime(tm& flushTM)
{
    time_t flushTime = time(0);
    if (!bExternalFlush) flushTime -= SMSC_STAT_DUMP_INTERVAL;
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
    smsc_log_debug(logger, "Flushing statistics for %02d.%02d.%04d %02d:%02d:%02d GMT",
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

void StatisticsManager::resetCounters(int index)
{
  statBySmeId[index].Empty();
  statByRouteId[index].Empty();
  srvStatBySmeId[index].Empty();
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
    int len = strlen(loc.c_str());
    if(len == 0)
        return false;

    if(strcmp(loc.c_str(), "/") == 0)
        return true;

    ++len;
    TmpBuf<char, 512> tmpBuff(len);
    char* buff = tmpBuff.get();
    memcpy(buff, loc.c_str(), len);
    if(buff[len-2] == '/'){
       buff[len-2] = 0;
       if(len > 2){
          if(buff[len-3] == '/'){
              return false;
           }
       }
    }

    std::vector<char*> dirs(0);

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
       dirs.push_back(dir);

       p1 = p1 + len + 1;
       p2 = strchr(p1, '/');
    }
    dirs.push_back(buff);

    std::vector<char*>::iterator it = dirs.begin();
    for(it = dirs.begin(); it != dirs.end(); it++){

        DIR* dirp = opendir(*it);
        if(dirp){
            closedir(dirp);            
        }else{
            try{
                createDir(std::string(*it));
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
        sprintf(dirName, SMSC_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
        createDir(location + "/" + dirName); bFileTM = false; hasDir = true;
        smsc_log_debug(logger, "New dir '%s' created", dirName);
    }

    if (!bFileTM || fileTM.tm_mday != flushTM.tm_mday)
    {
        char fileName[128]; 
        std::string fullPath = location;
        if (!hasDir) sprintf(dirName, SMSC_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
        sprintf(fileName, SMSC_STAT_FILE_NAME_FORMAT, flushTM.tm_mday);
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
            file.Write(SMSC_STAT_HEADER_TEXT, strlen(SMSC_STAT_HEADER_TEXT));
            uint16_t version = htons(SMSC_STAT_VERSION_INFO);
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

void StatisticsManager::flushTraffic()
{
    IntHash<TrafficRecord> traff;
    {
        MutexGuard mg(switchLock);
        traff = trafficByRouteId;
    }

    dumpTraffic(traff);
}

void StatisticsManager::dumpTraffic(const IntHash<TrafficRecord>& traff)
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
        std::string loc = traffloc + std::string("/") + "traffic.tmp";
        /*FILE *cfPtr = 0;
        Fopen(cfPtr, loc.c_str()); // open or create for update
        Fwrite(fbuff, fbuff.GetPos(), cfPtr);
        Fclose(cfPtr);*/

        try {
     
            File tfile;
            tfile.WOpen(loc.c_str());
            tfile.Write(fbuff, fbuff.GetPos());
            tfile.Close();

            std::string traffpath = traffloc + std::string("/") + "traffic.dat";
            rename(loc.c_str(), traffpath.c_str());
        }catch(FileException & e){
            smsc_log_warn(logger, "Failed to dump traffic. Detailes: %s", e.what());
        }catch(...){
            smsc_log_warn(logger, "Failed to dump traffic. Unknown error.");
        }
    }else{
        std::string traffpath = traffloc + std::string("/") + "traffic.dat";
        remove(traffpath.c_str());
    }
    

}

void StatisticsManager::initTraffic()
{

    const std::string loc = traffloc + std::string("/") + "traffic.dat";
    /*FILE *cfPtr = 0;
    Fopen(cfPtr, loc);
    Fseek(0, SEEK_SET, cfPtr);*/

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
    //while( (read_size = Fread(cfPtr, buff, sz)) == 1){
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

void StatisticsManager::incRouteTraffic(const int routeId, const tm& tmDate)
{
    TrafficRecord *tr = 0;
    tr = trafficByRouteId.GetPtr(routeId);
    if(tr){
        tr->inc(tmDate);
    }
}

void StatisticsManager::incSvcSmppCounter(const char* systemId, int index)
{
    if (!systemId || !systemId[0] || index<0 || index>=PERF_CNT_COUNT) return;

    SmePerformanceCounter*  counter = 0;
    SmePerformanceCounter** smeCounter = svcSmppCounters.GetPtr(systemId);

    if (!smeCounter) counter = new SmePerformanceCounter();
    else if (*smeCounter) counter = *smeCounter;
    else {
        counter = new SmePerformanceCounter();
        smeCounter = 0; svcSmppCounters.Delete(systemId);
    }

    counter->counters[index]++;
    if (!counter->slots[index]) counter->slots[index] = newSlotCounter();
    counter->slots[index]->Inc();

    if (!smeCounter) svcSmppCounters.Insert(systemId, counter);
}

void StatisticsManager::incSvcWapCounter(const char* systemId, int index)
{
    if (!systemId || !systemId[0] || index<0 || index>=PERF_CNT_COUNT) return;

    SmePerformanceCounter*  counter = 0;
    SmePerformanceCounter** smeCounter = svcWapCounters.GetPtr(systemId);

    if (!smeCounter) counter = new SmePerformanceCounter();
    else if (*smeCounter) counter = *smeCounter;
    else {
        counter = new SmePerformanceCounter();
        smeCounter = 0; svcWapCounters.Delete(systemId);
    }

    counter->counters[index]++;
    if (!counter->slots[index]) counter->slots[index] = newSlotCounter();
    counter->slots[index]->Inc();

    if (!smeCounter) svcWapCounters.Insert(systemId, counter);
}

void StatisticsManager::incSvcMmsCounter(const char* systemId, int index)
{
    if (!systemId || !systemId[0] || index<0 || index>=PERF_CNT_COUNT) return;

    SmePerformanceCounter*  counter = 0;
    SmePerformanceCounter** smeCounter = svcMmsCounters.GetPtr(systemId);

    if (!smeCounter) counter = new SmePerformanceCounter();
    else if (*smeCounter) counter = *smeCounter;
    else {
        counter = new SmePerformanceCounter();
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

    SmePerformanceCounter*  counter = 0;
    SmePerformanceCounter** smeCounter = scSmppCounters.GetPtr(systemId);

    if (!smeCounter) counter = new SmePerformanceCounter();
    else if (*smeCounter) counter = *smeCounter;
    else {
        counter = new SmePerformanceCounter();
        smeCounter = 0; scSmppCounters.Delete(systemId);
    }

    counter->counters[index]++;
    if (!counter->slots[index]) counter->slots[index] = newSlotCounter();
    counter->slots[index]->Inc();

    if (!smeCounter) scSmppCounters.Insert(systemId, counter);
}

void StatisticsManager::incScWapCounter(const char* systemId, int index)
{
    if (!systemId || !systemId[0] || index<0 || index>=PERF_CNT_COUNT) return;

    SmePerformanceCounter*  counter = 0;
    SmePerformanceCounter** smeCounter = scWapCounters.GetPtr(systemId);

    if (!smeCounter) counter = new SmePerformanceCounter();
    else if (*smeCounter) counter = *smeCounter;
    else {
        counter = new SmePerformanceCounter();
        smeCounter = 0; scWapCounters.Delete(systemId);
    }

    counter->counters[index]++;
    if (!counter->slots[index]) counter->slots[index] = newSlotCounter();
    counter->slots[index]->Inc();

    if (!smeCounter) scWapCounters.Insert(systemId, counter);
}

void StatisticsManager::incScMmsCounter(const char* systemId, int index)
{
    if (!systemId || !systemId[0] || index<0 || index>=PERF_CNT_COUNT) return;

    SmePerformanceCounter*  counter = 0;
    SmePerformanceCounter** smeCounter = scMmsCounters.GetPtr(systemId);

    if (!smeCounter) counter = new SmePerformanceCounter();
    else if (*smeCounter) counter = *smeCounter;
    else {
        counter = new SmePerformanceCounter();
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

    ld.size=htonl(sizeof(ld));
    ld.countersNumber=htonl(ld.countersNumber);


    for(int i=0;i<PERF_CNT_COUNT;i++)
    {
      ld.counters[i].lastSecond=htonl(ld.counters[i].lastSecond);
      ld.counters[i].average=htonl(ld.counters[i].average);

      uint64_t tmp=ld.counters[i].total;
      unsigned char *ptr=(unsigned char *)&ld.counters[i].total;
      ptr[0]=(unsigned char)(tmp>>56);
      ptr[1]=(unsigned char)(tmp>>48)&0xFF;
      ptr[2]=(unsigned char)(tmp>>40)&0xFF;
      ptr[3]=(unsigned char)(tmp>>32)&0xFF;
      ptr[4]=(unsigned char)(tmp>>24)&0xFF;
      ptr[5]=(unsigned char)(tmp>>16)&0xFF;
      ptr[6]=(unsigned char)(tmp>>8)&0xFF;
      ptr[7]=(unsigned char)(tmp&0xFF);
    }

    ld.uptime=htonl(ld.uptime);
    ld.now=htonl(ld.now);
    //ld.eventQueueSize=htonl(ld.eventQueueSize);
    //ld.inProcessingCount=htonl(ld.inProcessingCount);
    //ld.inScheduler=htonl(ld.inScheduler);

    for(int i=0;i<genSockets.Count();i++)
    {

      int wr=genSockets[i]->WriteAll((char*)&ld,sizeof(ld));

      if(wr!=sizeof(ld))
      {
        genSockets[i]->Abort();
        delete genSockets[i];
        genSockets.Delete(i);
        i--;
      }
    }
}

void StatisticsManager::getPerfData(uint64_t *cnt)
{
    genCounters.getCounters(cnt);
}

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

        uint32_t reclen = sizeof(char)*(smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1)+sizeof(uint16_t)*2*PERF_CNT_COUNT;
        smePerfDataSize = sizeof(uint32_t)+3*sizeof(uint16_t)+ 
             reclen*svcSmppCounters.GetCount()+
             reclen*svcWapCounters.GetCount()+
             reclen*svcMmsCounters.GetCount();

        uint8_t* data = new uint8_t[smePerfDataSize];
        uint8_t* packet = data; memset(packet, 0, smePerfDataSize);

        // uint32_t     Total packet size
        *((uint32_t*)packet) = htonl(smePerfDataSize-sizeof(uint32_t)); packet += sizeof(uint32_t);
        // uint16_t     Services count
        *((uint16_t*)packet) = htons((uint16_t)svcSmppCounters.GetCount()); packet += sizeof(uint16_t);
        
        svcSmppCounters.First();
        char* systemId = 0; SmePerformanceCounter* smeCounter = 0;
        while (svcSmppCounters.Next(systemId, smeCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (systemId) strncpy((char *)packet, systemId, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;
            
            for (int i=0; i<PERF_CNT_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (smeCounter) ? htons(smeCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (smeCounter && smeCounter->slots[i]) ? smeCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (smeCounter) smeCounter->clear();
        }

        *((uint16_t*)packet) = htons((uint16_t)svcWapCounters.GetCount()); packet += sizeof(uint16_t);

        svcWapCounters.First();
        systemId = 0; smeCounter = 0;
        while (svcWapCounters.Next(systemId, smeCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (systemId) strncpy((char *)packet, systemId, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;
            
            for (int i=0; i<PERF_CNT_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (smeCounter) ? htons(smeCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (smeCounter && smeCounter->slots[i]) ? smeCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (smeCounter) smeCounter->clear();
        }

        *((uint16_t*)packet) = htons((uint16_t)svcMmsCounters.GetCount()); packet += sizeof(uint16_t);

        svcMmsCounters.First();
        systemId = 0; smeCounter = 0;
        while (svcMmsCounters.Next(systemId, smeCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (systemId) strncpy((char *)packet, systemId, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;
            
            for (int i=0; i<PERF_CNT_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (smeCounter) ? htons(smeCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (smeCounter && smeCounter->slots[i]) ? smeCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (smeCounter) smeCounter->clear();
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

        uint32_t reclen = sizeof(char)*(smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1)+sizeof(uint16_t)*2*PERF_CNT_COUNT;
        smePerfDataSize = sizeof(uint32_t)+3*sizeof(uint16_t)+ 
             reclen*scSmppCounters.GetCount()+
             reclen*scWapCounters.GetCount()+
             reclen*scMmsCounters.GetCount();

        uint8_t* data = new uint8_t[smePerfDataSize];
        uint8_t* packet = data; memset(packet, 0, smePerfDataSize);

        // uint32_t     Total packet size
        *((uint32_t*)packet) = htonl(smePerfDataSize-sizeof(uint32_t)); packet += sizeof(uint32_t);
        // uint16_t     Services count
        *((uint16_t*)packet) = htons((uint16_t)scSmppCounters.GetCount()); packet += sizeof(uint16_t);
        
        scSmppCounters.First();
        char* systemId = 0; SmePerformanceCounter* smeCounter = 0;
        while (scSmppCounters.Next(systemId, smeCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (systemId) strncpy((char *)packet, systemId, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;
            
            for (int i=0; i<PERF_CNT_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (smeCounter) ? htons(smeCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (smeCounter && smeCounter->slots[i]) ? smeCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (smeCounter) smeCounter->clear();
        }

        *((uint16_t*)packet) = htons((uint16_t)scWapCounters.GetCount()); packet += sizeof(uint16_t);

        scWapCounters.First();
        systemId = 0; smeCounter = 0;
        while (scWapCounters.Next(systemId, smeCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (systemId) strncpy((char *)packet, systemId, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;
            
            for (int i=0; i<PERF_CNT_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (smeCounter) ? htons(smeCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (smeCounter && smeCounter->slots[i]) ? smeCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (smeCounter) smeCounter->clear();
        }

        *((uint16_t*)packet) = htons((uint16_t)scMmsCounters.GetCount()); packet += sizeof(uint16_t);

        scMmsCounters.First();
        systemId = 0; smeCounter = 0;
        while (scMmsCounters.Next(systemId, smeCounter))
        {
            // char[MAX_SMESYSID_TYPE_LENGTH+1], null terminated smeId
            if (systemId) strncpy((char *)packet, systemId, smsc::sms::MAX_SMESYSID_TYPE_LENGTH);
            packet += smsc::sms::MAX_SMESYSID_TYPE_LENGTH+1;
            
            for (int i=0; i<PERF_CNT_COUNT; i++)
            {
                // uint16_t(2)  xxx counter + avg (hour)
                *((uint16_t*)packet) = (smeCounter) ? htons(smeCounter->counters[i]):0; packet += sizeof(uint16_t);
                TimeSlotCounter<int>* cnt = (smeCounter && smeCounter->slots[i]) ? smeCounter->slots[i]:0;
                *((uint16_t*)packet) = (cnt) ? htons((uint16_t)cnt->Avg()):0; packet += sizeof(uint16_t);
            }
            if (smeCounter) smeCounter->clear();
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

}//namespace stat
}//namespace scag
