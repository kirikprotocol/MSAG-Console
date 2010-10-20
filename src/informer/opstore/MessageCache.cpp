#include <vector>
#include "MessageCache.h"
#include "informer/data/InputMessageSource.h"

namespace {
smsc::logger::Logger* log_ = 0;
void getlog() {
    if (!log_) {
        log_ = smsc::logger::Logger::getInstance("msgcache");
    }
}
}

namespace eyeline {
namespace informer {

MessageCache::MessageCache( DeliveryInfo&       dlvInfo,
                            StoreJournal&       storeJournal,
                            InputMessageSource* source ) :
dlvInfo_(dlvInfo),
storeJournal_(storeJournal),
source_(source)
{
    getlog();
    smsc_log_debug(log_,"ctor D=%u",dlvInfo.getDlvId());
}


MessageCache::~MessageCache()
{
    smsc_log_debug(log_,"dtor D=%u",dlvInfo_.getDlvId());
    storages_.Empty();
    delete source_;
    smsc_log_debug(log_,"dtor D=%u done",dlvInfo_.getDlvId());
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
                                                                       *source_)));
    }
    return *ptr;
}


void MessageCache::getRegionList( std::vector< regionid_type >& regIds )
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


size_t MessageCache::rollOver()
{
    size_t written = 0;
    // FIXME: refactor to use list for rolling over
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
    return written;
}


void MessageCache::addNewMessages( MsgIter begin, MsgIter end )
{
    source_->addNewMessages(begin,end);
}


void MessageCache::setRecordAtInit( regionid_type regionId,
                                    Message& msg,
                                    regionid_type serial )
{
    RegionalStoragePtr* ptr = storages_.GetPtr(regionId);
    if (!ptr) {
        ptr = &storages_.Insert(regionId,
                                RegionalStoragePtr(new RegionalStorage(dlvInfo_,
                                                                       regionId,
                                                                       storeJournal_,
                                                                       *source_)));
    }
    (*ptr)->setRecordAtInit(msg,serial);
}


void MessageCache::postInit( std::vector<regionid_type>& filledRegs,
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
