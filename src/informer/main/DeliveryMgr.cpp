#include <cassert>
#include "DeliveryMgr.h"
#include "InfosmeCoreV1.h"
#include "informer/newstore/InputStorage.h"
#include "informer/io/DirListing.h"
#include "informer/data/InputRegionRecord.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/FinalLog.h"
#include "system/status.h"

namespace {

struct NumericNameFilter {
    inline bool operator()( const char* fn ) const {
        char* endptr;
        strtoul(fn,&endptr,10);
        return (*endptr=='\0');
    }
};

const char* archiveName = "deliveries/incoming.txt";
const char* archiveExt = ".new";

}

namespace eyeline {
namespace informer {


class DeliveryMgr::InputJournalReader : public InputJournal::Reader
{
public:
    InputJournalReader( DeliveryMgr& mgr ) :
    mgr_(mgr),
    log_(smsc::logger::Logger::getInstance("ijreader")) {}
    virtual void setRecordAtInit( dlvid_type               dlvId,
                                  const InputRegionRecord& rec,
                                  uint64_t                 maxMsgId )
    {
        smsc_log_debug(log_,"setting input record R=%u/D=%u",rec.regionId,dlvId);
        DeliveryList::iterator* iter = mgr_.deliveryHash_.GetPtr(dlvId);
        if (!iter) {
            smsc_log_info(log_,"delivery D=%u is not found, ok",dlvId);
            return;
        }
        (**iter)->setRecordAtInit(rec,maxMsgId);
    }


    virtual void postInit()
    {
        BindSignal bs;
        bs.bind = true;
        int dlvId;
        DeliveryList::iterator iter;
        smsc_log_debug(log_,"invoking postInit to bind filled regions");
        for ( DeliveryHash::Iterator i(mgr_.deliveryHash_); i.Next(dlvId,iter); ) {
            if (getCS()->isStopping()) { break; }
            bs.regIds.clear();
            (*iter)->postInitInput(bs.regIds);
            if (!bs.regIds.empty()) {
                bs.dlvId = (*iter)->getDlvId();
                // we may not lock here
                mgr_.core_.bindDeliveryRegions(bs);
            }
        }
    }

private:
    DeliveryMgr&          mgr_;
    smsc::logger::Logger* log_;
};


class DeliveryMgr::StoreJournalReader : public StoreJournal::Reader
{
public:
    StoreJournalReader( DeliveryMgr& mgr ) : mgr_(mgr),
    log_(smsc::logger::Logger::getInstance("sjreader")) {}
    virtual void setRecordAtInit( dlvid_type    dlvId,
                                  regionid_type regionId,
                                  Message&      msg,
                                  regionid_type serial )
    {
        if (log_->isDebugEnabled()) {
            uint8_t ton, npi, len;
            const uint64_t addr = subscriberToAddress(msg.subscriber,len,ton,npi);
            smsc_log_debug(log_,"load record R=%u/D=%u/M=%llu state=%s A=.%u.%u.%*.*llu txtId=%d/'%s' serial=%u",
                           regionId, dlvId, ulonglong(msg.msgId),
                           msgStateToString(MsgState(msg.state)),
                           ton,npi,len,len,ulonglong(addr),
                           msg.text.getTextId(),
                           msg.text.getText() ? msg.text.getText() : "",
                           serial);
        }
        DeliveryList::iterator* iter = mgr_.deliveryHash_.GetPtr(dlvId);
        if (!iter) {
            smsc_log_info(log_,"delivery D=%u is not found, ok",dlvId);
            return;
        }
        (**iter)->setRecordAtInit(regionId,msg,serial);
    }


