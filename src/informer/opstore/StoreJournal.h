#ifndef _INFORMER_STOREJOURNAL_H
#define _INFORMER_STOREJOURNAL_H

#include <string>
#include "informer/io/Typedefs.h"
#include "informer/io/FileGuard.h"
#include "logger/Logger.h"

namespace eyeline {
namespace informer {

class Message;
class CommonSettings;

/// two functionality:
/// 1. journalling all activity on the working storage;
/// 2. restore working storage state on restart.
class StoreJournal
{
public:
    /// @a path -- a path to the storage root.
    StoreJournal( const CommonSettings& cs );

    /// journal messages.
    void journalMessage( dlvid_type     dlvId,
                         regionid_type  regionId,
                         const Message& msg );
private:
    smsc::logger::Logger* log_;
    const CommonSettings& cs_;
    uint32_t              version_;
    FileGuard             fg_;
};

} // informer
} // smsc

#endif
