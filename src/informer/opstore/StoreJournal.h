#ifndef _INFORMER_STOREJOURNAL_H
#define _INFORMER_STOREJOURNAL_H

#include <string>
#include "informer/io/Typedefs.h"
#include "informer/io/FileGuard.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

struct Message;
struct MessageLocker;

/// two functionality:
/// 1. journalling all activity on the working storage;
/// 2. restore working storage state on restart.
class StoreJournal
{
public:
    struct Reader {
        virtual ~Reader() {}
        // NOTE: message is not bound to glossary yet.
        virtual void setRecordAtInit( dlvid_type     dlvId,
                                      regionid_type  regionId,
                                      Message&       msg,
                                      regionid_type  serial ) = 0;
        virtual msgtime_type postInit(msgtime_type currentTime) = 0;
    };

    /// @a path -- a path to the storage root.
    StoreJournal();

    /// journal messages.
    /// @a msglock.serial: input -- the serial number of the journal file at previous write,
    ///            at exit -- the serial number of the journal file.
    /// @return the number of bytes written
    size_t journalMessage( dlvid_type     dlvId,
                           regionid_type  regionId,
                           MessageLocker& msglock );

    /// init the journal
    /// @return fixTime>0 if there were old version of deliveries
    msgtime_type init( Reader& jr, msgtime_type currentTime );

    void rollOver();

private:

    void readRecordsFrom( const std::string& path, Reader& reader );

private:
    smsc::logger::Logger*              log_;
    smsc::core::synchronization::Mutex lock_;
    uint32_t                           version_;
    FileGuard                          fg_;
    regionid_type                      serial_;
};

} // informer
} // smsc

#endif
