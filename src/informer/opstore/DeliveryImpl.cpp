#include <cassert>
#include "DeliveryImpl.h"
#include "informer/data/CommonSettings.h"

namespace eyeline {
namespace informer {

DeliveryImpl::DeliveryImpl( std::auto_ptr<DeliveryInfo> dlvInfo,
                            StoreJournal&               journal,
                            InputMessageSource*         source ) :
Delivery(dlvInfo,source),
storeJournal_(journal)
{
    const dlvid_type dlvId = dlvInfo_->getDlvId();
    smsc_log_info(log_,"ctor D=%u done",dlvId);
}


DeliveryImpl::~DeliveryImpl()
{
    smsc_log_info(log_,"dtor D=%u",dlvInfo_->getDlvId());
    storages_.Empty();
}


msgtime_type DeliveryImpl::initState()
{
    FileGuard fg;
    char buf[200];
    DlvState state = DLVSTATE_PAUSED;
    msgtime_type planTime = 0;
    const dlvid_type dlvId = dlvInfo_->getDlvId();
    do {
        try {
            sprintf(makeDeliveryPath(dlvId,buf),"status");
            fg.ropen(buf);
        } catch ( InfosmeException& e ) {
            smsc_log_debug(log_,"D=%u cannot open status file",dlvId);
            break;
        }

        try {
            fg.seek(-sizeof(buf),SEEK_END);
        } catch ( ErrnoException& e ) {
            smsc_log_debug(log_,"D=%u cannot seek -%u backward from EOF",dlvId,unsigned(sizeof(buf)));
        }

        size_t wasread = fg.read(buf,sizeof(buf));
        if (wasread==0) {
            smsc_log_debug(log_,"D=%u status file is empty",dlvId);
            state = dlvInfo_->getState();
            break;
        }

        char* ptr = reinterpret_cast<char*>(const_cast<void*>(memrchr(buf,'\n',wasread)));
        if (!ptr) {
            smsc_log_warn(log_,"D=%u status record is not terminated",dlvId);
            break;
        }

        if (size_t(ptr - buf) != wasread) {
            smsc_log_warn(log_,"D=%u status file is not terminated",dlvId);
            break;
        }

        *ptr = '\0';
        wasread = ptr-buf-1;
        assert(wasread);
        ptr = reinterpret_cast<char*>(const_cast<void*>(memrchr(buf,'\n',wasread)));
        if (!ptr) { ptr = buf; }
        else { ++ptr; }

        // scanning record
        char cstate;
        DeliveryStats ds;
        ds.clear();
        ulonglong tmbuf;
        unsigned offset;
        int shift = 0;
        sscanf(ptr,"%c,%llu,%u,%u,%u,%u,%u,%u,%u,%u%n",
               &cstate,&tmbuf,
               &offset,
               &ds.totalMessages, &ds.procMessages, &ds.sentMessages,
               &ds.retryMessages, &ds.dlvdMessages, &ds.failedMessages,
               &ds.expiredMessages, &shift );
        if (!shift) {
            smsc_log_warn(log_,"could not parse status record");
            break;
        }

        switch (cstate) {
        case 'P' : {
            state = DLVSTATE_PLANNED;
            planTime = scanfMsgTime(tmbuf) + offset;
            break;
        }
        case 'S' : state = DLVSTATE_PAUSED; break;
        case 'A' : state = DLVSTATE_ACTIVE; break;
        case 'F' : state = DLVSTATE_FINISHED; break;
        case 'C' : state = DLVSTATE_CANCELLED; break;
        default:
            smsc_log_warn(log_,"D=%u cannot parse state",dlvId);
            state = DLVSTATE_PAUSED;
        }

        // FIXME: check activity log stats
        DeliveryStats ads;
        activityLog_.getStats(ads);
        if ( ads != ds ) {
            smsc_log_warn(log_,"D=%u: FIXME: stats discrepance act(status): %u(%u)/%u(%u)/%u(%u)/%u(%u)/%u(%u)/%u(%u)/%u(%u)",
                          dlvId,
                          ads.totalMessages, ds.totalMessages,
                          ads.procMessages,  ds.procMessages,
                          ads.sentMessages,  ds.sentMessages,
                          ads.retryMessages, ds.retryMessages,
                          ads.dlvdMessages,  ds.dlvdMessages,
                          ads.failedMessages, ds.failedMessages,
                          ads.expiredMessages, ds.expiredMessages );
        }
        
    } while (false);

    dlvInfo_->setState(state);
    return planTime;
}


void DeliveryImpl::setState( DlvState newState, msgtime_type planTime )
{
    MutexGuard mg(cacheLock_);
    const DlvState oldState = dlvInfo_->getState();
    const dlvid_type dlvId = dlvInfo_->getDlvId();
    smsc_log_debug(log_,"D=%u changing state: %s into %s",dlvId,
                   dlvStateToString(oldState), dlvStateToString(newState));
    if (oldState == newState) return;
    if (oldState == DLVSTATE_CANCELLED) {
        smsc_log_warn(log_,"D=%u is cancelled",dlvId);
        return;
    }
    const msgtime_type now( currentTimeMicro() / tuPerSec );
    if (newState == DLVSTATE_PLANNED) {
        if (planTime < now) {
            char buf[20];
            formatMsgTime(buf,planTime);
            throw InfosmeException("D=%u cannot plan delivery into past %s",dlvId,buf);
        }
        planTime -= now;
    } else {
        planTime = now;
    }
    dlvInfo_->setState(newState);
    int regId;
    RegionalStoragePtr regPtr;
    switch (newState) {
    case DLVSTATE_PLANNED: 
    case DLVSTATE_PAUSED:
    case DLVSTATE_FINISHED:
    case DLVSTATE_CANCELLED:
        for ( StoreHash::Iterator i(storages_); i.Next(regId,regPtr); ) {
            regPtr->stopTransfer(newState == DLVSTATE_CANCELLED);
        }
        break;
    case DLVSTATE_ACTIVE:
        // FIXME: activate, do nothing here?
        break;
    default:
        throw InfosmeException("unknown state %d",newState);
    }
    // write status line into status file
    char buf[200];
    sprintf(makeDeliveryPath(dlvId,buf),"status");
    FileGuard fg;
    fg.create((dlvInfo_->getCS().getStorePath() + buf).c_str(),true);
    fg.seek(0,SEEK_END);
    DeliveryStats ds;
    activityLog_.getStats(ds);
    char tmbuf[20];
    formatMsgTime(tmbuf,now);
    int buflen = sprintf(buf,"%c,%s,%u,%u,%u,%u,%u,%u,%u,%u\n",
                         dlvStateToString(newState)[0],
                         tmbuf,
                         planTime,
                         ds.totalMessages,
                         ds.procMessages,
                         ds.sentMessages,
                         ds.retryMessages,
                         ds.dlvdMessages,
                         ds.failedMessages,
                         ds.expiredMessages );
    assert(buflen>0);
    fg.write(buf,buflen);
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
