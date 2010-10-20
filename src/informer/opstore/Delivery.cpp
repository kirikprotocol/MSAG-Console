#include <cassert>
#include "Delivery.h"

namespace eyeline {
namespace informer {

Delivery::Delivery( std::auto_ptr<DeliveryInfo> dlvInfo,
                    StoreJournal&               journal,
                    InputMessageSource*         source ) :
dlvInfo_(dlvInfo),
storeJournal_(journal),
source_(source),
ref_(0)
{
    char buf[20];
    sprintf(buf,"dl.%u",dlvInfo_->getDlvId());
    log_ = smsc::logger::Logger::getInstance(buf);
    smsc_log_info(log_,"ctor D=%u",dlvInfo_->getDlvId());
}


Delivery::~Delivery()
{
    smsc_log_info(log_,"dtor D=%u",dlvInfo_->getDlvId());
    storages_.Empty();
    delete source_;
}


RegionalStoragePtr Delivery::getRegionalStorage( regionid_type regId, bool create )
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


void Delivery::getRegionList( std::vector< regionid_type >& regIds )
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


void Delivery::updateDlvInfo( const DeliveryInfo& info )
{
    assert( dlvInfo_.get() );
    assert(dlvInfo_->getDlvId() == info.getDlvId());
    // FIXME: update dlvinfo
    //*dlvInfo_.get() = info;
}


size_t Delivery::rollOverStore()
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


void Delivery::setRecordAtInit( const InputRegionRecord& rec,
                                uint64_t                 maxMsgId )
{
    source_->setRecordAtInit(rec,maxMsgId);
}


void Delivery::setRecordAtInit( regionid_type            regionId,
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


void Delivery::postInitOperative( std::vector<regionid_type>& filledRegs,
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
}

}
}