    virtual void postInit()
    {
        BindSignal bsEmpty, bsFilled;
        bsEmpty.bind = false;
        bsFilled.bind = true;
        int dlvId;
        DeliveryList::iterator iter;
        smsc_log_debug(log_,"invoking postInit to bind/unbind regions");
        for ( DeliveryHash::Iterator i(mgr_.deliveryHash_); i.Next(dlvId,iter); ) {
            if (getCS()->isStopping()) { break; }
            bsEmpty.regIds.clear();
            bsFilled.regIds.clear();
            (*iter)->postInitOperative(bsFilled.regIds,bsEmpty.regIds);
            if (!bsEmpty.regIds.empty()) {
                bsEmpty.dlvId = (*iter)->getDlvId();
                // we may not lock here
                mgr_.core_.bindDeliveryRegions(bsEmpty);
            }
            if (!bsFilled.regIds.empty()) {
                bsFilled.dlvId = (*iter)->getDlvId();
                // we may not lock here
                mgr_.core_.bindDeliveryRegions(bsFilled);
            }
        }
    }

private:
    DeliveryMgr& mgr_;
    smsc::logger::Logger* log_;
};


class DeliveryMgr::InputJournalRoller : public smsc::core::threads::Thread
{
public:
    InputJournalRoller( DeliveryMgr& mgr ) :
    mgr_(mgr), log_(smsc::logger::Logger::getInstance("inroller")) {}
    ~InputJournalRoller() { WaitFor(); }
    virtual int Execute()
    {
        smsc_log_debug(log_,"input journal roller started");
        DeliveryList::iterator& iter = mgr_.inputRollingIter_;
        while (! getCS()->isStopping() ) {
            bool firstPass = true;
            size_t written = 0;
            do {
                DeliveryImplPtr ptr;
                {
                    smsc::core::synchronization::MutexGuard mg(mgr_.mon_);
                    if (firstPass) {
                        iter = mgr_.deliveryList_.begin();
                        firstPass = false;
                    }
                    if (iter == mgr_.deliveryList_.end()) { break; }
                    ptr = *iter;
                    ++iter;
                }
                smsc_log_debug(log_,"going to roll D=%u",ptr->getDlvId());
                written += ptr->rollOverInput();
            } while (true);
            smsc_log_debug(log_,"input rolling pass done, written=%llu",ulonglong(written));
            MutexGuard mg(mgr_.mon_);
            if (!getCS()->isStopping()) {
                mgr_.inputJournal_->rollOver(); // change files
                mgr_.mon_.wait(getCS()->getInputJournalRollingPeriod()*1000);
            }
        }
        smsc_log_debug(log_,"input journal roller stopped");
        return 0;
    }
private:
    DeliveryMgr&          mgr_;
    smsc::logger::Logger* log_;
};


class DeliveryMgr::StoreJournalRoller : public smsc::core::threads::Thread
{
public:
    StoreJournalRoller( DeliveryMgr& mgr ) :
    mgr_(mgr), log_(smsc::logger::Logger::getInstance("oproller")) {}
    ~StoreJournalRoller() { WaitFor(); }
    virtual int Execute()
    {
        smsc_log_debug(log_,"store journal roller started");
        DeliveryList::iterator& iter = mgr_.storeRollingIter_;
        while (! getCS()->isStopping()) { // never ending loop
            bool firstPass = true;
            size_t written = 0;
            do {
                DeliveryImplPtr ptr;
                {
                    if ( getCS()->isStopping()) { break; }
                    smsc::core::synchronization::MutexGuard mg(mgr_.mon_);
                    if (firstPass) {
                        iter = mgr_.deliveryList_.begin();
                        firstPass = false;
                    }
                    if (iter == mgr_.deliveryList_.end()) { break; }
                    ptr = *iter;
                    ++iter;
                }
                smsc_log_debug(log_,"going to roll D=%u",ptr->getDlvId());
                written += ptr->rollOverStore();
            } while (true);
            smsc_log_debug(log_,"store rolling pass done, written=%llu",ulonglong(written));
            MutexGuard mg(mgr_.mon_);
            if (! getCS()->isStopping()) {
                mgr_.storeJournal_->rollOver(); // change files
                mgr_.mon_.wait(getCS()->getOpJournalRollingPeriod()*1000);
            }
        }
        smsc_log_debug(log_,"store journal roller stopped");
        return 0;
    }
private:
    DeliveryMgr&          mgr_;
    smsc::logger::Logger* log_;
};


class DeliveryMgr::StatsDumper : public smsc::core::threads::Thread
{
public:
    StatsDumper( DeliveryMgr& mgr ) :
    mgr_(mgr), log_(smsc::logger::Logger::getInstance("statdump")) {}
    ~StatsDumper() { WaitFor(); }


    void init()
    {
        // cleaning the statistics
        mgr_.cs_.flipStatBank();
        mgr_.core_.initUserStats();
        DeliveryStats ds;
        MutexGuard mg(mgr_.mon_);
        for ( DeliveryList::iterator i = mgr_.deliveryList_.begin();
              i != mgr_.deliveryList_.end(); ++i ) {
            (*i)->popMsgStats(ds);
        }
        smsc_log_debug(log_,"stats dumper inited");
    }


