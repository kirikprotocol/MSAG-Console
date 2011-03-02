#include <cassert>
#include "DeliveryImpl.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/DeliveryActivator.h"
#include "informer/data/UserInfo.h"
#include "informer/data/BindSignal.h"
#include "util/config/Config.h"
#include "informer/io/ConfigWrapper.h"
#include "informer/io/FileGuard.h"

using smsc::util::config::Config;

#ifdef sun
namespace {
void* memrchr(const void* s, int c, size_t n)
{
    if (!n) return 0;
    for ( register const char* x = reinterpret_cast<const char*>(s)+n; x != s; ) {
        if (*--x == c) {
            return const_cast<char*>(x);
        }
    }
    return 0;
}
}
#endif

namespace eyeline {
namespace informer {

void DeliveryImpl::readDeliveryInfoData( dlvid_type            dlvId,
                                         DeliveryInfoData&     data )
{
    try {
        char buf[100];
        sprintf(makeDeliveryPath(buf,dlvId),"/config.xml");

        std::auto_ptr<Config> cfg(Config::createFromFile((getCS()->getStorePath()+buf).c_str()));
        char logname[20];
        sprintf(logname,"dlcf.%05u",dlvId % 10000);
        const ConfigWrapper config(*cfg.get(),smsc::logger::Logger::getInstance(logname));

        data.name = config.getString("name");
        data.priority = config.getInt("priority",1,1,100,false);
        data.transactionMode = config.getBool("transactionMode",false);
        data.startDate = config.getString("startDate","");
        data.endDate = config.getString("endDate","");
        data.activePeriodStart = config.getString("activePeriodStart","");
        data.activePeriodEnd = config.getString("activePeriodEnd","");
        data.activeWeekDays.clear();
        {
            std::string awd = config.getString("activeWeekDays","mon,tue,wed,thu,fri");
            std::vector< std::string > res;
            for ( size_t start = 0; start < awd.size(); ++start ) {
                while ( start < awd.size() && awd[start] == ' ' ) {
                    ++start;
                }
                if ( start >= awd.size() ) { break; }
                size_t comma = awd.find(',',start);
                if ( comma == std::string::npos ) {
                    comma = awd.size();
                }
                size_t end = comma - 1;
                while ( end > start && awd[end] == ' ') {
                    --end;
                }
                if (start < end) {
                    res.push_back( std::string(awd, start, end-start+1) );
                }
                start = comma;
            }
            data.activeWeekDays = res;
        }

        data.validityPeriod = config.getString("validityPeriod","");
        data.archivationPeriod = config.getString("archivationPeriod","");
        data.flash = config.getBool("flash",false);
        data.useDataSm = config.getBool("useDataSm",false);
        const std::string dlvMode = config.getString("deliveryMode","sms");
        if ( dlvMode == "sms" ) {
            data.deliveryMode = DLVMODE_SMS;
        } else if ( dlvMode == "ussdpush" ) {
            data.deliveryMode = DLVMODE_USSDPUSH;
        } else if ( dlvMode == "ussdpushvlr" ) {
            data.deliveryMode = DLVMODE_USSDPUSHVLR;
        } else {
            throw InfosmeException(EXC_CONFIG,"unknown delivery mode: '%s'",dlvMode.c_str());
        }
        data.owner = config.getString("owner");
        data.retryOnFail = config.getBool("retryOnFail",true);
        data.retryPolicy = config.getString("retryPolicy","1s:*");
        data.replaceMessage = config.getBool("replaceMessage",false,false);
        data.svcType = config.getString("svcType","");
        data.userData = config.getString("userData","");
        data.sourceAddress = config.getString("sourceAddress");
        data.finalDlvRecords = config.getBool("finalDlvRecords",false);
        data.finalMsgRecords = config.getBool("finalMsgRecords",false);

    } catch (std::exception& e) {
        throw InfosmeException(EXC_CONFIG,"D=%u config, exc: %s",dlvId,e.what());
    }
}


DeliveryImpl::DeliveryImpl( DeliveryInfo*               dlvInfo,
                            StoreJournal*               journal,
                            InputMessageSource*         source,
                            DlvState                    state,
                            msgtime_type                planTime ) :
Delivery(dlvInfo,source),
storeJournal_(journal)
{
    state_ = state;
    planTime_ = planTime;
    const dlvid_type dlvId = dlvInfo_->getDlvId();
    writeDeliveryInfoData();
    smsc_log_debug(log_,"ctor D=%u done",dlvId);
}


DeliveryImpl::~DeliveryImpl()
{
}


DlvState DeliveryImpl::readState( dlvid_type            dlvId,
                                  msgtime_type&         planTime )
{
    // we may not lock here, as we have exclusive access to the delivery
    planTime = 0;
    DlvState state = DLVSTATE_PAUSED;
    char buf[200];
    sprintf(makeDeliveryPath(buf,dlvId),"status.log");
    FileGuard fg;
    try {
        fg.ropen((getCS()->getStorePath()+buf).c_str());
    } catch ( InfosmeException& e ) {
        if ( e.getCode() == EXC_SYSTEM ) {
            return state;
        }
        throw;
    }
    struct stat st;
    const size_t pos = fg.getStat(st).st_size > sizeof(buf) ?
        st.st_size - sizeof(buf) : 0;
    if (fg.seek(pos) != pos) {
        throw InfosmeException(EXC_SYSTEM,"D=%u cannot seek status file to %u",dlvId,unsigned(pos));
    }

    size_t wasread = fg.read(buf,sizeof(buf));
    if (wasread==0) {
        throw InfosmeException(EXC_BADFILE,"D=%u status file is empty",dlvId);
    }

    char* ptr = reinterpret_cast<char*>(const_cast<void*>(::memrchr(buf,'\n',wasread)));
    if (!ptr) {
        throw InfosmeException(EXC_BADFILE,"D=%u status record is not terminated",dlvId);
    }

    if (size_t(ptr - buf)+1 != wasread) {
        throw InfosmeException(EXC_BADFILE,"D=%u status file is not terminated",dlvId);
    }

    *ptr = '\0';
    --wasread;
    if (!wasread) {
        throw InfosmeException(EXC_BADFILE,"D=%u last record is too short",dlvId);
    }
    ptr = reinterpret_cast<char*>(const_cast<void*>(::memrchr(buf,'\n',wasread)));
    if (!ptr) { ptr = buf; }
    else { ++ptr; }

    // scanning record
    char cstate;
    DeliveryStats ds;
    ds.clear();
    ulonglong ymdTime;
    unsigned offset;
    int shift = 0;
    sscanf(ptr,"%llu,%c,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u%n",
           &ymdTime, &cstate,
           &offset,
           &ds.totalMessages, &ds.procMessages, &ds.sentMessages,
           &ds.retryMessages, &ds.dlvdMessages, &ds.failedMessages,
           &ds.expiredMessages, &ds.dlvdSms, &ds.failedSms,
           &ds.expiredSms, &ds.killedMessages, &shift );
    if (!shift) {
        throw InfosmeException(EXC_BADFILE,"D=%u bad last status record",dlvId);
    }

    if (offset>0) { planTime = offset + ymdToMsgTime(ymdTime); }
    else { planTime = 0; }

    switch (cstate) {
    case 'P' : state = DLVSTATE_PLANNED; break;
    case 'S' : state = DLVSTATE_PAUSED; break;
    case 'A' : state = DLVSTATE_ACTIVE; break;
    case 'F' : state = DLVSTATE_FINISHED; break;
    case 'C' : state = DLVSTATE_CANCELLED; break;
    default:
        throw InfosmeException(EXC_BADFILE,"D=%u cannot parse state",dlvId);
    }
    return state;
}


void DeliveryImpl::updateDlvInfo( const DeliveryInfoData& data )
{
    if ( getCS()->isArchive() || getCS()->isEmergency() ) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive/emergency mode");
    }
    // should we unbind first?
    MutexGuard mg(stateLock_);
    dlvInfo_->update( data );
    writeDeliveryInfoData();
}


void DeliveryImpl::setState( DlvState newState, msgtime_type planTime )
{
    if ( getCS()->isArchive() || getCS()->isEmergency() ) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive/emergency mode");
    }
    const dlvid_type dlvId = dlvInfo_->getDlvId();
    BindSignal bs;
    msgtime_type now;
    ulonglong ymd;
    {
        MutexGuard mg(stateLock_);
        const DlvState oldState = state_;
        if (oldState == newState) return;
        smsc_log_debug(log_,"D=%u setState: %s into %s",dlvId,
                       dlvStateToString(oldState), dlvStateToString(newState));
        if (oldState == DLVSTATE_CANCELLED) {
            throw InfosmeException(EXC_LOGICERROR,
                                  "D=%u is cancelled",dlvId);
        }
        now = currentTimeSeconds();
        struct tm tmnow;
        ymd = msgTimeToYmd(now,&tmnow);
        if (newState == DLVSTATE_PLANNED) {
            if (planTime <= now) {
                planTime = now+1;
            }
        } else {
            planTime = 0;
        }

        bs.dlvId = dlvId;
        getRegionList(bs.regIds);
        if ( newState == DLVSTATE_ACTIVE && bs.regIds.empty() ) {
            smsc_log_debug(log_,"D=%u has no regions, switch to finish",dlvId);
            newState = DLVSTATE_FINISHED;
        }
        bs.bind = (newState == DLVSTATE_ACTIVE);
        dlvInfo_->getUserInfo().incDlvStats(newState,state_); // may NOT throw
        state_ = newState;
        planTime_ = planTime;
        switch (newState) {
        case DLVSTATE_PLANNED: 
        case DLVSTATE_PAUSED:
        case DLVSTATE_FINISHED:
        case DLVSTATE_CANCELLED: {
            MutexGuard cmg(cacheLock_);
            for ( StoreList::iterator i = storeList_.begin(); i != storeList_.end(); ++i ) {
                (*i)->stopTransfer();
            }
            break;
        }
        case DLVSTATE_ACTIVE:
            // do nothing here, activation is below in delivery activator
            break;
        default:
            throw InfosmeException(EXC_LOGICERROR,"unknown state %d",newState);
        }

        // write status line into status file
        char buf[200];
        sprintf(makeDeliveryPath(buf,dlvId),"status.log");
        FileGuard fg;
        fg.create((getCS()->getStorePath() + buf).c_str(),0666,true);
        fg.seek(0,SEEK_END);
        if (fg.getPos()==0) {
            const char* header = "#1 TIME,STATE,PLANTIME,TOTAL,PROC,SENT,RETRY,DLVD,FAIL,EXPD,SMSDLVD,SMSFAIL,SMSEXPD,KILL\n";
            fg.write(header,strlen(header));
        }
        DeliveryStats ds;
        dlvInfo_->getMsgStats(ds);
        int buflen = sprintf(buf,"%llu,%c,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
                             ymd,
                             dlvStateToString(newState)[0],
                             planTime ? planTime-now : 0,
                             ds.totalMessages,
                             ds.procMessages,
                             ds.sentMessages,
                             ds.retryMessages,
                             ds.dlvdMessages,
                             ds.failedMessages,
                             ds.expiredMessages,
                             ds.dlvdSms,
                             ds.failedSms,
                             ds.expiredSms,
                             ds.killedMessages );
        assert(buflen>0);
        fg.write(buf,buflen);
        smsc_log_debug(log_,"D=%u record written into status.log",dlvId);
        source_->getDlvActivator().finishStateChange(now, ymd, bs, *this );
    }
}


