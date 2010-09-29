#ifndef _INFORMER_DELIVERY_H
#define _INFORMER_DELIVERY_H

#include <memory>
#include "core/synchronization/Mutex.hpp"
#include "informer/io/EmbedRefPtr.h"
#include "informer/opstore/MessageCache.h"

namespace eyeline {
namespace informer {

class StoreJournal;
class InputMessageSource;

class Delivery
{
    friend class EmbedRefPtr< Delivery >;
public:
    Delivery( std::auto_ptr<DeliveryInfo> dlvInfo,
              StoreJournal&  storeLog,
              InputMessageSource& messageSource );

    inline dlvid_type getDlvId() const { return dlvInfo_->getDlvId(); }

    bool isActive() const {
        return dlvInfo_->isActive();
    }
    unsigned getPriority() const { return dlvInfo_->getPriority(); }

    /// get regional storage
    RegionalStoragePtr getRegionalStorage( regionid_type regId ) {
        return cache_.getRegionalStorage(regId);
    }

    void updateDlvInfo( const DeliveryInfo& info );

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
