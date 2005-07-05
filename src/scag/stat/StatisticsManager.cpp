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

    // Opens traffic file and initializes traffic hash
    tfopen();
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

void StatisticsManager::registerCommand(SmppCommand cmd)
{
  MutexGuard  switchGuard(switchLock);

  CommonStat* smeSt=0;
  CommonStat* routeSt=0;
  ServiceStat* srvSt=0;

  /*if (si.smeId && si.smeId[0])
  {
    TotalStat *st = totalStatBySmeId[currentIndex].GetPtr(si.smeId);
    if(!st)
    {
      TotalStat newStat;
      totalStatBySmeId[currentIndex].Insert(si.smeId, newStat);
      st=totalStatBySmeId[currentIndex].GetPtr(si.smeId);
    }
    smeSt=&st->common;
    srvSt=&st->service;
  }

  if (si.routeId && si.routeId[0])
  {
    routeSt = commonStatByRoute[currentIndex].GetPtr(si.routeId);
    if(!routeSt)
    {
      CommonStat newStat;
      commonStatByRoute[currentIndex].Insert(si.routeId, newStat);
      routeSt=commonStatByRoute[currentIndex].GetPtr(si.routeId);
    }
  }*/

  using namespace Counters;

  /*if(counter<cntServiceBase)
  {
    if(smeSt)incError(smeSt->errors,errcode);
    if(routeSt)incError(routeSt->errors,errcode);
  }*/

  //if(smeSt && si.smeProviderId!=-1)smeSt->providerId=si.smeProviderId;
  //if(routeSt && si.routeProviderId!=-1)routeSt->providerId=si.routeProviderId;

  switch(counter)
  {
#define INC_STAT(cnt,field) case cnt:{\
      if(smeSt)smeSt->field++; \
      if(routeSt)routeSt->field++; \
      }break;

    INC_STAT(cntAccepted,accepted)
    INC_STAT(cntRejected,rejected)
    INC_STAT(cntDelivered,delivered)
    INC_STAT(cntTemp,temperror)
    INC_STAT(cntPerm,permerror)

#undef INC_STAT

#define UPDATE_SRV_STAT_CNT(name) \
    case cnt##name: \
      if(srvSt)srvSt->name++; \
      break;

    UPDATE_SRV_STAT_CNT(DeniedByBilling)
    UPDATE_SRV_STAT_CNT(SmsTrOk)
    UPDATE_SRV_STAT_CNT(SmsTrFailed)
    UPDATE_SRV_STAT_CNT(SmsTrBilled)
    UPDATE_SRV_STAT_CNT(UssdTrFromScOk)
    UPDATE_SRV_STAT_CNT(UssdTrFromScFailed)
    UPDATE_SRV_STAT_CNT(UssdTrFromScBilled)
    UPDATE_SRV_STAT_CNT(UssdTrFromSmeOk)
    UPDATE_SRV_STAT_CNT(UssdTrFromSmeFailed)
    UPDATE_SRV_STAT_CNT(UssdTrFromSmeBilled)

#undef UPDATE_SRV_STAT_CNT
  }
}

