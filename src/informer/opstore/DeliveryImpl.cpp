#include <cassert>
#include "DeliveryImpl.h"

namespace eyeline {
namespace informer {

DeliveryImpl::DeliveryImpl( std::auto_ptr<DeliveryInfo> dlvInfo,
                            StoreJournal&               journal,
                            InputMessageSource*         source ) :
Delivery(dlvInfo,source),
storeJournal_(journal)
{
    smsc_log_info(log_,"ctor D=%u done",dlvInfo_->getDlvId());
}


DeliveryImpl::~DeliveryImpl()
{
    smsc_log_info(log_,"dtor D=%u",dlvInfo_->getDlvId());
    storages_.Empty();
}


RegionalStoragePtr DeliveryImpl::getRegionalStorage( regionid_type regId, bool create )
{
    MutexGuard mg(cacheLock_);
    RegionalStoragePtr* ptr = storages_.GetPtr(regId);
    if (!ptr) {
        if (!create) {
            return RegionalStoragePtr();
        }
        return storages_.Insert(regId,
                                RegionalStoragePtr(new RegionalStorage(*this,
                                                                       regId)));
    }
    return *ptr;
}


void DeliveryImpl::getRegionList( std::vector< regionid_type >& regIds )
{
    MutexGuard mg(cacheLock_);
    regIds.reserve(storages_.Count());
    int regId;
    RegionalStoragePtr* ptr;
    for (smsc::core::buffers::IntHash< RegionalStoragePtr >::Iterator i(storages_);
         i.Next(regId,ptr); ) {
        regIds.push_back(regionid_type(regId));
    }
}


size_t DeliveryImpl::rollOverStore()
{
    size_t written = 0;
    // FIXME: refactor to use list for rolling over
    smsc_log_debug(log_,"D=%u rolling store",dlvInfo_->getDlvId());
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
        written += (*i)->rollOver();
    }
    smsc_log_debug(log_,"D=%u rolling store done, written=%u",dlvInfo_->getDlvId(),written);
    return written;
}


void DeliveryImpl::setRecordAtInit( const InputRegionRecord& rec,
                                    uint64_t                 maxMsgId )
{
    source_->setRecordAtInit(rec,maxMsgId);
}


void DeliveryImpl::setRecordAtInit( regionid_type            regionId,
                                    Message&                 msg,
                                    regionid_type            serial )
{
    RegionalStoragePtr* ptr = storages_.GetPtr(regionId);
    if (!ptr) {
        ptr = &storages_.Insert(regionId,
                                RegionalStoragePtr(new RegionalStorage(*this,
                                                                       regionId)));
    }
    (*ptr)->setRecordAtInit(msg,serial);
}


void DeliveryImpl::postInitOperative( std::vector<regionid_type>& filledRegs,
                                      std::vector<regionid_type>& emptyRegs )
{
    int regId;
    RegionalStoragePtr* ptr;
    for ( smsc::core::buffers::IntHash< RegionalStoragePtr >::Iterator i(storages_);
          i.Next(regId,ptr); ) {
        if ( (*ptr)->postInit() ) {
            filledRegs.push_back(regionid_type(regId));
        } else {
            emptyRegs.push_back(regionid_type(regId));
            storages_.Delete(regId);
        }
    }
    DeliveryStats ds;
    activityLog_.getStats(ds);
    smsc_log_info(log_,"D=%u stats: total=%u proc=%u sent=%u retry=%u dlvd=%u fail=%u expd=%u",
                  dlvInfo_->getDlvId(),
                  ds.totalMessages, ds.procMessages,
                  ds.sentMessages, ds.retryMessages,
                  ds.dlvdMessages, ds.failedMessages,
                  ds.expiredMessages );
}

}
}
