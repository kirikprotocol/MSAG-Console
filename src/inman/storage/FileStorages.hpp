#ifndef _INMAN_FILE_STORAGES_H
#ifndef __GNUC__
#ident "@(#)$Id$"
#endif
#define _INMAN_FILE_STORAGES_H

#include <map>

#include "logger/Logger.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "inman/storage/FileStorageCore.hpp"

using smsc::logger::Logger;
using smsc::core::synchronization::EventMonitor;
using smsc::core::threads::Thread;

namespace smsc {
namespace inman {
namespace filestore {

//just a wrapper for RollingFileStorage that catches exceptions and logs them.
class InRollingFileStorage : RollingFileStorage {
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


//Just an external roller for several InRollingFileStorages
class InFileStorageRoller : Thread {
private:
    using Thread::Start; //hide it to avoid annoying CC warnings

public:
    //NOTE: 'rollInterval' is measured in seconds
    InFileStorageRoller(InRollingFileStorage * rfs = NULL, unsigned long rollInterval = 0);
    ~InFileStorageRoller();

    void Start(void);
    void Stop(bool wait_for = false);

    //adds FileStorage to list of rolled ones or changes rolling interval
    void attachRFS(InRollingFileStorage * rfs, unsigned long rollInterval);
    void detachRFS(InRollingFileStorage * rfs);

protected:
    //thread entry point, automatically ran by Thread::Start()
    int  Execute(void);

    typedef std::map<InRollingFileStorage *, unsigned long> RolledFSMap;
    time_t checkRoll(const RolledFSMap::iterator & it);

    volatile bool       _running;
    EventMonitor        _mutex;
    RolledFSMap         rfsMap;
};

} //filestore
} //inman
} //smsc

#endif /* _INMAN_FILE_STORAGES_H */