void StatisticsManager::registerCommand(WapCommand cmd)
{
  MutexGuard  switchGuard(switchLock);

  CommonStat* smeSt=0;
  CommonStat* routeSt=0;
  ServiceStat* srvSt=0;

  /*if (si.smeId && si.smeId[0])
  {
    TotalStat *st = totalStatBySmeId[currentIndex].GetPtr(si.smeId);
    if(!st)
    {
      TotalStat newStat;
      totalStatBySmeId[currentIndex].Insert(si.smeId, newStat);
      st=totalStatBySmeId[currentIndex].GetPtr(si.smeId);
    }
    smeSt=&st->common;
    srvSt=&st->service;
  }

  if (si.routeId && si.routeId[0])
  {
    routeSt = commonStatByRoute[currentIndex].GetPtr(si.routeId);
    if(!routeSt)
    {
      CommonStat newStat;
      commonStatByRoute[currentIndex].Insert(si.routeId, newStat);
      routeSt=commonStatByRoute[currentIndex].GetPtr(si.routeId);
    }
  }*/

  using namespace Counters;

  /*if(counter<cntServiceBase)
  {
    if(smeSt)incError(smeSt->errors,errcode);
    if(routeSt)incError(routeSt->errors,errcode);
  }*/

  //if(smeSt && si.smeProviderId!=-1)smeSt->providerId=si.smeProviderId;
  //if(routeSt && si.routeProviderId!=-1)routeSt->providerId=si.routeProviderId;

  switch(counter)
  {
#define INC_STAT(cnt,field) case cnt:{\
      if(smeSt)smeSt->field++; \
      if(routeSt)routeSt->field++; \
      }break;

    INC_STAT(cntAccepted,accepted)
    INC_STAT(cntRejected,rejected)
    INC_STAT(cntDelivered,delivered)
    INC_STAT(cntTemp,temperror)
    INC_STAT(cntPerm,permerror)

#undef INC_STAT

#define UPDATE_SRV_STAT_CNT(name) \
    case cnt##name: \
      if(srvSt)srvSt->name++; \
      break;

    UPDATE_SRV_STAT_CNT(DeniedByBilling)
    UPDATE_SRV_STAT_CNT(SmsTrOk)
    UPDATE_SRV_STAT_CNT(SmsTrFailed)
    UPDATE_SRV_STAT_CNT(SmsTrBilled)
    UPDATE_SRV_STAT_CNT(UssdTrFromScOk)
    UPDATE_SRV_STAT_CNT(UssdTrFromScFailed)
    UPDATE_SRV_STAT_CNT(UssdTrFromScBilled)
    UPDATE_SRV_STAT_CNT(UssdTrFromSmeOk)
    UPDATE_SRV_STAT_CNT(UssdTrFromSmeFailed)
    UPDATE_SRV_STAT_CNT(UssdTrFromSmeBilled)

#undef UPDATE_SRV_STAT_CNT
  }
}

void StatisticsManager::registerCommand(MmsCommand cmd)
{
  MutexGuard  switchGuard(switchLock);

  CommonStat* smeSt=0;
  CommonStat* routeSt=0;
  ServiceStat* srvSt=0;

  /*if (si.smeId && si.smeId[0])
  {
    TotalStat *st = totalStatBySmeId[currentIndex].GetPtr(si.smeId);
    if(!st)
    {
      TotalStat newStat;
      totalStatBySmeId[currentIndex].Insert(si.smeId, newStat);
      st=totalStatBySmeId[currentIndex].GetPtr(si.smeId);
    }
    smeSt=&st->common;
    srvSt=&st->service;
  }

  if (si.routeId && si.routeId[0])
  {
    routeSt = commonStatByRoute[currentIndex].GetPtr(si.routeId);
    if(!routeSt)
    {
      CommonStat newStat;
      commonStatByRoute[currentIndex].Insert(si.routeId, newStat);
      routeSt=commonStatByRoute[currentIndex].GetPtr(si.routeId);
    }
  }*/

  using namespace Counters;

  /*if(counter<cntServiceBase)
  {
    if(smeSt)incError(smeSt->errors,errcode);
    if(routeSt)incError(routeSt->errors,errcode);
  }*/

  //if(smeSt && si.smeProviderId!=-1)smeSt->providerId=si.smeProviderId;
  //if(routeSt && si.routeProviderId!=-1)routeSt->providerId=si.routeProviderId;

  switch(counter)
  {
#define INC_STAT(cnt,field) case cnt:{\
      if(smeSt)smeSt->field++; \
      if(routeSt)routeSt->field++; \
      }break;

    INC_STAT(cntAccepted,accepted)
    INC_STAT(cntRejected,rejected)
    INC_STAT(cntDelivered,delivered)
    INC_STAT(cntTemp,temperror)
    INC_STAT(cntPerm,permerror)

#undef INC_STAT

#define UPDATE_SRV_STAT_CNT(name) \
    case cnt##name: \
      if(srvSt)srvSt->name++; \
      break;

    UPDATE_SRV_STAT_CNT(DeniedByBilling)
    UPDATE_SRV_STAT_CNT(SmsTrOk)
    UPDATE_SRV_STAT_CNT(SmsTrFailed)
    UPDATE_SRV_STAT_CNT(SmsTrBilled)
    UPDATE_SRV_STAT_CNT(UssdTrFromScOk)
    UPDATE_SRV_STAT_CNT(UssdTrFromScFailed)
    UPDATE_SRV_STAT_CNT(UssdTrFromScBilled)
    UPDATE_SRV_STAT_CNT(UssdTrFromSmeOk)
    UPDATE_SRV_STAT_CNT(UssdTrFromSmeFailed)
    UPDATE_SRV_STAT_CNT(UssdTrFromSmeBilled)

#undef UPDATE_SRV_STAT_CNT
  }
}

