#ifndef _INFORMER_DELIVERYIMPL_H
#define _INFORMER_DELIVERYIMPL_H

#include <list>
#include <vector>
#include "informer/data/Delivery.h"
#include "RegionalStorage.h"
#include "core/buffers/IntHash.hpp"

namespace smsc {
namespace util {
namespace config {
class Config;
}
}
}

namespace eyeline {
namespace informer {

class StoreJournal;

class DeliveryImpl : public Delivery
{
    friend class EmbedRefPtr< DeliveryImpl >;
    friend class RegionalStorage;

public:
    static void readDeliveryInfoData( dlvid_type            dlvId,
                                      DeliveryInfoData&     data );

    DeliveryImpl( DeliveryInfo*               dlvInfo,
                  StoreJournal*               journal,
                  InputMessageSource*         source,
                  DlvState                    state,
                  msgtime_type                planTime );

    virtual ~DeliveryImpl();

    // init state, invoked from core init to get the last state of the delivery.
    static DlvState readState( dlvid_type            dlvId,
                               msgtime_type&         planTime );

    virtual void updateDlvInfo( const DeliveryInfoData& info );

    inline const DeliveryInfo& getDlvInfo() const { return *dlvInfo_; }
    inline DeliveryInfo& getDlvInfo() { return *dlvInfo_; }

    /// set delivery state.
    /// NOTE: must be invoked from core, with proper preparation.
    virtual void setState( DlvState state, msgtime_type planTime = 0 );

    /// get regional storage
    RegionalStoragePtr getRegionalStorage( regionid_type regId, bool create=false);

    virtual void getRegionList( std::vector<regionid_type>& regIds ) const;

    /// slowly dump all regions to storage
    size_t rollOverStore( SpeedControl<usectime_type,tuPerSec>& speedControl );

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


    void detachEverything( bool cleanDirectory = false,
                           bool moveToArchive = false );

    /// check if all regional storages is empty, and no messages in retries.
    void checkFinalize();

    /// invoked to cancel all messages from operative storage
    void cancelOperativeStorage();

    void writeDeliveryInfoData();

private:
    typedef std::list< RegionalStoragePtr >            StoreList;
    typedef smsc::core::buffers::IntHash< StoreList::iterator > StoreHash;

    // inline DeliveryInfo* getDlvInfo() { return &activityLog_.getDlvInfo(); }
    // inline const DeliveryInfo* getDlvInfo() const { return &activityLog_.getDlvInfo(); }

    /// cacheLock must be locked
    StoreList::iterator* createRegionalStorage(regionid_type regId,
                                               msgtime_type next = 0 );

    void makeResendFilePath( char* fpath,
                             regionid_type regionId,
                             ulonglong ymdTime ) const;
    msgtime_type findNextResendFile( regionid_type regionId ) const;

private:
    mutable smsc::core::synchronization::Mutex         cacheLock_;
    StoreList                                          storeList_;
    StoreHash                                          storeHash_;
    StoreList::iterator                                rollingIter_;

    StoreJournal*                                      storeJournal_;
};

typedef EmbedRefPtr< DeliveryImpl >  DeliveryImplPtr;

} // informer
} // smsc

#endif
