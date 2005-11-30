static char const ident[] = "$Id$";

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
                        const RollingFileStorageParms * parms/* = NULL*/, Logger * uselog/* = NULL*/)
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
InFileStorageRoller::InFileStorageRoller(InRollingFileStorage * rfs, unsigned long rollInterval)
    : _rFS(rfs), _running(false), _interval(rollInterval)
{
}

InFileStorageRoller::~InFileStorageRoller()
{ 
    if (_running)
        Stop();
    WaitFor();
}

void InFileStorageRoller::Stop(void)
{
    _mutex.Lock();
    _running = false;
    _mutex.Unlock();
    _mutex.notify();
}

int InFileStorageRoller::Execute(void)
{
    time_t          nextTm;
    int             sleepSecs;

    _running = true;
    while(_running) {
        _mutex.Unlock();
        if (time(NULL) >= (nextTm = (_rFS->getLastRollingTime() + _interval))) {
            _rFS->RFSRoll();
            nextTm = _rFS->getLastRollingTime() + _interval;
        }
        if ((sleepSecs = (int)(nextTm - time(NULL))) > 0)
            //NOTE: sleepSecs should be converted to millisecs, so check
            //for overflow. Consider sleepSecs*1000 ~~ sleepSecs*1024,
            //hence limit is 2**22 (= 4194304 = 0x400000)
            _mutex.wait(sleepSecs > 0x400000 ? 0x400000 : sleepSecs*1000);
        else
            _mutex.Lock();
    }
    return 0;
}

/* ************************************************************************** *
 * class InBillingFileStorage implementation:
 * ************************************************************************** */
//const unsigned SMSC_MIN_BILLING_INTERVAL = 10;
static const char*    _smsc_BILLING_HEADER_TEXT  =
    "MSG_ID,RECORD_TYPE,MEDIA_TYPE,BEARER_TYPE,SUBMIT,FINALIZED,STATUS,"
    "SRC_ADDR,SRC_IMSI,SRC_MSC,SRC_SME_ID,DST_ADDR,DST_IMSI,DST_MSC,DST_SME_ID,"
    "DIVERTED_FOR,ROUTE_ID,SERVICE_ID,USER_MSG_REF,DATA_LENGTH,PARTS_NUM\n";

static const RollingFileStorageParms _smsc_BILLING_STORAGE_parms = {
    _smsc_BILLING_HEADER_TEXT, 0, 0, NULL
};
static const char* _smsc_LAST_BILLING_FILE_EXTENSION = "lst";
static const char* _smsc_BILLING_FILE_EXTENSION = "csv";

InBillingFileStorage::InBillingFileStorage(const std::string & location,
                                            unsigned long rollInterval/* = 0*/,
                                            Logger * uselog /* = NULL */)
    : InRollingFileStorage(location,
                           _smsc_LAST_BILLING_FILE_EXTENSION,
                           _smsc_BILLING_FILE_EXTENSION, rollInterval,
                            &_smsc_BILLING_STORAGE_parms, uselog)
{ }

InBillingFileStorage::~InBillingFileStorage()
{ }

void InBillingFileStorage::bill(const CDRRecord & cdr)
{
    std::string rec;
    CDRRecord::csvEncode(cdr, rec);
    RFSWrite(rec.c_str(), rec.size());
    RFSFlush();
}

} //filestore
} //inman
} //smsc