bool StatisticsManager::checkTraffic(string routeId, int period)
{
        return true;
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
        value32 = commonStatByRoute[index].GetCount(); 
        value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

        commonStatByRoute[index].First();
        char* routeId = 0;
        CommonStat* routeStat = 0;
        while (commonStatByRoute[index].Next(routeId, routeStat))
        {
            if (!routeStat || !routeId || routeId[0] == '\0') continue;
            __trace2__("routeid=%s",routeId);
            // Writes length of routeId and routId
            uint8_t routIdLen = (uint8_t)strlen(routeId);
            buff.Append((uint8_t *)&routIdLen, sizeof(routIdLen));
            buff.Append((uint8_t *)routeId, routIdLen);
            // Writes rout statistics for this routId
            value32 = htonl(routeStat->providerId);     buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->accepted);       buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->rejected);       buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->delivered);      buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->temperror);      buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->permerror);      buff.Append((uint8_t *)&value32, sizeof(value32));

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
        value32 = totalStatBySmeId[index].GetCount(); 
        value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

        totalStatBySmeId[index].First();
        char* smeId = 0;
        TotalStat* smeStat = 0;
        while (totalStatBySmeId[index].Next(smeId, smeStat))
        {
            if (!smeStat || !smeId || smeId[0] == '\0') continue;
            int cnt=2;
            // Writes length of smeId and smeId
            uint8_t smeIdLen = (uint8_t)strlen(smeId);
            buff.Append((uint8_t *)&smeIdLen, sizeof(smeIdLen));
            buff.Append((uint8_t *)smeId, smeIdLen);
            // Writes sme statistics for this smeId
            value32 = htonl(smeStat->common.providerId);            buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->common.accepted);              buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->common.rejected);              buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->common.delivered);             buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->common.temperror);             buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->common.permerror);             buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->service.SmsTrOk);              buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->service.SmsTrFailed);          buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->service.SmsTrBilled);          buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->service.UssdTrFromScOk);       buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->service.UssdTrFromScFailed);   buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->service.UssdTrFromSmeFailed);  buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->service.UssdTrFromSmeBilled);  buff.Append((uint8_t *)&value32, sizeof(value32));

            // Writes sme common errors count.
            value32 = smeStat->common.errors.Count(); 
            value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));

            IntHash<int>::Iterator sit = smeStat->common.errors.First();
            int secError, seCounter;
            while (sit.Next(secError, seCounter))
            {
                // Statistics for this errors
                value32 = htonl(secError);  buff.Append((uint8_t *)&value32, sizeof(value32));
                value32 = htonl(seCounter); buff.Append((uint8_t *)&value32, sizeof(value32));
            }
            smeStat = 0;
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

void StatisticsManager::tfseek(long offset, int whence)
{
    if (tfile && fseek(tfile, offset, whence)) {
        int error = ferror(tfile);
        Exception exc("Failed to seek traffic file. Details: %s", strerror(error));
        tfclose(); throw exc;
    }
}

void StatisticsManager::tfclose()
{
    if (tfile) { 
        fclose(tfile); tfile = 0;
    }
}

