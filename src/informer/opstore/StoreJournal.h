#ifndef _INFORMER_STOREJOURNAL_H
#define _INFORMER_STOREJOURNAL_H

#include "informer/io/Typedefs.h"
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
    void journalMessage( dlvid_type     dlvId,
                         regionid_type  regionId,
                         const Message& msg );
private:
    smsc::logger::Logger* log_;
    uint32_t              version_;
};

} // informer
} // smsc

#endif
