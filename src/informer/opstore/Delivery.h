#ifndef _INFORMER_DELIVERY_H
#define _INFORMER_DELIVERY_H

#include <memory>
#include "core/synchronization/Mutex.hpp"
#include "informer/io/EmbedRefPtr.h"
#include "informer/opstore/MessageCache.h"
#include "informer/data/DeliveryInfo.h"

namespace eyeline {
namespace informer {

class StoreJournal;
class InfosmeCore;

class Delivery
{
    friend class EmbedRefPtr< Delivery >;
public:
    Delivery( std::auto_ptr<DeliveryInfo> dlvInfo,
              StoreJournal&               journal,
              InputMessageSource*         source );

    inline dlvid_type getDlvId() const { return dlvInfo_->getDlvId(); }

    bool isActive() const {
        return dlvInfo_->isActive();
    }
    unsigned getPriority() const { return dlvInfo_->getPriority(); }

    /// get regional storage
    RegionalStoragePtr getRegionalStorage( regionid_type regId, bool create=false) {
        return cache_.getRegionalStorage(regId,create);
    }

    void updateDlvInfo( const DeliveryInfo& info );
    // void createRegionalStorage( regionid_type regId );

    void addNewMessages( MsgIter begin, MsgIter end ) {
        cache_.addNewMessages(begin,end);
    }

    // NOTE: should be invoked at start only!
    void setInputRecord( regionid_type            regionId,
                         const InputRegionRecord& rec,
                         uint64_t                 maxMsgId );

private:
    void ref() {
        smsc::core::synchronization::MutexGuard mg(lock_);
        ++ref_;
    }
    void unref() {
        smsc::core::synchronization::MutexGuard mg(lock_);
        if (ref_<=1) {
            delete this;
        } else {
            --ref_;
        }
    }

public:
    std::auto_ptr<DeliveryInfo> dlvInfo_;
    MessageCache                cache_;
    smsc::core::synchronization::Mutex lock_;
    unsigned                           ref_;
};

typedef EmbedRefPtr< Delivery >  DeliveryPtr;

} // informer
} // smsc

#endif