void StatisticsManager::tfopen()
{
    std::string loc = traffloc + std::string('/') + "traffic.dat";
    tfile = fopen(loc.c_str(), "rb+"); // open or create for update
    if (!tfile)
        throw Exception("Failed to create/open traffic file '%s'. Details: %s", 
                            loc.c_str(), strerror(errno));
}

void StatisticsManager::tfflush()
{
    if (tfile && fflush(tfile)) {
        int error = ferror(file);
        Exception exc("Failed to flush traffic file. Details: %s", strerror(error));
        tfclose(); throw exc;
    }
}

void StatisticsManager::tfwrite(const void* data, size_t size)
{
    if (tfile && fwrite(data, size, 1, tffile) != 1) {
        int error = ferror(file);
        Exception exc("Failed to write traffic file. Details: %s", strerror(error));
        tfclose(); throw exc;
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
    char buff[25];
    long mincnt_, hourcnt_, daycnt_, monthcnt_; 
    uint8_t year_, month_, day_, hour_, min_;

    IntHash<TrafficRecord>::Iterator traffit = traff.First();
    int routeId = 0;
    TrafficRecord routeTraff;
    while (traffit.Next(routeId, routeTraff))
    {
        // Copies routeId
        uint32_t val = htonl(routeId);
        memcpy((void*)buff, (const void*)(&val), 4);

        routeTraff.getRouteData(mincnt_, hourcnt_, daycnt_, monthcnt_, 
                                    year_, month_, day_, hour_, min_);

        // Copies counters
        uint32_t lval = mincnt_;
        val = htonl(lval);
        memcpy((void*)(buff + 4), (const void*)(&val), 4);
        lval = hourcnt_;
        val = htonl(lval);
        memcpy((void*)(buff + 8), (const void*)(&val), 4);
        lval = daycnt_;
        val = htonl(lval);
        memcpy((void*)(buff + 12), (const void*)(&val), 4);
        lval = monthcnt_;
        val = htonl(lval);
        memcpy((void*)(buff + 16), (const void*)(&val), 4);

        // Copies date
        memcpy((void*)(buff + 20), (const void*)(&year_), 1);
        memcpy((void*)(buff + 21), (const void*)(&month_), 1);
        memcpy((void*)(buff + 22), (const void*)(&day_), 1);
        memcpy((void*)(buff + 23), (const void*)(&hour_), 1);
        memcpy((void*)(buff + 24), (const void*)(&min_), 1);

        int offset = 0;
        tfseek(offset, SEEK_SET);
        tfwrite((const void*)buff, 25);
        tfflush();
    }
}

void initTraffic()
{
    if(!tfile)
        return;

    tfseek(0, SEEK_SET);

    char buff[25];
    uint32_t val;
    uint8_t year, month, day, hour, min;
    int id;
    long mincnt, hourcnt, daycnt, monthcnt;

    int read_size = -1;
    while( (read_size = fread(buff, 25, 1, tfile)) == 1){

        // Copies routeId
        memcpy((void*)&val, (const void*)buff, 4);
        id = ntohl(val);

        // Coies counters
        memcpy((void*)&val, (const void*)(buff + 4), 4);
        mincnt = ntohl(val);
        memcpy((void*)&val, (const void*)(buff + 8), 4);
        hourcnt = ntohl(val);
        memcpy((void*)&val, (const void*)(buff + 12), 4);
        daycnt = ntohl(val);
        memcpy((void*)&val, (const void*)(buff + 16), 4);
        monthcnt = ntohl(val);

        // Copies date
        memcpy((void*)&year,    (const void*)(buff + 20), 1);
        memcpy((void*)&month,   (const void*)(buff + 21), 1);
        memcpy((void*)&day,     (const void*)(buff + 22), 1);
        memcpy((void*)&hour,    (const void*)(buff + 23), 1);
        memcpy((void*)&min,     (const void*)(buff + 24), 1);

        TrafficRecord tr(mincnt, hourcnt, daycnt, monthcnt, 
                         year, month, day, hour, min);

        trafficByRouteId.Insert(id, tr);

    }

    if(read_size != 0){
        int err = ferror(tfile);
        throw Exception("Can't read route data from traffic file. Details: %s", strerror(err));
    }
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