RegionalStoragePtr DeliveryImpl::getRegionalStorage( regionid_type regId, bool create )
{
    MutexGuard mg(cacheLock_);
    StoreList::iterator* ptr = storeHash_.GetPtr(regId);
    if (!ptr) {
        if (!create) {
            return RegionalStoragePtr();
        }
        ptr = createRegionalStorage( regId );
    }
    return **ptr;
}


void DeliveryImpl::getRegionList( std::vector< regionid_type >& regIds ) const
{
    if ( getCS()->isArchive() || getCS()->isEmergency() ) {
        throw InfosmeException(EXC_ACCESSDENIED,"in archive/emergency mode");
    }
    MutexGuard mg(cacheLock_);
    regIds.reserve(storeHash_.Count());
    int regId;
    StoreList::iterator ptr;
    for ( StoreHash::Iterator i(storeHash_); i.Next(regId,ptr); ) {
        regIds.push_back(regionid_type(regId));
    }
}


size_t DeliveryImpl::rollOverStore()
{
    size_t written = 0;
    smsc_log_debug(log_,"D=%u rolling store",dlvInfo_->getDlvId());
    bool firstPass = true;
    do {
        RegionalStoragePtr ptr;
        {
            MutexGuard mg(cacheLock_);
            if ( firstPass ) {
                rollingIter_ = storeList_.begin();
                firstPass = false;
            }
            if ( rollingIter_ == storeList_.end() ) { break; }
            ptr = *rollingIter_;
            ++rollingIter_;
        }
        written += ptr->rollOver();
        if (getCS()->isStopping()) { break; }
    } while ( true );
    smsc_log_debug(log_,"D=%u rolling store done, written=%u",dlvInfo_->getDlvId(),written);
    return written;
}


