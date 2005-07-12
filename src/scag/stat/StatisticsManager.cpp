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

#include "StatisticsManager.h"
#include <memory>

#include <sys/types.h>
#include <dirent.h>
#include <vector>
#include <string>


namespace scag {
namespace stat {

const uint16_t SMSC_STAT_DUMP_INTERVAL = 60; // in seconds
const uint16_t SMSC_STAT_VERSION_INFO  = 0x0001;
const char*    SMSC_STAT_HEADER_TEXT   = "SCAG.STAT";
const char*    SMSC_STAT_DIR_NAME_FORMAT  = "%04d-%02d";
const char*    SMSC_STAT_FILE_NAME_FORMAT = "%02d.rts";
StatisticsManager::routeMap;

StatisticsManager::StatisticsManager(Config config)
    :  logger(Logger::getInstance("scag.stat.StatisticsManager")),
       currentIndex(0), bExternalFlush(false), isStarted(false)
{
    try
    {
        location = config.getString("MessageStorage.statisticsDir");
    }catch(...)
    {
        throw Exception("Can't find parameter in config: 'MessageStorage.statisticsDir'");
    }

    try
    {
        location = config.getString("MessageStorage.trafficDir");
    }catch(...)
    {
        throw Exception("Can't find parameter in config: 'MessageStorage.trafficDir'");
    }

    if (!createStorageDir(location)) 
        throw Exception("Can't open statistics directory: '%s'", location.c_str());

    if (!createStorageDir(traffloc)) 
        throw Exception("Can't open traffic directory: '%s'", traffloc.c_str());

    // Initializes traffic hash
    initTraffic();
}

StatisticsManager::~StatisticsManager()
{
  MutexGuard guard(stopLock);
  close();
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

  if (si.smeId && si.smeId[0])
  {
    CommonStat *smeSt = statBySmeId[currentIndex].GetPtr(si.smeId);
    if(!smeSt)
    {
      CommonStat newStat;
      statBySmeId[currentIndex].Insert(si.smeId, newStat);
      smeSt=statBySmeId[currentIndex].GetPtr(si.smeId);
    }

    if(si.internal){
        CommonStat newStat;
        srvStatBySmeId[currentIndex].Insert(si.smeId, newStat);
        srvStat=srvStatBySmeId[currentIndex].GetPtr(si.smeId);
    }
  }

  if (si.routeId && si.routeId[0])
  {
    routeSt = statByRoute[currentIndex].GetPtr(si.routeId);
    if(!routeSt)
    {
      CommonStat newStat;
      statByRoute[currentIndex].Insert(si.routeId, newStat);
      routeSt=statByRoute[currentIndex].GetPtr(si.routeId);
    }
  }

  using namespace Counters;

  if(errcode < cntBillingOk)
  {
    if(smeSt) incError(smeSt->errors, errcode);
    if(srvSt) incError(srvSt->errors, errcode);
    if(routeSt) incError(routeSt->errors, errcode);
  }

  if(smeSt && si.smeProviderId!=-1)smeSt->providerId=si.smeProviderId;
  if(srvSt && si.smeProviderId!=-1)srvSt->providerId=si.smeProviderId;
  if(routeSt && si.routeProviderId!=-1)routeSt->providerId=si.routeProviderId;

  switch(counter)
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

#undef INC_STAT

  }

  if (si.routeId && si.routeId[0])
  {
      int id, newRouteId;
      if(  (id = routeMap.regRoute(si.routeId, newRouteId)) == -1)
          id = newRouteId;

      time_t now = time(0);
      tm tmnow;   localtime_r(&now, &tmnow);
      TrafficRecord *tr = 0;
      tr = trafficByRouteId.GetPtr(id);
      if(tr){
          tr.inc(tmnow;)
      }else{
          TrafficRecord tr(1, 1, 1, 1, 
                         tmnow.tm_year, tmnow.tm_month, tmnow.tm_mday, tmnow.tm_hour, tmnow.tm_min);
          trafficByRouteId.Insert(id, tr);
      }
  }
}

bool StatisticsManager::checkTraffic(string routeId, Statistics::CheckTrafficPeriod period, int64_t value)
{
        MutexGuard mg(switchLook);

        int id;
        if(  (id = routeMap.lookup(routeId.c_str())) == -1)
            return false;

        switch(period){
        case Statistics::checkMinPeriod:
            TrafficRecord *tr = 0;
            tr = trafficByRouteId.GetPtr(id);
            if(tr){
                time_t now = time(0);
                tm tmnow;   localtime_r(&now, &tmnow);
                tr.reset(tmnow;)

                long mincnt_, hourcnt_, daycnt_, monthcnt_;
                uint8_t year_, month_, day_, hour_, min_;
                tr.getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);
                if(mincnt_ <= value)
                    return true;
            }
            break;
        case Statistics::checkHourPeriod:
            TrafficRecord *tr = 0;
            tr = trafficByRouteId.GetPtr(id);
            if(tr){
                time_t now = time(0);
                tm tmnow;   localtime_r(&now, &tmnow);
                tr.reset(tmnow;)

                long mincnt_, hourcnt_, daycnt_, monthcnt_;
                uint8_t year_, month_, day_, hour_, min_;
                tr.getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);
                if(hourcnt_ <= value)
                    return true;
            }
            break;
        case Statistics::checkDayPeriod:
            TrafficRecord *tr = 0;
            tr = trafficByRouteId.GetPtr(id);
            if(tr){
                time_t now = time(0);
                tm tmnow;   localtime_r(&now, &tmnow);
                tr.reset(tmnow;)

                long mincnt_, hourcnt_, daycnt_, monthcnt_;
                uint8_t year_, month_, day_, hour_, min_;
                tr.getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);
                if(daycnt_ <= value)
                    return true;
            }
            break;
        case Statistics::checkMonthPeriod:
            TrafficRecord *tr = 0;
            tr = trafficByRouteId.GetPtr(id);
            if(tr){
                time_t now = time(0);
                tm tmnow;   localtime_r(&now, &tmnow);
                tr.reset(tmnow;)

                long mincnt_, hourcnt_, daycnt_, monthcnt_;
                uint8_t year_, month_, day_, hour_, min_;
                tr.getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);
                if(monthcnt_ <= value)
                    return true;
            }
            break;
        }
        return false;
}