    virtual int Execute()
    {
        smsc_log_debug(log_,"stats dumper started");

        const msgtime_type delta = 60; // mgr_.cs_.getStatDumpPeriod();
        assert( ( delta > 3600 && delta % 3600 == 0 ) ||
                ( delta < 3600 && 3600 % delta == 0 && delta % 60 == 0 ) );
        typedef ulonglong msectime_type;
        const msectime_type msecDelta = delta * 1000;
        msectime_type nextTime;
        {
            // get this time
            ulonglong tmpnow = msgTimeToYmd(currentTimeSeconds());
            // strip seconds and minutes, get absolute time
            msgtime_type now = ymdToMsgTime(tmpnow / 10000 * 10000);
            nextTime = msectime_type(now)*1000;
            msectime_type msecNow = msectime_type(currentTimeMicro()/1000);
            while ( nextTime < msecNow ) {
                nextTime += msecDelta;
            }
        }
        while (! getCS()->isStopping()) {
            {
                MutexGuard mg(mgr_.mon_);
                const msectime_type now = msectime_type(currentTimeMicro()/1000);
                if ( now < nextTime ) {
                    mgr_.mon_.wait(int(nextTime-now));
                    continue;
                }
                // core_.cs_.flipStatBank();
            }
            dumpStats( msgtime_type(nextTime / 1000) );
            nextTime += msecDelta;
        }
        smsc_log_debug(log_,"stats dumper finished");
        return 0;
    }


    void dumpStats( msgtime_type currentTime )
    {
        mgr_.cs_.flipStatBank();
        FinalLog::getFinalLog()->checkRollFile( currentTime );
        mgr_.core_.dumpUserStats( currentTime );
        FileGuard fg;
        char buf[200];
        const ulonglong ymd = msgTimeToYmd(currentTime);
        char* bufpos = buf + sprintf(buf,"%04u,",unsigned(ymd % 10000));

        DeliveryList::iterator& iter = mgr_.statsDumpingIter_;
        bool firstPass = true;
        do {
            DeliveryStats   ds;
            dlvid_type      dlvId;
            userid_type     userId;
            {
                smsc::core::synchronization::MutexGuard mg(mgr_.mon_);
                if (firstPass) {
                    iter = mgr_.deliveryList_.begin();
                    firstPass = false;
                }
                if (iter == mgr_.deliveryList_.end()) { break; }
                dlvId = (*iter)->getDlvId();
                userId = (*iter)->getUserInfo().getUserId();
                (*iter)->popMsgStats(ds);
                ++iter;
            }
            if ( ds.isEmpty() ) continue;
            if (!fg.isOpened()) {
                // open file
                char fpath[200];
                const unsigned dayhour = unsigned(ymd/10000);
                sprintf(fpath,"%04u.%02u.%02u/msg%02u.log",
                        dayhour / 1000000,
                        dayhour / 10000 % 100,
                        dayhour / 100 % 100,
                        dayhour % 100);
                fg.create((getCS()->getStatPath()+fpath).c_str(),0666,true);
                fg.seek(0,SEEK_END);
                if (fg.getPos() == 0) {
                    const char* header = "#1 MINSEC,DLVID,USER,NEW,PROC,DLVD,FAIL,EXPD,SMSDLVD,SMSFAIL,SMSEXPD,KILL\n";
                    fg.write(header,strlen(header));
                }
            }
            char* p = bufpos + sprintf(bufpos,"%u,%s,%u,%u,%u,%u,%u,%u,%u,%u,%u\n",
                                       dlvId, userId.c_str(),
                                       ds.totalMessages,
                                       ds.procMessages,
                                       ds.dlvdMessages,
                                       ds.failedMessages,
                                       ds.expiredMessages,
                                       ds.dlvdSms,
                                       ds.failedSms,
                                       ds.expiredSms,
                                       ds.killedMessages );
            fg.write(buf,p-buf);
        } while (true);
    }

private:
    DeliveryMgr&          mgr_;
    smsc::logger::Logger* log_;
};


class DeliveryMgr::CancelTask : public smsc::core::threads::ThreadedTask
{
public:
    CancelTask( dlvid_type dlvId, DeliveryMgr& mgr ) :
    dlvId_(dlvId), mgr_(mgr) {}

