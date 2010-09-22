#ifndef _INFORMER_STOREJOURNAL_H
#define _INFORMER_STOREJOURNAL_H

namespace smsc {
namespace informer {

/// two functionality:
/// 1. journalling all activity on the working storage;
/// 2. restore working storage state on restart.
class StoreJournal
{
public:
    void journalMessage( dlvid_type dlvId,
                         regionid_type regionId,
                         Message& msg );
};

} // informer
} // smsc

#endif
