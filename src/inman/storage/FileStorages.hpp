#ident "$Id$"
#ifndef _INMAN_FILE_STORAGES_H
#define _INMAN_FILE_STORAGES_H

#include "logger/Logger.h"
#include "inman/storage/FileStorageCore.hpp"
#include "inman/storage/cdrutil.hpp"

using smsc::logger::Logger;
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
    InRollingFileStorage(const std::string & location, const char *lastExt, const char *storageExt,
                         unsigned long rollInterval, const RollingFileStorageParms * parms = NULL,
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

protected:
    Logger *  logger;
};

class InBillingFileStorage : public InRollingFileStorage 
{
public:
    InBillingFileStorage(const std::string & location, unsigned long rollInterval, Logger * uselog  = NULL);
    ~InBillingFileStorage();

    void bill(const CDRRecord & cdr);
};

} //filestore
} //inman
} //smsc

#endif /* _INMAN_FILE_STORAGES_H */


