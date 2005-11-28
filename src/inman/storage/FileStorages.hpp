#ident "$Id$"
#ifndef _INMAN_FILE_STORAGES_H
#define _INMAN_FILE_STORAGES_H

#include "logger/Logger.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "inman/storage/FileStorageCore.hpp"
#include "inman/storage/cdrutil.hpp"


using smsc::logger::Logger;
using smsc::core::synchronization::EventMonitor;
using smsc::core::threads::Thread;
using smsc::inman::cdr::CDRRecord;
using smsc::inman::filestore::RollingFileStorage;
using smsc::inman::filestore::RollingFileStorageParms;

namespace smsc {
namespace inman {
namespace filestore {

//just a wrapper for RollingFileStorage that catches exceptions and logs them.
class InRollingFileStorage : RollingFileStorage
{
public:
    //'rollInterval' == 0 forbids rolling by timer
    InRollingFileStorage(const std::string & location, const char *lastExt, const char *storageExt,
                         unsigned long rollInterval = 0, const RollingFileStorageParms * parms = NULL,
                         Logger * uselog = NULL);
    ~InRollingFileStorage();

    //Opens rolling file storage, able to init storage over existing one,
    //old files are rolled by default
    //NOTE: if rollOld == false, all old files will be deleted
    //Returns:  zero - Ok
    //          positive - Ok, old storage files found and rolled,
    //          negative - error encountered and logged
    int RFSOpen(bool rollOld = true);
    //roll file storage
    void RFSRoll(void);
    //returns true if storage was rolled (by time interval)
    //setting 'roll' = false forbids rolling at all
    bool RFSWrite(const void* data, size_t size, bool roll = true);
    void RFSFlush(void);
    //
    void RFSClose(void);
    //returns number of files in storage, appends to 'files' their names
    int  getRFStorageFiles(FSEntriesArray& files);
    time_t getLastRollingTime(void) const;

protected:
    Logger *  logger;
};


//Just an external roller for InRollingFileStorage
class InFileStorageRoller : public Thread
{
public:
    //NOTE: 'rollInterval' is measured in seconds
    InFileStorageRoller(InRollingFileStorage * rfs, unsigned long rollInterval);
    ~InFileStorageRoller();

    //thread entry point, automatically ran by Thread::Start()
    int  Execute(void);
    void Stop(void);

protected:
    InRollingFileStorage *  _rFS;        //storage to roll
    unsigned long           _interval;   //rolling interval, in seconds
    bool                    _running;
    EventMonitor            _mutex;
};

class InBillingFileStorage : public InRollingFileStorage 
{
public:
    InBillingFileStorage(const std::string & location, unsigned long rollInterval = 0, Logger * uselog  = NULL);
    ~InBillingFileStorage();

    void bill(const CDRRecord & cdr);
};

} //filestore
} //inman
} //smsc

#endif /* _INMAN_FILE_STORAGES_H */


