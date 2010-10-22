#ifndef _INFORMER_DELIVERYIMPL_H
#define _INFORMER_DELIVERYIMPL_H

#include "informer/data/Delivery.h"
#include "RegionalStorage.h"

namespace eyeline {
namespace informer {

class StoreJournal;

class DeliveryImpl : public Delivery
{
    friend class RegionalStorage;

public:
    DeliveryImpl( std::auto_ptr<DeliveryInfo> dlvInfo,
                  StoreJournal&               journal,
                  InputMessageSource*         source );

    virtual ~DeliveryImpl();

    /// get regional storage
    RegionalStoragePtr getRegionalStorage( regionid_type regId, bool create=false);

    void getRegionList( std::vector<regionid_type>& regIds );

    /// slowly dump all regions to storage
    size_t rollOverStore();

    /// slowly dump all input records to storage
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
    smsc::core::synchronization::Mutex                 cacheLock_;
    smsc::core::buffers::IntHash< RegionalStoragePtr > storages_;

    StoreJournal&                                      storeJournal_;
};

typedef EmbedRefPtr< DeliveryImpl >  DeliveryImplPtr;

} // informer
} // smsc

#endif