    virtual ~CancelTask() {}
    virtual const char* taskName() { return "cancel"; }
    virtual int Execute() {
        DeliveryImplPtr ptr;
        if (!mgr_.getDelivery(dlvId_,ptr) || !ptr ) { return 1; }
        ptr->cancelOperativeStorage();
        return 0;
    }

private:
    dlvid_type   dlvId_;
    DeliveryMgr& mgr_;
};


// ============================================================================

DeliveryMgr::DeliveryMgr( InfosmeCoreV1& core, CommonSettings& cs ) :
log_(smsc::logger::Logger::getInstance("dlvmgr")),
core_(core),
cs_(cs),
inputRollingIter_(deliveryList_.end()),
storeRollingIter_(deliveryList_.end()),
statsDumpingIter_(deliveryList_.end()),
storeJournal_(0),
inputJournal_(0),
inputRoller_(0),
storeRoller_(0),
statsDumper_(0),
logStateTime_(0),
lastDlvId_(0),
trafficSpeed_(cs_.getLicenseLimit())
{
    smsc_log_debug(log_,"ctor");
    if ( getCS()->isArchive() ) { return; }
    storeJournal_ = new StoreJournal;
    inputJournal_ = new InputJournal;
    ctp_.setMaxThreads(10);
}


DeliveryMgr::~DeliveryMgr()
{
    smsc_log_info(log_,"dtor started");
    stop();

    if ( statsDumper_ ) {
        statsDumper_->dumpStats(currentTimeSeconds());
    }

    delete statsDumper_;
    delete storeRoller_;
    delete inputRoller_;
    delete storeJournal_;
    delete inputJournal_;

    smsc_log_info(log_,"--- destroying all deliveries ---");
    deliveryHash_.Empty();
    // detach all deliveries from user infos
    for ( DeliveryList::iterator i = deliveryList_.begin(); i != deliveryList_.end(); ++i ) {
        (*i)->detachEverything();
    }
    deliveryList_.clear();
    smsc_log_info(log_,"--- delivery mgr dtor done ---");
}


void DeliveryMgr::init()
{
    // loading deliveries
    smsc_log_debug(log_,"--- loading deliveries ---");

    smsc::core::buffers::TmpBuf<char,250> buf;
    const std::string& path = cs_.getStorePath();
    buf.setSize(path.size()+100);
    strcpy(buf.get(),path.c_str());
    strcat(buf.get(),"deliveries/");
    buf.SetPos(strlen(buf.get()));
    std::vector<std::string> chunks;
    std::vector<std::string> dlvs;
    smsc_log_debug(log_,"listing deliveries storage '%s'",buf.get());
    try {
        makeDirListing(NumericNameFilter(),S_IFDIR).list(buf.get(), chunks);
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"directory '%s' does not exist, creating",buf.get());
        FileGuard::makedirs(buf.get());
    }
    std::sort( chunks.begin(), chunks.end() );
    for ( std::vector<std::string>::iterator ichunk = chunks.begin();
          ichunk != chunks.end(); ++ichunk ) {
        strcpy(buf.GetCurPtr(),ichunk->c_str());
        strcat(buf.GetCurPtr(),"/");
        dlvs.clear();
        smsc_log_debug(log_,"listing delivery chunk '%s'",buf.get());
        makeDirListing(NumericNameFilter(),S_IFDIR).list(buf.get(), dlvs);
        for ( std::vector<std::string>::iterator idlv = dlvs.begin();
              idlv != dlvs.end();
              ++idlv ) {
            // get dlvid
            const dlvid_type dlvId(dlvid_type(strtoul(idlv->c_str(),0,10)));
            if (dlvId > lastDlvId_) {
                lastDlvId_ = dlvId;
            }
            try {
                readDelivery( dlvId );
            } catch (std::exception& e) {
                smsc_log_error(log_,"D=%u cannot read/add dlvInfo, exc: %s",dlvId,e.what());
            }
        }
    }

