#ifdef MOD_IDENT_ON
static char const ident[] = "@(#)$Id$";
#endif /* MOD_IDENT_ON */

#include "inman/storage/FileStorages.hpp"

namespace smsc {
namespace inman {
namespace filestore {

/* ************************************************************************** *
 * class InRollingFileStorage implementation:
 * ************************************************************************** */
InRollingFileStorage::InRollingFileStorage(const std::string & location,
                        const char *lastExt, const char *storageExt,
                        unsigned long rollInterval/* = 0*/,
                        const RollingFileStorageParms * parms/* = NULL*/,
                        Logger * uselog/* = NULL*/)
    : RollingFileStorage(location, lastExt, storageExt, rollInterval, parms)
{
    if (!(logger = uselog))
        logger = Logger::getInstance("smsc.inman.FileStore");
}

InRollingFileStorage::~InRollingFileStorage()
{ 
    if (_currFile.isOpened())
        RFSClose();
}

//Returns:  zero - Ok
//          positive - Ok, old storage files are rolled,
//          negative - error encountered and logged
int InRollingFileStorage::RFSOpen(bool rollOld /* = true */)
{
    bool  res;
    try { res = FSOpen(rollOld); }
    catch (FileSystemException & exc) {
        smsc_log_error(logger, "InRFStorage: %s", exc.what());
        return -1;
    }
    return (int)res;
}

void InRollingFileStorage::RFSRoll(void)
{
    try { FSRoll(); }
    catch (FileSystemException & exc) {
        smsc_log_error(logger, "InRFStorage: %s", exc.what());
    }
}

bool InRollingFileStorage::RFSWrite(const void* data, size_t size, bool roll/* = true*/)
{
    bool res;
    try { res = FSWrite(data, size, roll); }
    catch (FileSystemException & exc) {
        smsc_log_error(logger, "InRFStorage: %s", exc.what());
    }
    return res;
}

void InRollingFileStorage::RFSFlush(void)
{
    try { FSFlush(); }
    catch (FileSystemException & exc) {
        smsc_log_error(logger, "InRFStorage: %s", exc.what());
    }
}

void InRollingFileStorage::RFSClose(void)
{
    try { FSClose(); }
    catch (FileSystemException & exc) {
        smsc_log_error(logger, "InRFStorage: %s", exc.what());
    }
}

int  InRollingFileStorage::getRFStorageFiles(FSEntriesArray& files)
{
    int res;
    try { res = getStorageFiles(files); }
    catch (FileSystemException & exc) {
        smsc_log_error(logger, "InRFStorage: %s", exc.what());
    }
    return res;
}

time_t InRollingFileStorage::getLastRollingTime(void) const
{
    return _lastRollTime;
}

/* ************************************************************************** *
 * class InFileStorageRoller implementation:
 * ************************************************************************** */
#define ROLL_TIMEOUT_STEP 200 //millisecs
InFileStorageRoller::InFileStorageRoller(InRollingFileStorage * rfs/* = NULL*/,
                                         unsigned long rollInterval/* = 0*/)
    : _running(false)
{
    if (rfs && rollInterval)
        rfsMap.insert(RolledFSMap::value_type(rfs, rollInterval));
}

InFileStorageRoller::~InFileStorageRoller()
{ 
    if (_running)
        Stop(true);
//    Thread::WaitFor();
}

void InFileStorageRoller::attachRFS(InRollingFileStorage * rfs, unsigned long rollInterval)
{
    MutexGuard  tmp(_mutex);
    if (rfs && rollInterval)
        rfsMap.insert(RolledFSMap::value_type(rfs, rollInterval));
}

void InFileStorageRoller::detachRFS(InRollingFileStorage * rfs)
{
    MutexGuard  tmp(_mutex);
    rfsMap.erase(rfs);
}


void InFileStorageRoller::Start(void)
{
    MutexGuard grd(_mutex);
    if (!thread)
        Thread::Start(); //starts Execute() that waits for _mutex
    return;
}

void InFileStorageRoller::Stop(bool wait_for/* = false*/)
{
    {
        MutexGuard grd(_mutex);
        if (!_running)
            return;
        _running = false;
        _mutex.notify();
        if (!wait_for)
            return;
    }
    Thread::WaitFor();
}


time_t InFileStorageRoller::checkRoll(const RolledFSMap::iterator & it)
{
    InRollingFileStorage * rFS = (*it).first;
    time_t  nextTm = rFS->getLastRollingTime() + (*it).second; //interval;
    if (time(NULL) >= nextTm) {
        rFS->RFSRoll();
        nextTm = rFS->getLastRollingTime() + (*it).second; //interval;
    }
    return nextTm;
}

int InFileStorageRoller::Execute(void)
{
    _mutex.Lock();
    _running = true;
    while(_running) {
        time_t          awakeTm;
        int             sleepSecs;

        RolledFSMap::iterator it = rfsMap.begin();
        if (it != rfsMap.end()) {
            awakeTm = checkRoll(it);
            for (++it; it != rfsMap.end(); ++it) {
                time_t nextTm = checkRoll(it);
                if (nextTm < awakeTm)
                    awakeTm = nextTm;
            }
        } else
            awakeTm = time(NULL) + 600;

        if ((sleepSecs = (int)(awakeTm - time(NULL))) > 0)
            //NOTE: sleepSecs should be converted to millisecs, so check
            //for overflow. Consider sleepSecs*1000 ~~ sleepSecs*1024,
            //hence limit is 2**(32-10) = 2**22 = 4194304 = 0x400000
            _mutex.wait((sleepSecs > 0x400000 ? 0x400000 : sleepSecs)*1000);
    }
    _mutex.Unlock();
    return 0;
}

} //filestore
} //inman
} //smsc

