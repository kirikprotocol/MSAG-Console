#ifndef _INFORMER_MESSAGECACHE_H
#define _INFORMER_MESSAGECACHE_H

#include <vector>
#include "RegionalStorage.h"

namespace eyeline {
namespace informer {

/// the cache of messages for the task.
class MessageCache
{
public:
    MessageCache( const DeliveryInfo& dlvInfo,
                  StoreJournal&       storeJournal,
                  InputMessageSource* source );

    ~MessageCache();

    /// get the regional storage for given region.
    RegionalStoragePtr getRegionalStorage( regionid_type regionId, bool create=false );

    /// rolling over the whole storage.
    void rollOver();

    inline const DeliveryInfo& getDlvInfo() const { return dlvInfo_; }

    void addNewMessages( MsgIter begin, MsgIter end );

    InputMessageSource& getInputSource() const { return *source_; }

    void setRecordAtInit( regionid_type regionId,
                          Message& msg,
                          regionid_type serial );

    void postInit( std::vector<regionid_type>& filledRegs,
                   std::vector<regionid_type>& emptyRegs );

private:
    smsc::core::synchronization::Mutex                 cacheLock_;
    smsc::core::buffers::IntHash< RegionalStoragePtr > storages_;

    const DeliveryInfo&                                dlvInfo_;
    StoreJournal&                                      storeJournal_;
    InputMessageSource*                                source_; // owned
};

}
}

#endif