    // reading journals and binding deliveries and regions
    if ( ! getCS()->isArchive() ) {

        // signalling archive
        signalArchive(0);

        smsc_log_info(log_,"--- reading journals ---");
        StoreJournalReader sjr(*this);
        storeJournal_->init(sjr);
        InputJournalReader ijr(*this);
        inputJournal_->init(ijr);
        inputRoller_ = new InputJournalRoller(*this);
        storeRoller_ = new StoreJournalRoller(*this);
        statsDumper_ = new StatsDumper(*this);
        statsDumper_->init();
    }
    smsc_log_debug(log_,"--- init done ---");
}


void DeliveryMgr::start()
{
    MutexGuard mg(mon_);
    if (inputRoller_) { inputRoller_->Start(); }
    if (storeRoller_) { storeRoller_->Start(); }
    if (statsDumper_) { statsDumper_->Start(); }
    Start();
}


void DeliveryMgr::stop()
{
    {
        MutexGuard mg(mon_);
        smsc_log_info(log_,"stop() received");
        mon_.notifyAll();
    }
    ctp_.stopNotify();
    {
        MutexGuard mg(trafficMon_);
        trafficMon_.notifyAll();
    }
    if (inputRoller_) { inputRoller_->WaitFor(); }
    if (storeRoller_) { storeRoller_->WaitFor(); }
    if (statsDumper_) { statsDumper_->WaitFor(); }

    WaitFor();
    ctp_.shutdown(0);
    smsc_log_debug(log_,"leaving stop()");
}


void DeliveryMgr::receiveReceipt( const DlvRegMsgId& drmId,
                                  const RetryPolicy& policy,
                                  int      status,
                                  bool     retry,
                                  unsigned nchunks )
{
    smsc_log_debug(log_,"rcpt received R=%u/D=%u/M=%llu status=%u retry=%d nchunks=%u",
                   drmId.regId, drmId.dlvId,
                   drmId.msgId, status, retry, nchunks );
    try {
        DeliveryImplPtr dlv;
        if ( !getDelivery(drmId.dlvId,dlv) ) {
            smsc_log_warn(log_,"R=%u/D=%u/M=%llu rcpt: delivery not found",
                          drmId.regId, drmId.dlvId, drmId.msgId );
            return;
        }

        // const DeliveryInfo& info = dlv->getDlvInfo();

        RegionalStoragePtr reg = dlv->getRegionalStorage(drmId.regId);
        if (!reg.get()) {
            smsc_log_warn(log_,"R=%u/D=%u/M=%llu rcpt: region is not found",
                          drmId.regId, drmId.dlvId, drmId.msgId );
            return;
        }

        const msgtime_type now(currentTimeSeconds());

        const bool ok = (status == smsc::system::Status::OK);
        if (!ok && retry) {
            // attempt to retry
            reg->retryMessage( drmId.msgId, policy, now, status, nchunks);
        } else {
            reg->finalizeMessage(drmId.msgId, now,
                                 ok ? MSGSTATE_DELIVERED : MSGSTATE_FAILED,
                                 status, nchunks );
        }
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"R=%u/D=%u/M=%llu rcpt process failed, exc: %s",
                      drmId.regId,
                      drmId.dlvId,
                      drmId.msgId, e.what() );
    }
}


bool DeliveryMgr::receiveResponse( const DlvRegMsgId& drmId )
{
    smsc_log_debug(log_,"good resp received R=%u/D=%u/M=%llu",
                   drmId.regId,
                   drmId.dlvId,
                   drmId.msgId);
    try {
        DeliveryImplPtr dlv;
        if ( !getDelivery(drmId.dlvId,dlv) ) {
            smsc_log_warn(log_,"R=%u/D=%u/M=%llu resp: delivery not found",
                          drmId.regId,
                          drmId.dlvId,
                          drmId.msgId );
            return false;
        }

        RegionalStoragePtr reg = dlv->getRegionalStorage(drmId.regId);
        if (!reg.get()) {
            smsc_log_warn(log_,"R=%u/D=%u/M=%llu resp: region is not found",
                          drmId.regId,
                          drmId.dlvId,
                          drmId.msgId );
            return false;
        }

        const msgtime_type now(currentTimeSeconds());

        reg->messageSent(drmId.msgId,now);
        return true;

    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"R=%u/D=%u/M=%llu resp process failed, exc: %s",
                      drmId.regId,
                      drmId.dlvId,
                      drmId.msgId, e.what() );
    }
    return false;
}


void DeliveryMgr::incIncoming()
{
    // NOTE: we do nothing here right now as incOutgoing
    // need not to be waked
}


void DeliveryMgr::incOutgoing( unsigned nchunks )
{
    // NOTE: this code is taked from infosme v2
    smsc_log_debug(log_,"incOutgoing(%u)",nchunks);
    MutexGuard mg(trafficMon_);
    trafficSpeed_.consumeQuant();
    do {
        const usectime_type currentTime = currentTimeMicro() % flipTimePeriod;
        const usectime_type delay = trafficSpeed_.isReady(currentTime,maxSnailDelay);
        if ( delay == 0 ) { break; }
        if (getCS()->isStopping()) { break; }
        int waitTime = int(delay / 1000) + 1;
        smsc_log_debug(log_,"waiting %lluusec/%umsec on license",
                       ulonglong(delay),waitTime);
        trafficMon_.wait(waitTime);
    } while (true);
}


dlvid_type DeliveryMgr::createDelivery( UserInfo& userInfo,
                                        const DeliveryInfoData& infoData )
{
    const dlvid_type dlvId = getNextDlvId();
    DeliveryInfo* info = new DeliveryInfo(dlvId,infoData,userInfo);
    DlvState state = DLVSTATE_PAUSED;
    msgtime_type planTime = 0;
    if (info->getStartDate()) {
        // msgtime_type now = currentTimeSeconds();
        // if (info->getStartDate() > now + 5) {
        state = DLVSTATE_PLANNED;
        planTime = info->getStartDate();
        // }
    }
    addDelivery(info,state,planTime,true);
    userInfo.incDlvStats(DLVSTATE_CREATED);
    return dlvId;
}


