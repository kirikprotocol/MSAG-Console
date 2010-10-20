#ifndef _INFORMER_DELIVERY_H
#define _INFORMER_DELIVERY_H

#include <memory>
#include "core/synchronization/Mutex.hpp"
#include "informer/io/EmbedRefPtr.h"
#include "informer/data/DeliveryInfo.h"
#include "RegionalStorage.h"

namespace eyeline {
namespace informer {

class StoreJournal;
class InfosmeCore;

class Delivery
{
    friend class EmbedRefPtr< Delivery >;
    friend class RegionalStorage;
public:
    Delivery( std::auto_ptr<DeliveryInfo> dlvInfo,
              StoreJournal&               journal,
              InputMessageSource*         source );

    ~Delivery();

    inline dlvid_type getDlvId() const { return dlvInfo_->getDlvId(); }

    const DeliveryInfo& getDlvInfo() const { return *dlvInfo_; }

    /// get regional storage
    RegionalStoragePtr getRegionalStorage( regionid_type regId, bool create=false);

    void getRegionList( std::vector<regionid_type>& regIds );

    void updateDlvInfo( const DeliveryInfo& info );

    void addNewMessages( MsgIter begin, MsgIter end ) {
        source_->addNewMessages(begin,end);
    }

    /// dump all regions to storage
    size_t rollOverStore();

    /// dump all input records to storage
    size_t rollOverInput() {
        return source_->rollOver();
    }

    // --- initialization part

    // NOTE: this method is invoked at init ONLY!
    void setRecordAtInit( const InputRegionRecord& rec,
                          uint64_t                 maxMsgId );

    // NOTE: this method is invoked at init ONLY!
    void setRecordAtInit( regionid_type            regionId,
                          Message&                 msg,
                          regionid_type            serial );

    // post process regions and collect empty ones
    void postInitInput( std::vector<regionid_type>& filledRegs ) {
        source_->postInit(filledRegs);
    }

    void postInitOperative( std::vector<regionid_type>& filledRegs,
                            std::vector<regionid_type>& emptyRegs );


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
    std::auto_ptr<DeliveryInfo>                        dlvInfo_;
    smsc::logger::Logger*                              log_;

    smsc::core::synchronization::Mutex                 cacheLock_;
    smsc::core::buffers::IntHash< RegionalStoragePtr > storages_;
    StoreJournal&                                      storeJournal_;
    InputMessageSource*                                source_;       // owned

    smsc::core::synchronization::Mutex lock_;
    unsigned                           ref_;
};

typedef EmbedRefPtr< Delivery >  DeliveryPtr;

} // informer
} // smsc

#endif
