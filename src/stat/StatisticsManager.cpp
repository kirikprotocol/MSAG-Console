
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
#include <vector>

#include "StatisticsManager.h"

namespace smsc { namespace stat
{

const uint16_t SMSC_STAT_DUMP_INTERVAL = 60; // in seconds
const uint16_t SMSC_STAT_VERSION_INFO  = 0x0001;
const char*    SMSC_STAT_HEADER_TEXT   = "SMSC.STAT";
const char*    SMSC_STAT_DIR_NAME_FORMAT  = "%04d-%02d";
const char*    SMSC_STAT_FILE_NAME_FORMAT = "%02d.rts";

StatisticsManager::StatisticsManager(const std::string& _location)
    :  logger(Logger::getInstance("smsc.stat.StatisticsManager")),
       currentIndex(0), bExternalFlush(false), isStarted(false), storage(_location)
{     
    resetCounters(0); resetCounters(1);
}

StatisticsManager::~StatisticsManager()
{
  MutexGuard guard(stopLock);
}

void StatisticsManager::addError(IntHash<int>& hash, int errcode)
{
    int* counter = hash.GetPtr(errcode);
    if (!counter) hash.Insert(errcode, 1);
    else (*counter)++;
}

// SMS accepted by SMSC, affects accepted only
void StatisticsManager::updateAccepted(const StatInfo& info)
{
    MutexGuard  switchGuard(switchLock);
    
    statGeneral[currentIndex].accepted++; 
    statGeneral[currentIndex].incICounter();
    
    const char* srcSmeId = info.smeId.c_str();
    if (srcSmeId && srcSmeId[0])
    {
        SmsStat* stat = statBySmeId[currentIndex].GetPtr(srcSmeId);
        if (stat) stat->accepted++;
        else {
            statBySmeId[currentIndex].Insert(srcSmeId, SmsStat(1));
            stat = statBySmeId[currentIndex].GetPtr(srcSmeId);
        }
        if (stat) stat->incICounter();
    }
    
    const char* routeId = info.routeId.c_str();
    if (routeId && routeId[0])
    {
        RouteStat* stat = statByRoute[currentIndex].GetPtr(routeId);
        if (stat) stat->accepted++;
        else {
            RouteStat newStat(1, 0, 0, 0, 0, 0, info.providerId, info.categoryId); // accepted
            statByRoute[currentIndex].Insert(routeId, newStat);
            stat = statByRoute[currentIndex].GetPtr(routeId);
        }
        if (stat) stat->incICounter();
    }
}
// SMS rejected by SMSC. Affects rejected && errors only
void StatisticsManager::updateRejected(const StatInfo& info)
{
    MutexGuard  switchGuard(switchLock);

    if (info.errcode != 0)
    {
        statGeneral[currentIndex].rejected++;

        const char* srcSmeId = info.smeId.c_str();
        if (srcSmeId && srcSmeId[0])
        {
            SmsStat* stat = statBySmeId[currentIndex].GetPtr(srcSmeId);
            if (stat) {
                stat->rejected++;
                addError(stat->errors, info.errcode);
            }
            else {
                RouteStat newStat(0, 1, 0, 0, 0, 0, info.providerId, info.categoryId); // rejected
                addError(newStat.errors, info.errcode);
                statBySmeId[currentIndex].Insert(srcSmeId, newStat);
            }
        }

        const char* routeId = info.routeId.c_str();
        if (routeId && routeId[0])
        {
            RouteStat* stat = statByRoute[currentIndex].GetPtr(routeId);
            if (stat) {
                stat->rejected++;
                addError(stat->errors, info.errcode);
            }
            else {
                RouteStat newStat(0, 1, 0, 0, 0, 0, info.providerId, info.categoryId); // rejected
                addError(newStat.errors, info.errcode);
                statByRoute[currentIndex].Insert(routeId, newStat);
            }
        }
    }
    addError(statGeneral[currentIndex].errors, info.errcode);
}

// SMS was't delivered by SMSC with temporal error. Affects temporal && errors only 
void StatisticsManager::updateTemporal(const StatInfo& info)
{
    MutexGuard  switchGuard(switchLock);

    if (info.errcode != 0)
    {
        statGeneral[currentIndex].temporal++;

        const char* dstSmeId = info.smeId.c_str();
        if (dstSmeId && dstSmeId[0])
        {
            SmsStat* stat = statBySmeId[currentIndex].GetPtr(dstSmeId);
            if (stat) {
                stat->temporal++;
                addError(stat->errors, info.errcode);
            }
            else {
                SmsStat newStat(0, 0, 0, 0, 0, 1); // temporal
                addError(newStat.errors, info.errcode);
                statBySmeId[currentIndex].Insert(dstSmeId, newStat);
            }
        }
        
        const char* routeId = info.routeId.c_str();
        if (routeId && routeId[0])
        {
            RouteStat* stat = statByRoute[currentIndex].GetPtr(routeId);
            if (stat) {
                stat->temporal++;
                addError(stat->errors, info.errcode);
            }
            else {
                RouteStat newStat(0, 0, 0, 0, 0, 1, info.providerId, info.categoryId); // temporal
                addError(newStat.errors, info.errcode);
                statByRoute[currentIndex].Insert(routeId, newStat);
            }
        }
    }
    addError(statGeneral[currentIndex].errors, info.errcode);
}

// SMS was delivered or failed by SMSC. Affects delivered or failed
void StatisticsManager::updateChanged(const StatInfo& info)
{
    MutexGuard  switchGuard(switchLock);

    const char* dstSmeId = info.smeId.c_str();
    if (dstSmeId && dstSmeId[0])
    {
        SmsStat* stat = statBySmeId[currentIndex].GetPtr(dstSmeId);
        if (stat) {
            if (info.errcode == 0) stat->delivered++;
            else stat->failed++;
            addError(stat->errors, info.errcode);
        }
        else {
            SmsStat newStat(0, 0, (info.errcode) ? 0:1, (info.errcode) ? 1:0, 0, 0); // delivered or failed
            addError(newStat.errors, info.errcode);
            statBySmeId[currentIndex].Insert(dstSmeId, newStat);
            if (info.errcode == 0) {
                stat = statBySmeId[currentIndex].GetPtr(dstSmeId);
            }
        }
        if (info.errcode == 0 && stat) stat->incOCounter();
    }
    
    const char* routeId = info.routeId.c_str();
    if (routeId && routeId[0])
    {
        RouteStat* stat = statByRoute[currentIndex].GetPtr(routeId);
        if (stat) {
            if (info.errcode == 0) stat->delivered++;
            else stat->failed++;
            addError(stat->errors, info.errcode);
        }
        else {
            RouteStat newStat(0, 0, (info.errcode) ? 0:1, (info.errcode) ? 1:0, 0, 0,
                              info.providerId, info.categoryId); // delivered or failed
            addError(newStat.errors, info.errcode);
            statByRoute[currentIndex].Insert(routeId, newStat);
            if (info.errcode == 0) {
                stat = statByRoute[currentIndex].GetPtr(routeId);
            }
        }
        if (info.errcode == 0 && stat) stat->incOCounter();
    }
    
    if (info.errcode == 0) {
        statGeneral[currentIndex].delivered++;
        statGeneral[currentIndex].incOCounter();
    }
    else statGeneral[currentIndex].failed++;
    addError(statGeneral[currentIndex].errors, info.errcode);
}
void StatisticsManager::updateScheduled(const StatInfo& info)
{
    MutexGuard  switchGuard(switchLock);

    const char* dstSmeId = info.smeId.c_str();
    if (dstSmeId && dstSmeId[0])
    {
        SmsStat* stat = statBySmeId[currentIndex].GetPtr(dstSmeId);
        if (stat) stat->rescheduled++;
        else statBySmeId[currentIndex].Insert(dstSmeId, SmsStat(0, 0, 0, 0, 1, 0)); 
    }
    
    const char* routeId = info.routeId.c_str();
    if (routeId && routeId[0])
    {
        RouteStat* stat = statByRoute[currentIndex].GetPtr(routeId);
        if (stat) stat->rescheduled++;
        else {
            RouteStat newStat(0, 0, 0, 0, 1, 0, info.providerId, info.categoryId); // rescheduled
            statByRoute[currentIndex].Insert(routeId, newStat);
        }
    }
    
    statGeneral[currentIndex].rescheduled++;
}

int StatisticsManager::Execute()
{
    smsc_log_debug(logger, "Execute() started (%d)", isStopping);
    isStarted = true; bExternalFlush = false;
    while (!isStopping)
    {
        int toSleep = calculateToSleep();
        smsc_log_debug(logger, "Execute() >> Start wait %d", toSleep);
        awakeEvent.Wait(toSleep); // Wait for next interval begins ...
        smsc_log_debug(logger, "Execute() >> End wait");

        flushCounters(switchCounters());
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

void StatisticsManager::flushStatistics()
{
    MutexGuard flushGuard(flushLock);

    if (doneEvent.isSignaled()) doneEvent.Wait(0);
    bExternalFlush = true;
    awakeEvent.Signal();
    doneEvent.Wait();
}

short StatisticsManager::switchCounters()
{
    MutexGuard  switchGuard(switchLock);

    short flushIndex = currentIndex;
    currentIndex = (currentIndex == 0) ? 1:0;
    return flushIndex;
}

void StatisticsManager::calculateTime(tm& flushTM)
{
    time_t flushTime = time(0);
    if (!bExternalFlush) flushTime -= SMSC_STAT_DUMP_INTERVAL;
    gmtime_r(&flushTime, &flushTM); flushTM.tm_sec = 0;
}
int StatisticsManager::calculateToSleep() // returns msecs to next minute
{
    time_t currTime = time(0);
    time_t nextTime = currTime + SMSC_STAT_DUMP_INTERVAL;
    tm tmNT; localtime_r(&nextTime, &tmNT);
    tmNT.tm_sec = 0; nextTime = mktime(&tmNT);
    return (((nextTime-currTime)*1000)+1);
}

StatStorage::StatStorage(const std::string& _location)
    : logger(Logger::getInstance("smsc.stat.StatStorage")),
      location(_location), bFileTM(false), file(0) 
{
    if (!createStatDir()) 
        throw Exception("Can't open statistics directory: '%s'", location.c_str());
}
StatStorage::~StatStorage()
{
    close();
}

void StatStorage::close()
{
    if (file) { 
        bFileTM = false; fclose(file); file = 0;
    }
}
void StatStorage::flush()
{
    if (file && fflush(file)) {
        int error = ferror(file);
        Exception exc("Failed to flush file. Details: %s", strerror(error));
        close(); throw exc;
    }
}
void StatStorage::write(const void* data, size_t size)
{
    if (file && fwrite(data, size, 1, file) != 1) {
        int error = ferror(file);
        Exception exc("Failed to write file. Details: %s", strerror(error));
        close(); throw exc;
    }
}

bool StatStorage::createStatDir()
{
    const char* loc = location.c_str();
    int len = location.length();
    if (!loc || len <= 0) return false;
    else if (loc[0] == '/' && !loc[1]) return true;

    ++len;
    TmpBuf<char, 512> tmpBuff(len);
    char* buff = tmpBuff.get();
    memcpy(buff, loc, len);
    if (buff[len-2] == '/') {
       buff[len-2] = 0;
       if(len > 2 && (buff[len-3] == '/')) return false;
    }

    std::vector<char*> dirs(0);

    char* p1 = buff+1;
    int dirlen = 0;
    char* p2 = strchr(p1, '/');
    while(p2)
    {
       int len = p2 - p1;
       dirlen += len + 1;
       if (len == 0) return false;

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
        if (dirp) {
            closedir(dirp);            
        } else {
            try{
                createDir(std::string(*it));
            }catch(...){
                return false;
            }
        }
    }
    
    return true;
}

bool StatStorage::createDir(const std::string& dir)
{
    if (mkdir(dir.c_str(), S_IRWXU|S_IRWXG|S_IROTH|S_IXOTH) != 0) {
        if (errno == EEXIST) return false;
        throw Exception("Failed to create directory '%s'. Details: %s", 
                        dir.c_str(), strerror(errno));
    }
    return true;
}
void StatStorage::dump(const uint8_t* buff, int buffLen, const tm& flushTM)
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

uint64_t toNetworkOrder(uint64_t value)
{
    uint64_t result = 0;
    unsigned char *ptr=(unsigned char *)&result;
    ptr[0]=(value>>56)&0xFF; ptr[1]=(value>>48)&0xFF;
    ptr[2]=(value>>40)&0xFF; ptr[3]=(value>>32)&0xFF;
    ptr[4]=(value>>24)&0xFF; ptr[5]=(value>>16)&0xFF;
    ptr[6]=(value>>8 )&0xFF; ptr[7]=(value    )&0xFF;
    return result;
}
void StatisticsManager::flushCounters(short index)
{
    tm flushTM; calculateTime(flushTM);
    smsc_log_debug(logger, "Flushing statistics for %02d.%02d.%04d %02d:%02d:%02d GMT",
                   flushTM.tm_mday, flushTM.tm_mon+1, flushTM.tm_year+1900,
                   flushTM.tm_hour, flushTM.tm_min, flushTM.tm_sec);
    try
    {
        TmpBuf<uint8_t, 64096> buff(64096);

        // General statistics dump
        uint8_t value8 = 0;
        value8 = (uint8_t)(flushTM.tm_hour); buff.Append((uint8_t *)&value8, sizeof(value8));
        value8 = (uint8_t)(flushTM.tm_min);  buff.Append((uint8_t *)&value8, sizeof(value8));
        
        int32_t value32 = 0; 
        value32 = htonl(statGeneral[index].accepted);    buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(statGeneral[index].rejected);    buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(statGeneral[index].delivered);   buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(statGeneral[index].failed);      buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(statGeneral[index].rescheduled); buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(statGeneral[index].temporal);    buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(statGeneral[index].peak_i);      buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(statGeneral[index].peak_o);      buff.Append((uint8_t *)&value32, sizeof(value32));
	
	smsc_log_debug(logger, "General peak i/o: %d/%d", statGeneral[index].peak_i, statGeneral[index].peak_o);

        // General errors statistics dump
        value32 = statGeneral[index].errors.Count(); value32 = htonl(value32);
        buff.Append((uint8_t *)&value32, sizeof(value32));
        IntHash<int>::Iterator it = statGeneral[index].errors.First();
        int ecError, eCounter;
        while (it.Next(ecError, eCounter))
        {
            value32 = htonl(ecError);  buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(eCounter); buff.Append((uint8_t *)&value32, sizeof(value32));
        }

        // Sme statistics dump
        value32 = statBySmeId[index].GetCount(); 
        value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));
        statBySmeId[index].First();
        char* smeId = 0; SmsStat* smeStat = 0;
        while (statBySmeId[index].Next(smeId, smeStat))
        {
            if (!smeStat || !smeId || smeId[0] == '\0')
                throw Exception("Invalid sme stat record!");

            uint8_t smeIdLen = (uint8_t)strlen(smeId);
            buff.Append((uint8_t *)&smeIdLen, sizeof(smeIdLen));
            buff.Append((uint8_t *)smeId, smeIdLen);
            value32 = htonl(smeStat->accepted);    buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->rejected);    buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->delivered);   buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->failed);      buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->rescheduled); buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->temporal);    buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->peak_i);      buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(smeStat->peak_o);      buff.Append((uint8_t *)&value32, sizeof(value32));
	    
	    smsc_log_debug(logger, "Sme '%s' peak i/o: %d/%d", smeId, smeStat->peak_i, smeStat->peak_o);

            // Sme error statistics dump
            value32 = smeStat->errors.Count(); 
            value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));
            IntHash<int>::Iterator sit = smeStat->errors.First();
            int secError, seCounter;
            while (sit.Next(secError, seCounter))
            {
                value32 = htonl(secError);  buff.Append((uint8_t *)&value32, sizeof(value32));
                value32 = htonl(seCounter); buff.Append((uint8_t *)&value32, sizeof(value32));
            }
            smeStat = 0; // ???
        }

        // Route statistics dump
        value32 = statByRoute[index].GetCount(); 
        value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));
        statByRoute[index].First();
        char* routeId = 0; RouteStat* routeStat = 0;
        while (statByRoute[index].Next(routeId, routeStat))
        {
            if (!routeStat || !routeId || routeId[0] == '\0')
                throw Exception("Invalid route stat record!");

            uint8_t routeIdLen = (uint8_t)strlen(routeId);
            buff.Append((uint8_t *)&routeIdLen, sizeof(routeIdLen));
            buff.Append((uint8_t *)routeId, routeIdLen);

            int64_t value64 = 0;
            value64 = toNetworkOrder(routeStat->providerId); buff.Append((uint8_t *)&value64, sizeof(value64));
            value64 = toNetworkOrder(routeStat->categoryId); buff.Append((uint8_t *)&value64, sizeof(value64));

            value32 = htonl(routeStat->accepted);    buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->rejected);    buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->delivered);   buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->failed);      buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->rescheduled); buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->temporal);    buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->peak_i);      buff.Append((uint8_t *)&value32, sizeof(value32));
            value32 = htonl(routeStat->peak_o);      buff.Append((uint8_t *)&value32, sizeof(value32));
	    
	    smsc_log_debug(logger, "Route '%s' peak i/o: %d/%d", routeId, routeStat->peak_i, routeStat->peak_o);

            // Route errors statistics dump
            value32 = routeStat->errors.Count(); 
            value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));
            IntHash<int>::Iterator rit = routeStat->errors.First();
            int recError, reCounter;
            while (rit.Next(recError, reCounter))
            {
                value32 = htonl(recError);  buff.Append((uint8_t *)&value32, sizeof(value32));
                value32 = htonl(reCounter); buff.Append((uint8_t *)&value32, sizeof(value32));
            }
            routeStat = 0; // ???
        }

        storage.dump(buff, buff.GetPos(), flushTM);
    }
    catch (Exception& exc)
    {
        smsc_log_error(logger, "Statistics flush failed. Cause: %s", exc.what());
    }
    
    resetCounters(index);
}
void StatisticsManager::resetCounters(short index)
{
    statGeneral[index].Empty();
    statBySmeId[index].Empty();
    statByRoute[index].Empty();
}

}}
