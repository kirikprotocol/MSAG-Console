#ifndef _INFORMER_STOREJOURNAL_H
#define _INFORMER_STOREJOURNAL_H

#include <string>
#include "informer/io/Typedefs.h"
#include "informer/io/FileGuard.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

class Message;

/// two functionality:
/// 1. journalling all activity on the working storage;
/// 2. restore working storage state on restart.
class StoreJournal
{
public:
    struct Reader {
        virtual ~Reader() {}
        virtual bool isStopping() const = 0;
        // NOTE: message is not bound to glossary yet.
        virtual void setRecordAtInit( dlvid_type     dlvId,
                                      regionid_type  regionId,
                                      Message&       msg,
                                      regionid_type  serial ) = 0;
        virtual void postInit() = 0;
    };

    /// @a path -- a path to the storage root.
    StoreJournal();

    /// journal messages.
    /// @a serial: input -- the serial number of the journal file at previous write,
    ///            at exit -- the serial number of the journal file.
    /// @return the number of bytes written
    size_t journalMessage( dlvid_type     dlvId,
                           regionid_type  regionId,
                           const Message& msg,
                           regionid_type& serial );

    void init( Reader& jr );

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