void DeliveryMgr::deleteDelivery( dlvid_type dlvId,
                                  std::vector<regionid_type>& regIds,
                                  bool moveToArchive )
{
    DeliveryList tokill;
    DeliveryList::iterator iter;
    {
        MutexGuard mg(mon_);
        if (!deliveryHash_.Pop(dlvId,iter) ) {
            throw InfosmeException(EXC_NOTFOUND,"delivery %u is not found",dlvId);
        }
        if (inputRollingIter_ == iter) ++inputRollingIter_;
        if (storeRollingIter_ == iter) ++storeRollingIter_;
        if (statsDumpingIter_ == iter) ++statsDumpingIter_;
        tokill.splice(tokill.begin(),deliveryList_,iter);
    }
    if (!moveToArchive) {
        (*iter)->setState(DLVSTATE_CANCELLED);
    }
    (*iter)->getRegionList(regIds);
    (*iter)->detachEverything(!moveToArchive,moveToArchive);
    if (moveToArchive) {
        signalArchive(dlvId);
    }
}


int DeliveryMgr::Execute()
{
    typedef int64_t msectime_type;

    DeliveryWakeQueue wakeList;
    // DeliveryWakeQueue stopList;
    while ( !getCS()->isStopping() ) {

        const msectime_type curTime = msectime_type(currentTimeMicro() / 1000);
        const msgtime_type now(msgtime_type(curTime/1000));

        // processing wakelist
        if ( ! wakeList.empty() ) {
            for ( DeliveryWakeQueue::const_iterator i = wakeList.begin();
                  i != wakeList.end(); ++i ) {
                DeliveryImplPtr dlv;
                if (!getDelivery(i->second,dlv)) {continue;}
                msgtime_type planTime;
                if ( DLVSTATE_PLANNED == dlv->getState(&planTime) &&
                     planTime == i->first ) {
                    dlv->setState( DLVSTATE_ACTIVE );
                }
            }
            wakeList.clear();
        }

        // read archive signals
        try {
            readFromArchive();
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"reading from archive, exc: %s", e.what());
        }

        MutexGuard mg(mon_);
        DeliveryWakeQueue::iterator uptoNow = deliveryWakeQueue_.upper_bound(now);
        if ( uptoNow != deliveryWakeQueue_.begin() ) {
            wakeList.insert(deliveryWakeQueue_.begin(),uptoNow);
            deliveryWakeQueue_.erase(deliveryWakeQueue_.begin(), uptoNow);
        }
        if (!wakeList.empty()) continue;

        msectime_type wakeTime = 10000;
        if (!deliveryWakeQueue_.empty()) {
            msectime_type thisWakeTime = 
                msectime_type(deliveryWakeQueue_.begin()->first)*1000 - curTime;
            if (wakeTime > thisWakeTime) {wakeTime = thisWakeTime;}
        }
        if (wakeTime>0) { mon_.wait(int(wakeTime)); }
    }
    return 0;
}


