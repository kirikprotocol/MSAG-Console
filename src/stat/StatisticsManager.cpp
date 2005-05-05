
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
const char*    SMSC_TRNS_FILE_NAME_FORMAT = "%02d.trs";

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

void StatisticsManager::addError(IntHash<int>& hash, int errcode, int count/*=1*/)
{
    int* counter = hash.GetPtr(errcode);
    if (!counter) hash.Insert(errcode, count);
    else (*counter) += count;
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
                StatisticsManager::addError(stat->errors, info.errcode);
            }
            else {
                RouteStat newStat(0, 1, 0, 0, 0, 0, info.providerId, info.categoryId); // rejected
                StatisticsManager::addError(newStat.errors, info.errcode);
                statBySmeId[currentIndex].Insert(srcSmeId, newStat);
            }
        }

        const char* routeId = info.routeId.c_str();
        if (routeId && routeId[0])
        {
            RouteStat* stat = statByRoute[currentIndex].GetPtr(routeId);
            if (stat) {
                stat->rejected++;
                StatisticsManager::addError(stat->errors, info.errcode);
            }
            else {
                RouteStat newStat(0, 1, 0, 0, 0, 0, info.providerId, info.categoryId); // rejected
                StatisticsManager::addError(newStat.errors, info.errcode);
                statByRoute[currentIndex].Insert(routeId, newStat);
            }
        }
    }
    StatisticsManager::addError(statGeneral[currentIndex].errors, info.errcode);
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
                StatisticsManager::addError(stat->errors, info.errcode);
            }
            else {
                SmsStat newStat(0, 0, 0, 0, 0, 1); // temporal
                StatisticsManager::addError(newStat.errors, info.errcode);
                statBySmeId[currentIndex].Insert(dstSmeId, newStat);
            }
        }
        
        const char* routeId = info.routeId.c_str();
        if (routeId && routeId[0])
        {
            RouteStat* stat = statByRoute[currentIndex].GetPtr(routeId);
            if (stat) {
                stat->temporal++;
                StatisticsManager::addError(stat->errors, info.errcode);
            }
            else {
                RouteStat newStat(0, 0, 0, 0, 0, 1, info.providerId, info.categoryId); // temporal
                StatisticsManager::addError(newStat.errors, info.errcode);
                statByRoute[currentIndex].Insert(routeId, newStat);
            }
        }
    }
    StatisticsManager::addError(statGeneral[currentIndex].errors, info.errcode);
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
            StatisticsManager::addError(stat->errors, info.errcode);
        }
        else {
            SmsStat newStat(0, 0, (info.errcode) ? 0:1, (info.errcode) ? 1:0, 0, 0); // delivered or failed
            StatisticsManager::addError(newStat.errors, info.errcode);
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
            StatisticsManager::addError(stat->errors, info.errcode);
        }
        else {
            RouteStat newStat(0, 0, (info.errcode) ? 0:1, (info.errcode) ? 1:0, 0, 0,
                              info.providerId, info.categoryId); // delivered or failed
            StatisticsManager::addError(newStat.errors, info.errcode);
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
    StatisticsManager::addError(statGeneral[currentIndex].errors, info.errcode);
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
      location(_location), bFileTM(false), statFile(0) 
{
    if (!createStatDir()) 
        throw Exception("Can't open statistics directory: '%s'", location.c_str());
}
StatStorage::~StatStorage()
{
    if (statFile) fclose(statFile);
}

// Static members
bool StatStorage::read(FILE* file, void* data, size_t size)
{
    if (!file) return false;
    if (fread(data, size, 1, file) != 1) {
        if (feof(file)) {
            clearerr(file);
            return false;
        }
        int error = ferror(file);
        Exception exc("Failed to read from file. Details: %s", strerror(error));
        fclose(file); throw exc;
    }
    return true;
}
void StatStorage::write(FILE* file, const void* data, size_t size)
{
    if (file && fwrite(data, size, 1, file) != 1) {
        int error = ferror(file);
        Exception exc("Failed to write to file. Details: %s", strerror(error));
        fclose(file); throw exc;
    }
}
void StatStorage::flush(FILE* file)
{
    if (file && fflush(file)) {
        int error = ferror(file);
        Exception exc("Failed to flush file. Details: %s", strerror(error));
        fclose(file); throw exc;
    }
}
void StatStorage::seekPos(FILE* file, long offset, int whence)
{
    if (file && fseek(file, offset, whence)) {
        int error = ferror(file);
        Exception exc("Failed to seek %s. Details: %s", 
                      (whence == SEEK_END) ? "EOF":"BOF", strerror(error));
        fclose(file); throw exc;
    }
}
void StatStorage::getPos(FILE* file, fpos_t* pos)
{
    __require__(pos);

    if (file && fgetpos(file, pos) != 0) {
        int error = ferror(file);
        Exception exc("Failed to get file pos. Details: %s", strerror(error));
        fclose(file); throw exc;
    }
}
void StatStorage::setPos(FILE* file, const fpos_t* pos)
{
    __require__(pos);

    if (file && fsetpos(file, pos) != 0) {
        int error = ferror(file);
        Exception exc("Failed to set file pos. Details: %s", strerror(error));
        fclose(file); throw exc;
    }
}
void StatStorage::deleteFile(const char* path)
{
    if (remove(path) != 0)
        throw Exception("Failed to remove file '%s'. Details: %s", path, strerror(errno));
}
void StatStorage::truncateFile(const char* path, off_t length)
{
    if (truncate(path, length) != 0)
        throw Exception("Failed to truncate file '%s' to %d bytes. Details: %s",
                         path, length, strerror(errno));
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
uint64_t toHostOrder(uint64_t value)
{
    unsigned char *ptr=(unsigned char *)&value;
    return (((((uint64_t)ptr[0])<<56)&0xFF00000000000000)|
            ((((uint64_t)ptr[1])<<48)&0x00FF000000000000)|
            ((((uint64_t)ptr[2])<<40)&0x0000FF0000000000)|
            ((((uint64_t)ptr[3])<<32)&0x000000FF00000000)|
            ((((uint64_t)ptr[4])<<24)&0x00000000FF000000)|
            ((((uint64_t)ptr[5])<<16)&0x0000000000FF0000)|
            ((((uint64_t)ptr[6])<< 8)&0x000000000000FF00)|
            ((((uint64_t)ptr[7])    )&0x00000000000000FF));
}

const int MAX_STACK_BUFFER_SIZE = 64*1024;

void StatStorage::dump(const uint8_t* buff, int buffLen, const tm& flushTM)
{
    smsc_log_debug(logger, "Statistics dump called for %02d:%02d GMT", 
                   flushTM.tm_hour, flushTM.tm_min);
    
    FILE* trnsFile = 0;
    try 
    {
        char dirName[128]; char fileName[128]; 
        sprintf(dirName, SMSC_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
        sprintf(fileName, SMSC_STAT_FILE_NAME_FORMAT, flushTM.tm_mday);
        std::string fullPath = location; fullPath += '/'; fullPath += (const char*)dirName;
        std::string statPath = fullPath; statPath += '/'; statPath += (const char*)fileName; 
        const char* statPathStr = statPath.c_str();

        if (!bFileTM || fileTM.tm_mon != flushTM.tm_mon || fileTM.tm_year != flushTM.tm_year)
        {
            createDir(location + "/" + dirName); bFileTM = false;
            smsc_log_debug(logger, "New dir '%s' created", dirName);
        }

        bool needHeader = false;
        if (!bFileTM || fileTM.tm_mday != flushTM.tm_mday)
        {
            if (statFile) { // close old RTS file (if it was opened)
                fclose(statFile); statFile = 0;
            }

            needHeader = true;
            statFile = fopen(statPathStr, "r");
            if (statFile) { // file already exists (was opened for reading)
                fclose(statFile); statFile = 0; needHeader = false;
            }
            statFile = fopen(statPathStr, "ab+"); // open or create for append
            if (!statFile)
                throw Exception("Failed to create/open file '%s'. Details: %s", 
                                statPathStr, strerror(errno));

            fileTM = flushTM; bFileTM = true;
            smsc_log_debug(logger, "%s file '%s' %s", (needHeader) ? "New":"Existed",
                           fileName, (needHeader) ? "created":"opened");
        }

        sprintf(fileName, SMSC_TRNS_FILE_NAME_FORMAT, fileTM.tm_mday);
        std::string trnsPath = fullPath; trnsPath += '/'; trnsPath += (const char*)fileName; 
        const char* trnsPathStr = trnsPath.c_str();

        trnsFile = fopen(trnsPathStr, "r");
        if (trnsFile) // transaction file exists => last record(s) in RTS file invalid
        { 
            smsc_log_warn(logger, "Found transaction file '%s'", trnsPathStr);
            StatStorage::seekPos(trnsFile, 0, SEEK_SET); // set TRANS file position to BOF
            uint64_t pos;
            StatStorage::read(trnsFile, &pos, sizeof(pos));
            fpos_t fpos = (fpos_t)toHostOrder(pos);
            StatStorage::truncateFile(statPathStr, (off_t)fpos); // truncate RTS file
            StatStorage::seekPos(statFile, 0, SEEK_END); // set RTS file position to EOF
            if (fpos <= 0) needHeader = true; 
            smsc_log_warn(logger, "Rollback to RTS file position %lld in file '%s'",
                          fpos, statPathStr);
        } 
        else // create new TRANS file & write RTS position to it
        { 
            StatStorage::seekPos(statFile, 0, SEEK_END); // set RTS file position to EOF
            trnsFile = fopen(trnsPathStr, "ab+"); // open or create for append
            if (!trnsFile) 
                throw Exception("Failed to create/open file '%s'. Details: %s", 
                                trnsPathStr, strerror(errno));
            StatStorage::seekPos(trnsFile, 0, SEEK_SET); // set TRANS file position to BOF
            fpos_t fpos = 0;
            StatStorage::getPos(statFile, &fpos);
            uint64_t pos = toNetworkOrder((uint64_t)fpos);
            StatStorage::write(trnsFile, &pos, sizeof(pos));
            StatStorage::flush(trnsFile);
        }

        if (trnsFile) { fclose(trnsFile); trnsFile = 0; }

        TmpBuf<uint8_t, MAX_STACK_BUFFER_SIZE> writeBuff(MAX_STACK_BUFFER_SIZE);
        if (needHeader) // create header (if new file created)
        { 
            writeBuff.Append((uint8_t *)SMSC_STAT_HEADER_TEXT, strlen(SMSC_STAT_HEADER_TEXT));
            uint16_t version = htons(SMSC_STAT_VERSION_INFO);
            writeBuff.Append((uint8_t *)&version, sizeof(version));
        }
        uint32_t value32 = htonl(buffLen);
        writeBuff.Append((uint8_t *)&value32, sizeof(value32));
        writeBuff.Append((uint8_t *)buff, buffLen);
        writeBuff.Append((uint8_t *)&value32, sizeof(value32));
        StatStorage::write(statFile, (const void *)writeBuff, writeBuff.GetPos());
        StatStorage::flush(statFile);
        StatStorage::deleteFile(trnsPathStr); // drop TRANS file

        smsc_log_debug(logger, "Record dumped (%d bytes)", buffLen);
    }
    catch (Exception exc)
    {
        bFileTM = false;
        if (trnsFile) fclose(trnsFile); trnsFile = 0;
        if (statFile) fclose(statFile); statFile = 0; 
        throw exc;
    }
}

void StatisticsManager::flush(const tm& flushTM, StatStorage& _storage, SmsStat& general,
                              Hash<SmsStat>& statSme, Hash<RouteStat>& statRoute)
{
    
    TmpBuf<uint8_t, MAX_STACK_BUFFER_SIZE> buff(MAX_STACK_BUFFER_SIZE);

    // General statistics dump
    uint8_t value8 = 0;
    value8 = (uint8_t)(flushTM.tm_hour); buff.Append((uint8_t *)&value8, sizeof(value8));
    value8 = (uint8_t)(flushTM.tm_min);  buff.Append((uint8_t *)&value8, sizeof(value8));

    int32_t value32 = 0; 
    value32 = htonl(general.accepted);    buff.Append((uint8_t *)&value32, sizeof(value32));
    value32 = htonl(general.rejected);    buff.Append((uint8_t *)&value32, sizeof(value32));
    value32 = htonl(general.delivered);   buff.Append((uint8_t *)&value32, sizeof(value32));
    value32 = htonl(general.failed);      buff.Append((uint8_t *)&value32, sizeof(value32));
    value32 = htonl(general.rescheduled); buff.Append((uint8_t *)&value32, sizeof(value32));
    value32 = htonl(general.temporal);    buff.Append((uint8_t *)&value32, sizeof(value32));
    value32 = htonl(general.peak_i);      buff.Append((uint8_t *)&value32, sizeof(value32));
    value32 = htonl(general.peak_o);      buff.Append((uint8_t *)&value32, sizeof(value32));

    //smsc_log_debug(logger, "General peak i/o: %d/%d", statGeneral[index].peak_i, statGeneral[index].peak_o);

    // General errors statistics dump
    value32 = general.errors.Count(); value32 = htonl(value32);
    buff.Append((uint8_t *)&value32, sizeof(value32));
    IntHash<int>::Iterator it = general.errors.First();
    int ecError, eCounter;
    while (it.Next(ecError, eCounter))
    {
        value32 = htonl(ecError);  buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(eCounter); buff.Append((uint8_t *)&value32, sizeof(value32));
    }

    // Sme statistics dump
    value32 = statSme.GetCount(); 
    value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));
    statSme.First();
    char* smeId = 0; SmsStat* smeStat = 0;
    while (statSme.Next(smeId, smeStat))
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

        //smsc_log_debug(logger, "Sme '%s' peak i/o: %d/%d", smeId, smeStat->peak_i, smeStat->peak_o);

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
    value32 = statRoute.GetCount(); 
    value32 = htonl(value32); buff.Append((uint8_t *)&value32, sizeof(value32));
    statRoute.First();
    char* routeId = 0; RouteStat* routeStat = 0;
    while (statRoute.Next(routeId, routeStat))
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

        //smsc_log_debug(logger, "Route '%s' peak i/o: %d/%d", routeId, routeStat->peak_i, routeStat->peak_o);

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

    _storage.dump(buff, buff.GetPos(), flushTM);
}

void StatisticsManager::flushCounters(short index)
{
    tm flushTM; calculateTime(flushTM);
    smsc_log_debug(logger, "Flushing statistics for %02d.%02d.%04d %02d:%02d:%02d GMT",
                   flushTM.tm_mday, flushTM.tm_mon+1, flushTM.tm_year+1900,
                   flushTM.tm_hour, flushTM.tm_min, flushTM.tm_sec);
    try 
    {
        StatisticsManager::flush(flushTM, storage, statGeneral[index],
                                 statBySmeId[index], statByRoute[index]);
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
