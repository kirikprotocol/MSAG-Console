#ifndef _INFORMER_MESSAGECACHE_H
#define _INFORMER_MESSAGECACHE_H

#include "RegionalStorage.h"

namespace eyeline {
namespace informer {

/// the cache of messages for the task.
class MessageCache
{
public:
    MessageCache( const DeliveryInfo& dlvInfo,
                  StoreJournal&       storeLog,
                  InputMessageSource& messageSource );

    /// get the regional storage for given region.
    RegionalStoragePtr getRegionalStorage( regionid_type regionId );

    /// rolling over the whole storage.
    void rollOver();

    const DeliveryInfo& getDlvInfo() const { return *dlvInfo_; }

private:
    smsc::core::synchronization::Mutex                 cacheLock_;
    smsc::core::buffers::IntHash< RegionalStoragePtr > storages_;
    const DeliveryInfo*                                dlvInfo_;
    InputMessageSource*                                messageSource_;
    StoreJournal*                                      storeLog_;
};

}
}

#endif