bool DeliveryMgr::finishStateChange( msgtime_type    currentTime,
                                     ulonglong       ymdTime,
                                     const Delivery& dlv )
{
    const dlvid_type dlvId = dlv.getDlvId();
    msgtime_type planTime;
    const DlvState newState = dlv.getState(&planTime);

    // prepare the buffer to write state change
    char buf[100];
    const int buflen = sprintf(buf,"%04u,%c,%u,%s,%u\n",
                               unsigned(ymdTime % 10000), dlvStateToString(newState)[0],
                               dlvId, dlv.getUserInfo().getUserId(),
                               newState == DLVSTATE_PLANNED ? planTime : 0);
    if ( buflen < 0 ) {
        throw InfosmeException(EXC_SYSTEM,"cannot write dlv state change, dlvId=%u",dlvId);
    }

    const ulonglong fileTime = ymdTime / 10000 * 10000;
    char fnbuf[50];
    if ( logStateTime_ < fileTime ) {
        const unsigned day( unsigned(fileTime / 1000000));
        sprintf(fnbuf,"status_log/%04u.%02u.%02u/%02u.log",
                day / 10000, (day / 100) % 100, day % 100,
                unsigned((ymdTime/10000) % 100) );
    }

    {
        MutexGuard mg(logStateLock_);
        bool newFile = false;
        if ( logStateTime_ < fileTime ) {
            // need to replace cur file
            FileGuard fg;
            fg.create( (cs_.getStorePath() + fnbuf).c_str(), 0666, true );
            fg.seek(0,SEEK_END);
            if (fg.getPos() == 0) {
                const char* header = "#1 MINSEC,STATE,DLVID,USER,PLAN\n";
                fg.write( header, strlen(header));
                newFile = true;
            }
            logStateTime_ = fileTime;
            logStateFile_.swap(fg);
        } else if ( logStateTime_ > fileTime ) {
            // fix delayed record
            memcpy(buf,"0000",4);
        }
        if (newFile) {
            char dbuf[100];
            memcpy(dbuf,buf,4);
            MutexGuard dmg(mon_);
            for ( DeliveryList::iterator i = deliveryList_.begin(); i != deliveryList_.end(); ++i ) {
                const DeliveryImpl* ptr = i->get();
                if ( ptr && ptr->getState() == DLVSTATE_ACTIVE ) {
                    const int dbuflen = sprintf(dbuf+4,",a,%u,%s,0\n",
                                                ptr->getDlvId(),
                                                ptr->getUserInfo().getUserId());
                    logStateFile_.write(dbuf,size_t(dbuflen+4));
                }
            }
        }
        logStateFile_.write(buf,size_t(buflen));
    }

    // put into plan queue
    if (newState == DLVSTATE_PLANNED) {
        MutexGuard mg(mon_);
        deliveryWakeQueue_.insert(std::make_pair(planTime,dlvId));
        mon_.notify();
    } else if (newState == DLVSTATE_CANCELLED) {
        startCancelThread(dlvId);
    }
    // return true if we need to activate delivery regions
    return newState == DLVSTATE_ACTIVE;
}


void DeliveryMgr::addDelivery( DeliveryInfo* info,
                               DlvState      state,
                               msgtime_type  planTime,
                               bool          checkDlvLimit )
{
    UserInfo& userInfo = info->getUserInfo();
    std::auto_ptr< DeliveryInfo > infoptr(info);
    if (!info) {
        throw InfosmeException(EXC_LOGICERROR,"delivery info is NULL");
    }
    const dlvid_type dlvId = info->getDlvId();
    DeliveryImplPtr dlv;
    if ( getDelivery(dlvId,dlv) ) {
        throw InfosmeException(EXC_ALREADYEXIST,"D=%u already exists",dlvId);
    }
    try {
        userInfo.incDlvStats(state,0,checkDlvLimit);
    } catch (std::exception& e) {
        // cannot create delivery!
        throw InfosmeException(EXC_DLVLIMITEXCEED,"U='%s' cannot create delivery, exc: %s",
                               userInfo.getUserId(),e.what());
    }
    InputMessageSource* ims;
    if (!getCS()->isArchive()) {
        ims = new InputStorage(core_,*inputJournal_);
    }
    dlv.reset( new DeliveryImpl(infoptr.release(),
                                storeJournal_,
                                ims,
                                state,
                                planTime ));
    userInfo.attachDelivery( dlv );
    MutexGuard mg(mon_);
    deliveryHash_.Insert(dlvId, deliveryList_.insert(deliveryList_.begin(), dlv));
    if (state == DLVSTATE_PLANNED && planTime && !getCS()->isArchive() ) {
        deliveryWakeQueue_.insert(std::make_pair(planTime,dlv->getDlvId()));
    }
    mon_.notify();
}


void DeliveryMgr::startCancelThread( dlvid_type dlvId )
{
    smsc_log_info(log_,"D=%u start cancellation task",dlvId);
    ctp_.startTask( new CancelTask(dlvId,*this) );
}


dlvid_type DeliveryMgr::getNextDlvId()
{
    MutexGuard mg(mon_);
    for ( int i = 0; i < 1000; ++i ) {
        while ( !++lastDlvId_ ) {}
        DeliveryList::iterator* iter = deliveryHash_.GetPtr(lastDlvId_);
        if (!iter) return lastDlvId_;
    }
    throw InfosmeException(EXC_SYSTEM,"no more free delivery ids, try again");
}


