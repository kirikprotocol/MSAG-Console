#include <vector>
#include "MessageCache.h"

namespace eyeline {
namespace informer {

MessageCache::MessageCache( const DeliveryInfo& dlvInfo,
                            StoreJournal&       storeJournal,
                            InputMessageSource& source ) :
dlvInfo_(dlvInfo),
storeJournal_(storeJournal),
source_(source)
{
}


RegionalStoragePtr MessageCache::getRegionalStorage( regionid_type regionId,
                                                     bool create )
{
    MutexGuard mg(cacheLock_);
    RegionalStoragePtr* ptr = storages_.GetPtr(regionId);
    if (!ptr) {
        if (!create) {
            return RegionalStoragePtr();
        }
        return storages_.Insert(regionId,
                                RegionalStoragePtr(new RegionalStorage(dlvInfo_,
                                                                       regionId,
                                                                       storeJournal_,
                                                                       source_)));
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
