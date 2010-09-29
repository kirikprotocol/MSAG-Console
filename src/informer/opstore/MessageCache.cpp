#include <vector>
#include "MessageCache.h"

namespace eyeline {
namespace informer {

MessageCache::MessageCache( const DeliveryInfo& dlvInfo,
                            StoreJournal&       storeLog,
                            InputMessageSource& messageSource ) :
dlvInfo_(&dlvInfo),
messageSource_(&messageSource),
storeLog_(&storeLog)
{
}


RegionalStoragePtr MessageCache::getRegionalStorage( regionid_type regionId )
{
    MutexGuard mg(cacheLock_);
    RegionalStoragePtr* ptr = storages_.GetPtr(regionId);
    if (!ptr) {
        return storages_.Insert(regionId,
                                RegionalStoragePtr(new RegionalStorage(*dlvInfo_,
                                                                       regionId,
                                                                       *storeLog_,
                                                                       *messageSource_)) );
    }
    return *ptr;
}


void MessageCache::rollOver()
{
    std::vector<RegionalStoragePtr> ptrs;
    {
        MutexGuard mg(cacheLock_);
        ptrs.reserve(storages_.Count());
        int id;
        RegionalStoragePtr* ptr;
        for ( smsc::core::buffers::IntHash< RegionalStoragePtr >::Iterator i(storages_);
              i.Next(id,ptr); ) {
            if (ptr->get()) ptrs.push_back(*ptr);
        }
    }
    for ( std::vector< RegionalStoragePtr >::iterator i = ptrs.begin();
          i != ptrs.end();
          ++i ) {
        (*i)->rollOver();
    }
}

}
}