int StatisticsManager::Execute()
{
    smsc_log_debug(logger, "Execute() started (%d)", isStopping);
    isStarted = true; bExternalFlush = false;
    while (!isStopping)
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
    isStarted = false;
    exitEvent.Signal();
    smsc_log_debug(logger, "Execute() exited");
    return 0;
}

void StatisticsManager::stop()
{
    MutexGuard guard(stopLock);

    smsc_log_debug(logger, "stop() called, started=%d", isStarted);
    ThreadedTask::stop();
    if (isStarted)
    {
        bExternalFlush = true;
        awakeEvent.Signal();
        smsc_log_debug(logger, "stop() waiting finish ...");
        exitEvent.Wait();
    }
    smsc_log_debug(logger, "stop() exited");
}

int StatisticsManager::switchCounters()
{
    MutexGuard  switchGuard(switchLock);

    tm tmDate;
    time_t date = time(0);
    localtime_r(&date, &tmDate);
    incTraffic(tmDate);

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
        value32 = statByRoute[index].GetCount(); 
        value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

        statByRoute[index].First();
        char* routeId = 0;
        CommonStat* routeStat = 0;
        while (statByRoute[index].Next(routeId, routeStat))
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
            value32 = htonl(smeStat->billingOk);            buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->billingFailed);        buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->recieptOk);            buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->recieptFailed);        buff.Append((uint8_t *)&value32, sizeof(value32));

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
            value32 = htonl(srvStat->billingOk);            buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->billingFailed);        buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->recieptOk);            buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(srvStat->recieptFailed);        buff.Append((uint8_t *)&value32, sizeof(value32));

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
  totalStatBySmeId[index].Empty();
  commonStatByRoute[index].Empty();
}

void StatisticsManager::close()
{
    if (file) { 
        bFileTM = false; fclose(file); file = 0;
    }
}
void StatisticsManager::flush()
{
    if (file && fflush(file)) {
        int error = ferror(file);
        Exception exc("Failed to flush file. Details: %s", strerror(error));
        close(); throw exc;
    }
}

void StatisticsManager::write(const void* data, size_t size)
{
    if (file && fwrite(data, size, 1, file) != 1) {
        int error = ferror(file);
        Exception exc("Failed to write file. Details: %s", strerror(error));
        close(); throw exc;
    }
}

void StatisticsManager::Fseek(long offset, int whence, FILE &*cfPtr)
{
    if (cfPtr && fseek(cfPtr, offset, whence)) {
        int error = ferror(cfPtr);
        Exception exc("Failed to seek file. Details: %s", strerror(error));
        Flose(cfPtr); throw exc;
    }
}

void StatisticsManager::Fclose(FILE &*cfPtr)
{
    if (cfPtr) { 
        fclose(cfPtr); cfPtr = 0;
    }
}

void StatisticsManager::Fopen(FILE &*cfPtr, const std::string loc)
{
    cfPtr = fopen(loc.c_str(), "rb+"); // open or create for update
    if (!cfPtr)
        throw Exception("Failed to create/open file '%s'. Details: %s", 
                            loc.c_str(), strerror(errno));
}

void StatisticsManager::Fflush(FILE &*cfPtr)
{
    if (cfPtr && fflush(cfPtr)) {
        int error = ferror(cfPtr);
        Exception exc("Failed to flush file. Details: %s", strerror(error));
        Fclose(cfPtr); throw exc;
    }
}

void StatisticsManager::Fread(FILE &*cfPtr, void* data, size_t size)
{
    int read_size = fread(data, size, 1, cfPtr);

    if( (read_size != 1) && (read_size != 0)){
        int err = ferror(cfPtr);
        Fclose(cfPtr);
        throw Exception("Can't read route from file. Details: %s", strerror(err));
    }
}