void DeliveryImpl::setRecordAtInit( const InputRegionRecord& rec,
                                    uint64_t                 maxMsgId )
{
    if (source_) {
        source_->setRecordAtInit(rec,maxMsgId);
    }
}


void DeliveryImpl::setRecordAtInit( regionid_type            regionId,
                                    Message&                 msg,
                                    regionid_type            serial )
{
    StoreList::iterator* ptr = storeHash_.GetPtr(regionId);
    if (!ptr) {
        ptr = createRegionalStorage(regionId);
    }
    (**ptr)->setRecordAtInit(msg,serial);
}


void DeliveryImpl::postInitOperative( std::vector<regionid_type>& filledRegs,
                                      std::vector<regionid_type>& emptyRegs )
{
    int regId;
    StoreList::iterator ptr;
    smsc_log_debug(log_,"D=%u postInitOperative",getDlvId());
    for ( StoreHash::Iterator i(storeHash_); i.Next(regId,ptr); ) {
        if ( (*ptr)->postInit() ) {
            filledRegs.push_back(regionid_type(regId));
        } else {
            emptyRegs.push_back(regionid_type(regId));
            storeList_.erase(ptr);
            storeHash_.Delete(regId);
        }
    }
    DeliveryStats ds;
    dlvInfo_->getMsgStats(ds);
    smsc_log_info(log_,"D=%u stats: total=%u proc=%u sent=%u retry=%u dlvd=%u fail=%u expd=%u kill=%u",
                  dlvInfo_->getDlvId(),
                  ds.totalMessages, ds.procMessages,
                  ds.sentMessages, ds.retryMessages,
                  ds.dlvdMessages, ds.failedMessages,
                  ds.expiredMessages, ds.killedMessages );
}


