#include <cassert>
#include "DeliveryMgr.h"
#include "InfosmeCoreV1.h"
#include "informer/newstore/InputStorage.h"
#include "informer/io/DirListing.h"
#include "system/status.h"

namespace {

struct NumericNameFilter {
    inline bool operator()( const char* fn ) const {
        char* endptr;
        strtoul(fn,&endptr,10);
        return (*endptr=='\0');
    }
};

}

namespace eyeline {
namespace informer {


class DeliveryMgr::InputJournalReader : public InputJournal::Reader
{
public:
    InputJournalReader( DeliveryMgr& mgr ) :
    mgr_(mgr),
    log_(smsc::logger::Logger::getInstance("ijreader")) {}
    virtual bool isStopping() const { return mgr_.isCoreStopping(); }
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
            if (mgr_.isCoreStopping()) break;
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
    virtual bool isStopping() const { return mgr_.isCoreStopping(); }
    virtual void setRecordAtInit( dlvid_type    dlvId,
                                  regionid_type regionId,
                                  Message&      msg,
                                  regionid_type serial )
    {
        smsc_log_debug(log_,"load store record R=%u/D=%u/M=%llu state=%s serial=%u",
                       regionId, dlvId,
                       ulonglong(msg.msgId),
                       msgStateToString(MsgState(msg.state)), serial);
        DeliveryList::iterator* iter = mgr_.deliveryHash_.GetPtr(dlvId);
        if (!iter) {
            smsc_log_info(log_,"delivery D=%u is not found, ok",dlvId);
            return;
        }
        (**iter)->setRecordAtInit(regionId,msg,serial);
    }


