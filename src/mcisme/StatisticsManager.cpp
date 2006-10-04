#include <dirent.h>
#include <unistd.h>
#include <errno.h>  
#include <vector>

#include "StatisticsManager.h"
#include "core/buffers/TmpBuf.hpp"

namespace smsc { namespace mcisme
{

using smsc::core::buffers::TmpBuf;

const char * SMSC_MCISME_STAT_DIR_NAME_FORMAT = "%04d-%02d";
const char * SMSC_MCISME_STAT_FILE_NAME_FORMAT = "%02d.rts";
const char * SMSC_MCISME_STAT_HEADER_TEXT = "SMSC.MCISME.STAT";
const uint16_t SMSC_MCISME_STAT_VERSION_INFO = 0x0001;
const uint16_t SMSC_MCISME_STAT_DUMP_INTERVAL = 60;

StatisticsManager::StatisticsManager(const std::string& loc)
    : Statistics(), Thread(), logger(0), processLog(0), 
            currentIndex(0), bExternalFlush(false), bStarted(false), 
            bNeedExit(false), location(loc)
{
    logger = Logger::getInstance("mci.statmgr");
    processLog = Logger::getInstance("mci.process");

    if(!createStorageDir(location))
        smsc_log_warn(logger, "Can't create directory: '%s'", location.c_str());

}
StatisticsManager::~StatisticsManager()
{
    Stop();
}

EventsStat StatisticsManager::getStatistics()
{
    MutexGuard guard(switchLock);
    return statistics[currentIndex];
}
void StatisticsManager::incMissed(const char* abonent)
{
    MutexGuard  guard(switchLock);
    statistics[currentIndex].missed++;
    smsc_log_info(processLog, "M %s", abonent ? abonent:"-");
}
void StatisticsManager::incDelivered(const char* abonent)
{
    MutexGuard  guard(switchLock);
    statistics[currentIndex].delivered++;
    smsc_log_info(processLog, "D %s", abonent ? abonent:"-");
}
void StatisticsManager::incFailed(const char* abonent)
{
    MutexGuard  guard(switchLock);
    statistics[currentIndex].failed++;
    smsc_log_info(processLog, "F %s", abonent ? abonent:"-");
}
void StatisticsManager::incNotified(const char* abonent)
{
    MutexGuard  guard(switchLock);
    statistics[currentIndex].notified++;
    smsc_log_info(processLog, "N %s", abonent ? abonent:"-");
}

void StatisticsManager::incMissed(uint8_t count)
{
    MutexGuard  guard(switchLock);
    statistics[currentIndex].missed+=count;
    smsc_log_info(processLog, "Missed calls count is %d (increased by %d, currentIndex=%d)", statistics[currentIndex].missed, count, currentIndex);
}
void StatisticsManager::incDelivered(uint8_t count)
{
    MutexGuard  guard(switchLock);
    statistics[currentIndex].delivered+=count;
    smsc_log_info(processLog, "Delivered Events is %d (increased by %d, currentIndex=%d)", statistics[currentIndex].delivered, count, currentIndex);
}
void StatisticsManager::incFailed(uint8_t count)
{
    MutexGuard  guard(switchLock);
    statistics[currentIndex].failed+=count;
    smsc_log_info(processLog, "Failed Events is %d (increased by %d, currentIndex=%d)", statistics[currentIndex].failed, count, currentIndex);
}
void StatisticsManager::incNotified(uint8_t count)
{
    MutexGuard  guard(switchLock);
    statistics[currentIndex].notified+=count;
    smsc_log_info(processLog, "Notified abonents is %d (increased by %d, currentIndex=%d)", statistics[currentIndex].notified, count, currentIndex);
}

int StatisticsManager::Execute()
{
    while (!bNeedExit)
    {
        int toSleep = calculateToSleep();
        smsc_log_debug(logger, "Start wait %d", toSleep);
        awakeEvent.Wait(toSleep); // Wait for next hour begins ...
        smsc_log_debug(logger, "End wait");

        flushCounters(switchCounters());
        bExternalFlush = false;
        doneEvent.Signal();
        smsc_log_debug(logger, "Statistics flushed");
    }
    exitEvent.Signal();
    return 0;
}

void StatisticsManager::Start()
{
    MutexGuard guard(startLock);
    
    if (!bStarted)
    {
        smsc_log_info(logger, "Starting ...");
        bExternalFlush = false;
        bNeedExit = false;
        awakeEvent.Wait(0);
        Thread::Start();
        bStarted = true;
        smsc_log_info(logger, "Started.");
    }
}
void StatisticsManager::Stop()
{
    MutexGuard  guard(startLock);
    
    if (bStarted)
    {
        smsc_log_info(logger, "Stopping ...");
        bExternalFlush = true;
        bNeedExit = true;
        awakeEvent.Signal();
        exitEvent.Wait();
        bStarted = false;
        smsc_log_info(logger, "Stoped.");
    }
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
    currentIndex = (currentIndex == 0) ? 1:0;	//:))))))))))
    return flushIndex;
}

/*uint32_t StatisticsManager::calculatePeriod()
{
    time_t currTime = time(0);
    if (!bExternalFlush) currTime -= 3600;
    tm tmCT; localtime_r(&currTime, &tmCT);
    return  (tmCT.tm_year+1900)*1000000+(tmCT.tm_mon+1)*10000+(tmCT.tm_mday)*100+tmCT.tm_hour;
}*/
int StatisticsManager::calculateToSleep() // returns msecs to next hour
{
	return SMSC_MCISME_STAT_DUMP_INTERVAL*1000;

    //time_t currTime = time(0);
    //time_t nextTime = currTime + SMSC_MCISME_STAT_DUMP_INTERVAL;
    //tm tmNT; localtime_r(&nextTime, &tmNT);
    //tmNT.tm_sec = 0; tmNT.tm_min = 0;
    //nextTime = mktime(&tmNT);
    //return (((nextTime-currTime)*1000)+1);
}

/*const char* INSERT_EVENTS_STAT_STATE_ID = "INSERT_EVENTS_STAT_STATE_ID";
const char* INSERT_EVENTS_STAT_STATE_SQL = (const char*)
"INSERT INTO MCISME_STAT (period, missed, delivered, failed, notified) "
"VALUES (:period, :missed, :delivered, :failed, :notified)";*/

/*void StatisticsManager::flushCounters(short index)
{
    if (statistics[index].isEmpty()) return;

    uint32_t period = calculatePeriod();
    smsc_log_debug(logger, "Flushing statistics for period: %lu / %lu", period, time(NULL));

    try
    {
        Statement* statement = connection->getStatement(INSERT_EVENTS_STAT_STATE_ID, 
                                                        INSERT_EVENTS_STAT_STATE_SQL);
        if (!statement)
            throw Exception("Failed to obtain statement for statistics update");
        
        statement->setUint32(1, period);
        statement->setUint32(2, statistics[index].missed);
        statement->setUint32(3, statistics[index].delivered);
        statement->setUint32(4, statistics[index].failed);
        statement->setUint32(5, statistics[index].notified);

        statement->executeUpdate();
        connection->commit();
    }
    catch (Exception& exc)
    {
        try { if (connection) connection->rollback(); }
        catch (Exception& exc) {
            smsc_log_error(logger, "Failed to roolback transaction (statistics). "
                         "Details: %s", exc.what());
        } catch (...) {
            smsc_log_error(logger, "Failed to roolback transaction (statistics).");
        }
        smsc_log_error(logger, "Error occurred during statistics flushing. Details: %s", exc.what());
    }

    statistics[index].Empty();
}*/

void StatisticsManager::flushCounters(short index)
{

    if (statistics[index].isEmpty()) return;

    tm flushTM; calculateTime(flushTM);
    smsc_log_debug(logger, "Flushing statistics for %02d.%02d.%04d %02d:%02d:%02d GMT",
                   flushTM.tm_mday, flushTM.tm_mon+1, flushTM.tm_year+1900,
                   flushTM.tm_hour, flushTM.tm_min, flushTM.tm_sec);

    try
    {
        TmpBuf<uint8_t, 128> buff(128);

        // Head of record
        uint8_t value8 = 0;
        value8 = (uint8_t)(flushTM.tm_hour); buff.Append((uint8_t *)&value8, sizeof(value8));

        // Counters
        int32_t value32 = 0;
        value32 = htonl(statistics[index].missed);         buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(statistics[index].delivered);      buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(statistics[index].failed);         buff.Append((uint8_t *)&value32, sizeof(value32));
        value32 = htonl(statistics[index].notified);       buff.Append((uint8_t *)&value32, sizeof(value32));

        dumpCounters(buff, buff.GetPos(), flushTM);

    }
    catch (Exception& exc)
    {
        smsc_log_error(logger, "Statistics flush failed. Cause: %s", exc.what());
    }

    statistics[index].Empty();

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
        sprintf(dirName, SMSC_MCISME_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
        std::string loc = location + "/" + dirName;
        smsc_log_debug(logger, "New dir '%s' created", loc.c_str());
        createDir(location + "/" + dirName); bFileTM = false; hasDir = true;
        smsc_log_debug(logger, "New dir '%s' created", dirName);
    }

    if (!bFileTM || fileTM.tm_mday != flushTM.tm_mday)
    {
        char fileName[128]; 
        std::string fullPath = location;
        if (!hasDir) sprintf(dirName, SMSC_MCISME_STAT_DIR_NAME_FORMAT, flushTM.tm_year+1900, flushTM.tm_mon+1);
        sprintf(fileName, SMSC_MCISME_STAT_FILE_NAME_FORMAT, flushTM.tm_mday);
        fullPath += '/'; fullPath += (const char*)dirName; 
        fullPath += '/'; fullPath += (const char*)fileName; 
        const char* fullPathStr = fullPath.c_str();

        if (file.isOpened()) file.Close();
            
        bool needHeader = true;
        if (File::Exists(fullPathStr)) { 
            needHeader = false;
            file.WOpen(fullPathStr);
			file.SeekEnd(0);
        } else {
            file.RWCreate(fullPathStr);
        }

        if (needHeader) { // create header (if new file created)
            file.Write(SMSC_MCISME_STAT_HEADER_TEXT, strlen(SMSC_MCISME_STAT_HEADER_TEXT));
            uint16_t version = htons(SMSC_MCISME_STAT_VERSION_INFO);
            file.Write(&version, sizeof(version));
            file.Flush();
        }
        fileTM = flushTM; bFileTM = true;
        smsc_log_debug(logger, "%s file '%s' %s", (needHeader) ? "New":"Existed",
                       fileName, (needHeader) ? "created":"opened");
    }
    
    smsc_log_debug(logger, "Statistics data dump...");
    uint32_t value32 = htonl(buffLen);
//    file.Write((const void *)&value32, sizeof(value32));
    file.Write((const void *)buff, buffLen); // write dump to it
//    file.Write((const void *)&value32, sizeof(value32));		// νΰ υσÿ??????
    file.Flush();
    smsc_log_debug(logger, "Statistics data dumped.");

    }catch(std::exception & exc){
        if (file.isOpened()) file.Close();
        bFileTM = false;
        throw exc;
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

void StatisticsManager::calculateTime(tm& flushTM)
{
    time_t flushTime = time(0);
    if (!bExternalFlush) flushTime -= SMSC_MCISME_STAT_DUMP_INTERVAL;
    localtime_r(&flushTime, &flushTM); flushTM.tm_sec = 0; flushTM.tm_min = 0;
}

}}                                                            