void StatisticsManager::Fwrite(const void* data, size_t size, FILE *cfPtr)
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

    char dirName[128]; bool hasDir = false;

    if (!bFileTM || fileTM.tm_mon != flushTM.tm_mon || fileTM.tm_year != flushTM.tm_year)
    {
        sprintf(dirName, SMSC_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
        createDir(location + "/" + dirName); bFileTM = false; hasDir = true;
        smsc_log_debug(logger, "New dir '%s' created", dirName);
    }

    if (!bFileTM || fileTM.tm_mday != flushTM.tm_mday)
    {
        close(); // close old file (if it was opened)
        
        char fileName[128]; 
        std::string fullPath = location;
        if (!hasDir) sprintf(dirName, SMSC_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
        sprintf(fileName, SMSC_STAT_FILE_NAME_FORMAT, flushTM.tm_mday);
        fullPath += '/'; fullPath += (const char*)dirName; 
        fullPath += '/'; fullPath += (const char*)fileName; 
        const char* fullPathStr = fullPath.c_str();

        bool needHeader = true;
        file = fopen(fullPathStr, "r");
        if (file) { // file already exists (was opened for reading)
            close(); needHeader = false;
        }
        
        file = fopen(fullPathStr, "ab+"); // open or create for append
        if (!file)
            throw Exception("Failed to create/open file '%s'. Details: %s", 
                            fullPathStr, strerror(errno));
        
        if (fseek(file, 0, SEEK_END)) { // set position to EOF
            int error = ferror(file);
            Exception exc("Failed to seek EOF. Details: %s", strerror(error));
            close(); throw exc;
        }

        if (needHeader) { // create header (if new file created)
            write(SMSC_STAT_HEADER_TEXT, strlen(SMSC_STAT_HEADER_TEXT));
            uint16_t version = htons(SMSC_STAT_VERSION_INFO);
            write(&version, sizeof(version));
            flush();
        }
        fileTM = flushTM; bFileTM = true;
        smsc_log_debug(logger, "%s file '%s' %s", (needHeader) ? "New":"Existed",
                       fileName, (needHeader) ? "created":"opened");
    }
    
    smsc_log_debug(logger, "Statistics data dump...");
    uint32_t value32 = htonl(buffLen);
    write((const void *)&value32, sizeof(value32));
    write((const void *)buff, buffLen); // write dump to it
    write((const void *)&value32, sizeof(value32));
    flush();
    smsc_log_debug(logger, "Statistics data dumped.");
}

void StatisticsManager::flushTraffic()
{
    {
        MutexGuard mg(switchLock);
        Hash<TrafficRecord> traff;
        traff = trafficByRoutedId;
    }

    dumpTraffic(traff);
}

void StatisticsManager::dumpTraffic(const IntHash<TrafficRecord>& traff)
{
    int pos = smsc::sms::MAX_ROUTE_ID_TYPE_LENGTH + 1;
    const int sz = pos + 21;
    char buff[sz]; tmpBuff<uint8_t, 27648> fbuff(27648);
    long mincnt_, hourcnt_, daycnt_, monthcnt_; 
    uint8_t year_, month_, day_, hour_, min_;

    IntHash<TrafficRecord>::Iterator traffit = traff.First();
    int routeId = 0;
    TrafficRecord routeTraff;
    while (traffit.Next(routeId, routeTraff))
    {
        // Copies routeId
        char * id = routeMap.lookup(routeId);
        memcpy((void*)buff, (const void*)(id), pos);

        routeTraff.getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);

        // Copies counters
        uint32_t lval = mincnt_;
        val = htonl(lval);
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

    std::string loc = traffloc + std::string('/') + "traffic.tmp";
    FILE *cfPtr;
    Fopen(cfPtr, loc.c_str()); // open or create for update
    Fwrite(fbuff, fbuff.GetPos(), cfPtr);
    Fclose(cfPtr);

    std::string traffpath = traffloc + std::string('/') + "traffic.dat";
    rename(loc, traffpath);
    

}

void initTraffic()
{

    const std::string loc = traffloc + std::string('/') + "traffic.dat";
    FILE *cfPtr;
    Fopen(cfPtr, loc);
    Fseek(0, SEEK_SET, cfPtr);

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
    while( (read_size = Fread(buff, 25, cfPtr)) == 1){

        // Copies routeId
        memcpy((void*)routeId, (const void*)buff, pos);
        if(routeMap.lookup(routeId, id) == -1)
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

        if(tmnow.tm_year != year_)
            continue;
        if(tmnow.tm_month != month_)
            continue;

        tr.inc(tmnow);
        trafficByRouteId.Insert(id, tr);

    }

    Fclose(cfPtr);
}

void incTraffc(const tm& tmDate)
{
    IntHash<TrafficRecord>::Iterator traffit = traff.First();
    int routeId = 0;
    TrafficRecord routeTraff;
    while (traffit.Next(routeId, routeTraff))
    {
        routeTraff.inc(tmDate);
    }
}

void incRouteTraffic(const int routeId, const tm& tmDate)
{
    TrafficRecord *tr = 0;
    tr = trafficByRouteId.GetPtr(routeId);
    if(tr){
        tr->inc(tmDate);
    }
}

}//namespace stat
}//namespace scag