    virtual void setNextResendAtInit( dlvid_type    dlvId,
                                      regionid_type regId,
                                      msgtime_type  nextResend )
    {
        smsc_log_debug(log_,"load next resend record R=%u/D=%u resend=%llu",
                       regId, dlvId, msgTimeToYmd(nextResend));
        DeliveryList::iterator* iter = mgr_.deliveryHash_.GetPtr(dlvId);
        if (!iter) {
            smsc_log_info(log_,"delivery D=%u is not found, ok",dlvId);
            return;
        }
        (**iter)->setNextResendAtInit(regId,nextResend);
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
            if (mgr_.isCoreStopping()) break;
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
        while (! mgr_.isCoreStopping()) { // never ending loop
            bool firstPass = true;
            size_t written = 0;
            do {
                DeliveryImplPtr ptr;
                {
                    if (mgr_.isCoreStopping()) { break; }
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
            if (!mgr_.isCoreStopping()) {
                mgr_.inputJournal_->rollOver(); // change files
                mgr_.mon_.wait(10000);
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
        while (! mgr_.isCoreStopping()) { // never ending loop
            bool firstPass = true;
            size_t written = 0;
            do {
                DeliveryImplPtr ptr;
                {
                    if (mgr_.isCoreStopping()) { break; }
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
            if (!mgr_.isCoreStopping()) {
                mgr_.storeJournal_->rollOver(); // change files
                mgr_.mon_.wait(10000);
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

    virtual int Execute()
    {
        smsc_log_debug(log_,"stats dumper started");

        const msgtime_type delta = mgr_.cs_.getStatDumpPeriod();
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
        while (!mgr_.isCoreStopping()) {
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
        mgr_.core_.dumpUserStats( currentTime );
        FileGuard fg;
        char buf[200];
        char* bufpos;
        {
            struct tm now;
            const ulonglong ymd = msgTimeToYmd(currentTime,&now);
            sprintf(buf,"%04u.%02u.%02u/msg%02u.log",
                    now.tm_year+1900, now.tm_mon+1,
                    now.tm_mday, now.tm_hour);
            fg.create((mgr_.cs_.getStatPath()+buf).c_str(),0666,true);
            fg.seek(0,SEEK_END);
            if (fg.getPos() == 0) {
                const char* header = "#1 MINSEC,DLVID,USER,NEW,PROC,DLVD,FAIL,EXPD,SMSDLVD,SMSFAIL,SMSEXPD\n";
                fg.write(header,strlen(header));
            }
            bufpos = buf + sprintf(buf,"%04u,",unsigned(ymd % 10000));
        }

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
                (*iter)->popIncrementalStats(ds);
                ++iter;
            }
            if ( ds.isEmpty() ) continue;
            char* p = bufpos + sprintf(bufpos,"%u,%s,%u,%u,%u,%u,%u,%u,%u,%u\n",
                                       dlvId, userId.c_str(),
                                       ds.totalMessages,
                                       ds.procMessages,
                                       ds.dlvdMessages,
                                       ds.failedMessages,
                                       ds.expiredMessages,
                                       ds.dlvdSms,
                                       ds.failedSms,
                                       ds.expiredSms );
            fg.write(buf,p-buf);
        } while (true);
    }

private:
    DeliveryMgr&          mgr_;
    smsc::logger::Logger* log_;
};

// ============================================================================

DeliveryMgr::DeliveryMgr( InfosmeCoreV1& core, CommonSettings& cs ) :
log_(smsc::logger::Logger::getInstance("dlvmgr")),
core_(core),
cs_(cs),
stopping_(true),
inputRollingIter_(deliveryList_.end()),
storeRollingIter_(deliveryList_.end()),
statsDumpingIter_(deliveryList_.end()),
storeJournal_( new StoreJournal(cs_)),
inputJournal_( new InputJournal(cs_)),
inputRoller_(0),
storeRoller_(0),
statsDumper_(0),
logStateTime_(0),
nextDlvId_(1)
{
    smsc_log_debug(log_,"ctor");
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
        (*i)->detachFromUserInfo();
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
        smsc_log_warn(log_,"directory '%s' does not exist",buf.get());
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
//        const size_t buflen = strlen(buf.get());
        for ( std::vector<std::string>::iterator idlv = dlvs.begin();
              idlv != dlvs.end();
              ++idlv ) {
            // get dlvid
            const dlvid_type dlvId(dlvid_type(strtoul(idlv->c_str(),0,10)));
            try {

                DeliveryInfoData data;
                DeliveryImpl::readDeliveryInfoData( cs_, dlvId, data );

                UserInfoPtr user(core_.getUserInfo(data.owner.c_str()));
                if (!user.get()) {
                    throw InfosmeException(EXC_CONFIG,"D=%u has unknown user: '%s'",
                                           dlvId,data.owner.c_str());
                }

                // read state
                msgtime_type planTime = 0;
                const DlvState state = DeliveryImpl::readState( cs_,
                                                                dlvId,
                                                                planTime );

                DeliveryInfo* info = new DeliveryInfo(cs_,
                                                      dlvId,
                                                      data );
                addDelivery(*user.get(), info, state, planTime );

            } catch (std::exception& e) {
                smsc_log_error(log_,"cannot read/add dlvInfo D=%u: %s",dlvId,e.what());
                continue;
            }
        }
    }

    // reading journals and binding deliveries and regions
    smsc_log_debug(log_,"--- reading journals ---");
    StoreJournalReader sjr(*this);
    storeJournal_->init(sjr);
    InputJournalReader ijr(*this);
    inputJournal_->init(ijr);
    inputRoller_ = new InputJournalRoller(*this);
    storeRoller_ = new StoreJournalRoller(*this);
    statsDumper_ = new StatsDumper(*this);
    smsc_log_debug(log_,"--- init done ---");
}


void DeliveryMgr::start()
{
    if (!stopping_) return;
    MutexGuard mg(mon_);
    if (!stopping_) return;
    stopping_ = false;
    inputRoller_->Start();
    storeRoller_->Start();
    statsDumper_->Start();
    Start();
}


void DeliveryMgr::stop()
{
    if (stopping_) return;
    {
        MutexGuard mg(mon_);
        if (stopping_) return;
        smsc_log_info(log_,"stop() received");
        stopping_ = true;
        mon_.notifyAll();
    }
    if (inputRoller_) inputRoller_->WaitFor();
    if (storeRoller_) storeRoller_->WaitFor();
    if (statsDumper_) statsDumper_->WaitFor();

    WaitFor();
    smsc_log_debug(log_,"leaving stop()");
}


bool DeliveryMgr::isCoreStopping() const
{
    return core_.isStopping();
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

        if (retry) {
            // attempt to retry
            reg->retryMessage( drmId.msgId, policy, now, status, nchunks);
        } else {
            const bool ok = (status == smsc::system::Status::OK);
            reg->finalizeMessage(drmId.msgId, now,
                                 ok ? MSGSTATE_DELIVERED : MSGSTATE_FAILED,
                                 status, nchunks );
        }
    } catch ( std::exception& e ) {
        smsc_log_warn(log_,"R=%u/D=%u/M=%llu rcpt processing failed: %s",
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
        smsc_log_warn(log_,"R=%u/D=%u/M=%llu resp processing failed: %s",
                      drmId.regId,
                      drmId.dlvId,
                      drmId.msgId, e.what() );
    }
    return false;
}


void DeliveryMgr::incIncoming()
{
    smsc_log_error(log_,"FIXME: incIncoming() not impl");
}


void DeliveryMgr::incOutgoing( unsigned nchunks )
{
    smsc_log_error(log_,"FIXME: incOutgoing(%u) not impl",nchunks);
}


dlvid_type DeliveryMgr::createDelivery( UserInfo& userInfo,
                                        const DeliveryInfoData& infoData )
{
    const dlvid_type dlvId = getNextDlvId();
    addDelivery(userInfo,new DeliveryInfo(cs_,dlvId,infoData));
    return dlvId;
}


void DeliveryMgr::deleteDelivery( dlvid_type dlvId, std::vector<regionid_type>& regIds )
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
    (*iter)->getRegionList(regIds);
    (*iter)->detachFromUserInfo();
}


int DeliveryMgr::Execute()
{
    typedef int64_t msectime_type;

    DeliveryWakeQueue wakeList;
    DeliveryWakeQueue stopList;
    while ( !stopping_ ) {

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
        if ( !stopList.empty() ) {
            struct ::tm tmnow;
            const time_t tmp(now);
            if ( !gmtime_r(&tmp,&tmnow) ) {
                throw InfosmeException(EXC_SYSTEM,"gmtime_r");
            }
            for ( DeliveryWakeQueue::const_iterator i = stopList.begin();
                  i != stopList.end(); ++i ) {
                DeliveryImplPtr dlv;
                if ( !getDelivery(i->second,dlv) ||
                     (DLVSTATE_ACTIVE != dlv->getState()) ) {
                    continue;
                }
                const timediff_type planTime = 
                    dlv->getDlvInfo().nextActiveTime(tmnow.tm_wday,now);
                if ( planTime <= 0 ) {
                    // should not stop
                    continue;
                }
                dlv->setState(DLVSTATE_PLANNED,now+planTime);
            }
            stopList.clear();
        }


        MutexGuard mg(mon_);
        DeliveryWakeQueue::iterator uptoNow = deliveryWakeQueue_.upper_bound(now);
        if ( uptoNow != deliveryWakeQueue_.begin() ) {
            wakeList.insert(deliveryWakeQueue_.begin(),uptoNow);
            deliveryWakeQueue_.erase(deliveryWakeQueue_.begin(), uptoNow);
        }
        uptoNow = deliveryStopQueue_.upper_bound(now);
        if ( uptoNow != deliveryStopQueue_.begin() ) {
            stopList.insert(deliveryStopQueue_.begin(),uptoNow);
            deliveryStopQueue_.erase(deliveryStopQueue_.begin(), uptoNow);
        }
        if (!wakeList.empty() || !stopList.empty()) continue;

        msectime_type wakeTime = 10000;
        if (!deliveryWakeQueue_.empty()) {
            msectime_type thisWakeTime = 
                msectime_type(deliveryWakeQueue_.begin()->first)*1000 - curTime;
            if (wakeTime > thisWakeTime) {wakeTime = thisWakeTime;}
        }
        if (!deliveryStopQueue_.empty()) {
            msectime_type thisWakeTime =
                msectime_type(deliveryStopQueue_.begin()->first)*1000 - curTime;
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
        if ( logStateTime_ < fileTime ) {
            // need to replace cur file
            FileGuard fg;
            fg.create( (cs_.getStorePath() + fnbuf).c_str(), 0666, true );
            fg.seek(0,SEEK_END);
            if (fg.getPos() == 0) {
                const char* header = "#1 MINSEC,STATE,DLVID,USER,PLAN\n";
                fg.write( header, strlen(header));
            }
            logStateTime_ = fileTime;
            logStateFile_.swap(fg);
        } else if ( logStateTime_ > fileTime ) {
            // fix delayed record
            memcpy(buf,"0000",4);
        }
        logStateFile_.write(buf,size_t(buflen));
    }

    // put into plan queue
    if (newState == DLVSTATE_PLANNED) {
        MutexGuard mg(mon_);
        deliveryWakeQueue_.insert(std::make_pair(planTime,dlvId));
        mon_.notify();
    }
    if (newState == DLVSTATE_ACTIVE && planTime != 0) {
        MutexGuard mg(mon_);
        deliveryStopQueue_.insert(std::make_pair(planTime,dlvId));
        mon_.notify();
    }
    // return true if we need to activate delivery regions
    return newState == DLVSTATE_ACTIVE;
}


void DeliveryMgr::addDelivery( UserInfo&     userInfo,
                               DeliveryInfo* info,
                               DlvState      state,
                               msgtime_type  planTime )
{
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
        userInfo.incStats(cs_,state);
    } catch (std::exception& e) {
        // FIXME: move to paused?
        throw InfosmeException(EXC_DLVLIMITEXCEED,"D=%u cannot set state: %s",dlvId,e.what());
    }
    InputMessageSource* ims = new InputStorage(core_,*inputJournal_);
    dlv.reset( new DeliveryImpl(infoptr.release(),
                                userInfo,
                                *storeJournal_,
                                ims,
                                state,
                                planTime ));
    userInfo.attachDelivery( dlv );
    MutexGuard mg(mon_);
    deliveryHash_.Insert(dlvId, deliveryList_.insert(deliveryList_.begin(), dlv));
    if (state == DLVSTATE_PLANNED && planTime) {
        deliveryWakeQueue_.insert(std::make_pair(planTime,dlv->getDlvId()));
    }
    if (state == DLVSTATE_ACTIVE && planTime) {
        deliveryStopQueue_.insert(std::make_pair(planTime,dlv->getDlvId()));
    }
    mon_.notify();
}


dlvid_type DeliveryMgr::getNextDlvId()
{
    MutexGuard mg(mon_);
    for ( int i = 0; i < 1000; ++i ) {
        while ( !++nextDlvId_ ) {}
        DeliveryList::iterator* iter = deliveryHash_.GetPtr(nextDlvId_);
        if (!iter) return nextDlvId_;
    }
    throw InfosmeException(EXC_SYSTEM,"no more free delivery ids, try again");
}

}
}
