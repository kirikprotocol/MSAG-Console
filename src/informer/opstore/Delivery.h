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

    const DeliveryInfo& getDlvInfo() const { return *dlvInfo_; }

    bool isActive() const {
        return dlvInfo_->isActive();
    }
    unsigned getPriority() const { return dlvInfo_->getPriority(); }

    /// get regional storage
    RegionalStoragePtr getRegionalStorage( regionid_type regId, bool create=false) {
        return cache_.getRegionalStorage(regId,create);
    }

    void getRegionList( std::vector<regionid_type>& regIds ) {
        cache_.getRegionList(regIds);
    }

    void updateDlvInfo( const DeliveryInfo& info );
    // void createRegionalStorage( regionid_type regId );

    void addNewMessages( MsgIter begin, MsgIter end ) {
        cache_.addNewMessages(begin,end);
    }

    // NOTE: this method is invoked at init ONLY!
    void setRecordAtInit( const InputRegionRecord& rec,
                          uint64_t                 maxMsgId );

    // NOTE: this method is invoked at init ONLY!
    void setRecordAtInit( regionid_type            regionId,
                          Message&                 msg,
                          regionid_type            serial );

    // post process regions and collect empty ones
    void postInitInput( std::vector<regionid_type>& filledRegs );
    void postInitOperative( std::vector<regionid_type>& filledRegs,
                            std::vector<regionid_type>& emptyRegs ) {
        cache_.postInit(filledRegs,emptyRegs);
    }

    /// dump all regions to storage
    size_t rollOverStore() { return cache_.rollOver(); }

    /// dump all input records to storage
    size_t rollOverInput() { return cache_.getInputSource().rollOver(); }

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