void DeliveryImpl::detachEverything( bool cleanDirectory,
                                     bool moveToArchive )
{
    const dlvid_type dlvId = dlvInfo_->getDlvId();
    smsc_log_debug(log_,"D=%u detaching everything",dlvId);
    {
        dlvInfo_->getUserInfo().detachDelivery(dlvId);
        MutexGuard mg(cacheLock_);
        storeHash_.Empty();
        storeList_.clear();
        rollingIter_ = storeList_.end();
    }
    if (cleanDirectory) {
        char buf[100];
        makeDeliveryPath(buf,dlvId);
        try {
            FileGuard::rmdirs((getCS()->getStorePath() + buf).c_str());
        } catch ( ErrnoException& e ) {
            smsc_log_warn(log_,"D=%u exc: %s",dlvId,e.what());
        }
    } else if (moveToArchive) {
        char buf[100];
        *(makeDeliveryPath(buf,dlvId)-1) = '\0';
        try {
            ::rename((getCS()->getStorePath()+buf).c_str(),
                     (getCS()->getStorePath()+buf+".out").c_str());
        } catch ( ErrnoException& e ) {
            smsc_log_warn(log_,"D=%u exc: %u",dlvId,e.what());
        }
    }
    smsc_log_debug(log_,"D=%u detached",dlvId);
}


void DeliveryImpl::checkFinalize()
{
    if ( state_ != DLVSTATE_ACTIVE ) return;
    const dlvid_type dlvId = dlvInfo_->getDlvId();
    smsc_log_debug(log_,"D=%u check finalize invoked",dlvId);
    DeliveryStats ds;
    bool finalize = true;
    {
        MutexGuard mg(cacheLock_);
        int regId;
        StoreList::iterator ptr;
        for ( StoreHash::Iterator i(storeHash_); i.Next(regId,ptr); ) {
            if (! (*ptr)->isFinished()) {
                finalize = false;
                smsc_log_debug(log_,"R=%u/D=%u is still active",regId,dlvId);
                break;
            }
        }
        dlvInfo_->getMsgStats(ds);
    }
    if (finalize) {
        if (ds.isFinished()) {
            smsc_log_debug(log_,"D=%u all messages are final, confirmed by stats", dlvId);
            setState(DLVSTATE_FINISHED);
            // source_->getDlvActivator().setDeliveryState(dlvId,DLVSTATE_FINISHED,0);
        } else {
            smsc_log_warn(log_,"D=%u all messages are final, discrep by stats: tot=%u/dlv=%u/fail=%u/exp=%u/kill=%u",
                          dlvId,
                          ds.totalMessages, ds.dlvdMessages,
                          ds.failedMessages, ds.expiredMessages,
                          ds.killedMessages );
        }
    }
}