void DeliveryMgr::signalArchive( dlvid_type dlvId )
{
    // signal the archive instance
    const std::string fname = getCS()->getArchivePath() + ::archiveName;
    MutexGuard mg(archiveLock_);
    bool fileExist = false;
    if (dlvId) {
        FileGuard fg;
        fg.create((fname+::archiveExt).c_str(),0666,true);
        fg.seek(0,SEEK_END);
        char buf[30];
        const int buflen = sprintf(buf,"%u\n",dlvId);
        assert(buflen>0);
        fg.write(buf,size_t(buflen));
        fileExist = true;
        fg.close();
    }
    struct stat st;
    if (!fileExist) {
        if ( -1 != ::stat((fname+archiveExt).c_str(),&st) &&
             S_ISREG(st.st_mode) ) {
            fileExist = true;
        }
    }
    if ( fileExist ) {
        if ( -1 == ::stat(fname.c_str(),&st) ) {
            // destination is not found, renaming
            if ( -1 == ::rename((fname+archiveExt).c_str(),fname.c_str()) ) {
                throw ErrnoException(errno,"rename('%s')",fname.c_str());
            }
        }
    }
}


void DeliveryMgr::readFromArchive()
{
    const std::string fname = getCS()->getStorePath() + archiveName;
    FileGuard fg;
    struct stat st;
    if ( -1 == ::stat(fname.c_str(),&st) ) {
        return;
    }
    fg.ropen(fname.c_str());
    std::vector<dlvid_type> dlvs;
    {
        char buf[256];
        char* ptr = buf;
        while (true) {
            const size_t wasread = fg.read(ptr,sizeof(buf)-(ptr-buf));
            if ( 0 == wasread ) {
                if ( ptr != buf ) {
                    throw InfosmeException(EXC_BADFILE,"file '%s' is corrupted",
                                           archiveName);
                }
                break;
            }
            ptr += wasread;
            char* p = buf;
            while ( p < ptr ) {
                char* eol = static_cast<char*>(memchr(p,'\n',ptr-p));
                if (!eol) {break;}
                if ( (eol - p) > 30 ) {
                    throw InfosmeException(EXC_BADFILE,"too big record '%*s'",eol-p,p);
                }
                *eol = '\0';
                unsigned dlvId;
                int shift = 0;
                sscanf(p,"%u%n",&dlvId,&shift);
                if ( shift == 0 || p[shift] != '\0' ) {
                    throw InfosmeException(EXC_BADFILE,"bad record '%s'",p);
                }
                dlvs.push_back(dlvId);
                p = eol + 1;
            }
            if ( p < ptr ) {
                memcpy(buf,p,ptr-p);
            }
            ptr = buf + (ptr-p);
        }
        fg.close();
        // delete the incoming file
        FileGuard::unlink(fname.c_str());
    }

    // processing dlvs
    for ( std::vector<dlvid_type>::const_iterator i = dlvs.begin();
          i != dlvs.end(); ++i ) {
        // first of all checks if we have a delivery already
        DeliveryImplPtr ptr;
        if ( ! getDelivery(*i,ptr) ) {
            try {
                char dname[100];
                *(makeDeliveryPath(dname,*i)-1) = '\0';
                FileGuard::copydir( (getCS()->getArchivePath() + dname + ".out/").c_str(),
                                    getCS()->getStorePath() + dname + ".in/",
                                    10 );
                if ( -1 == ::rename( (getCS()->getStorePath() + dname + ".in").c_str(),
                                     (getCS()->getStorePath() + dname).c_str()) ) {
                    throw ErrnoException(errno,"rename('%s')",dname);
                }
                FileGuard::rmdirs( (getCS()->getArchivePath() + dname + ".out/").c_str() );

                readDelivery(*i);
            } catch ( std::exception& e ) {
                smsc_log_warn(log_,"D=%u cannot attach, exc: %s", *i, e.what());
            }
        } else {
            smsc_log_warn(log_,"D=%u is already attached",*i);
        }
    }
}


void DeliveryMgr::readDelivery( dlvid_type dlvId )
{
    DeliveryInfoData data;
    DeliveryImpl::readDeliveryInfoData(dlvId, data);

    UserInfoPtr user(core_.getUserInfo(data.owner.c_str()));
    if (!user.get()) {
        throw InfosmeException(EXC_CONFIG,"D=%u has unknown user: '%s'",
                               dlvId,data.owner.c_str());
    }

    // read state
    msgtime_type planTime = 0;
    DlvState state = DeliveryImpl::readState(dlvId, planTime );
    if ( getCS()->isArchive() ) {
        switch (state) {
        case DLVSTATE_PLANNED:
        case DLVSTATE_ACTIVE: state = DLVSTATE_FINISHED; break;
        default: break;
        }
        planTime = 0;
    }

    DeliveryInfo* info = new DeliveryInfo(dlvId, data, *user.get());
    addDelivery(info, state, planTime, false);
    if ( state == DLVSTATE_CANCELLED && !getCS()->isArchive() ) {
        startCancelThread(dlvId);
    }
}

}
}