void DeliveryImpl::cancelOperativeStorage()
{
    dlvid_type dlvId = getDlvId();
    smsc_log_debug(log_,"D=%u cancellation of operative storage started",dlvId);
    std::vector< regionid_type > regIds;
    getRegionList(regIds);
    for ( std::vector<regionid_type>::const_iterator i = regIds.begin();
          i != regIds.end(); ++i ) {
        RegionalStoragePtr ptr = getRegionalStorage(*i,false);
        if (ptr.get()) { ptr->cancelOperativeStorage(); }
        if (getCS()->isStopping()) { break; }
    }
    smsc_log_debug(log_,"D=%u cancellation of operative storage finished",dlvId);
}


void DeliveryImpl::writeDeliveryInfoData()
{
    DeliveryInfoData data;
    dlvInfo_->getDeliveryData(data);
    Config config;
    config.setString("name",data.name.c_str());
    config.setInt("priority",data.priority);
    config.setBool("transactionMode",data.transactionMode);
    if (!data.startDate.empty()) {
        config.setString("startDate",data.startDate.c_str());
    }
    if (!data.endDate.empty()) { config.setString("endDate",data.endDate.c_str()); }
    if (!data.activePeriodStart.empty()) {
        config.setString("activePeriodStart",data.activePeriodStart.c_str());
    }
    if (!data.activePeriodEnd.empty()) {
        config.setString("activePeriodEnd",data.activePeriodEnd.c_str());
    }
    if (!data.activeWeekDays.empty()) {
        std::string awd = data.activeWeekDays.front();
        std::vector< std::string >::const_iterator i = data.activeWeekDays.begin();
        for ( ++i; i != data.activeWeekDays.end(); ++i ) {
            awd += ',';
            awd += *i;
        }
        config.setString("activeWeekDays",awd.c_str());
    }
    if (!data.validityPeriod.empty()) {
        config.setString("validityPeriod",data.validityPeriod.c_str());
    }
    if (!data.archivationPeriod.empty()) {
        config.setString("archivationPeriod",data.archivationPeriod.c_str());
    }
    config.setBool("flash",data.flash);
    config.setBool("useDataSm",data.useDataSm);
    {
        const char* what;
        switch (data.deliveryMode) {
        case DLVMODE_SMS : what = "sms"; break;
        case DLVMODE_USSDPUSH : what = "ussdpush"; break;
        case DLVMODE_USSDPUSHVLR : what = "ussdpushvlr"; break;
        default: throw InfosmeException(EXC_LOGICERROR,"invalid dlvmode: %d",data.deliveryMode); break;
        }
        config.setString("deliveryMode",what);
    }
    config.setString("owner",dlvInfo_->getUserInfo().getUserId());
    config.setBool("retryOnFail",data.retryOnFail);
    if (!data.retryPolicy.empty()) {
        config.setString("retryPolicy",data.retryPolicy.c_str());
    }
    config.setBool("replaceMessage",data.replaceMessage);
    if (!data.svcType.empty()) {
        config.setString("svcType",data.svcType.c_str());
    }
    if (!data.userData.empty()) {
        config.setString("userData",data.userData.c_str());
    }
    if (!data.sourceAddress.empty()) {
        config.setString("sourceAddress",data.sourceAddress.c_str());
    }
    config.setBool("finalDlvRecords",data.finalDlvRecords);
    config.setBool("finalMsgRecords",data.finalMsgRecords);

    char buf[100];
    const dlvid_type dlvId = getDlvId();
    sprintf(makeDeliveryPath(buf,dlvId),"config.xml");

    const std::string fpo = getCS()->getStorePath() + buf;
    const std::string fpn = fpo + ".new";
    try {
        config.saveToFile( fpn.c_str(), "utf-8" );
    } catch ( std::exception& fe ) {
        FileGuard fg;
        fg.create(fpn.c_str(),0666,true,true);
        fg.close();
        config.saveToFile( fpn.c_str(), "utf-8" );
    }
    if ( -1 == rename( fpn.c_str(), fpo.c_str() ) ) {
        throw ErrnoException(errno,"rename('%s')",fpo.c_str());
    }
}


DeliveryImpl::StoreList::iterator* 
    DeliveryImpl::createRegionalStorage( regionid_type regId )
{
    StoreList::iterator iter = 
        storeList_.insert( storeList_.begin(),
                           RegionalStoragePtr( new RegionalStorage(*this,regId)) );
    return &storeHash_.Insert(regId,iter);
}

}
}
