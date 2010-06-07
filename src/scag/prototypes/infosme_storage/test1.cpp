#include <cstdio>
#include <memory>
#include <cassert>

#include "MessageCache.h"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/ThreadPool.hpp"
#include "util/TimeSource.h"
#include "TaskInfo.h"
#include "StoreLog.h"
#include "logger/Logger.h"
#include "infosme/v2/ScoredList.h"
#include "infosme/v2/SpeedControl.h"

using namespace scag2::prototypes::infosme;
using smsc::infosme::ScoredList;
using smsc::infosme::SpeedControl;

const unsigned REGIONS = 10;

class NewMessageStorage : public MessageSource
{
private:
    class GenTask : public MessageUploadTask
    {
    public:
        GenTask( NewMessageStorage&      storage,
                 MessageUploadRequester& req,
                 dlvid_type             dlvId,
                 regionid_type           regionId,
                 unsigned                count ) :
        MessageUploadTask(req),
        storage_(&storage),
        dlvId_(dlvId), regionId_(regionId), count_(count)
        {
            if (!log_) {
                log_ = smsc::logger::Logger::getInstance("updlv");
            }
        }

        virtual int Execute()
        {
            smsc_log_debug(log_,"upload dlv %u/%u started to add %u messages",dlvId_,regionId_,count_);
            MessageList ml;
            for ( unsigned i = 0; i < count_; ++i ) {
                MessageData md;
                Message& msg = md.msg;
                msg.subscriber = 9137654079ULL;
                msg.msgId = storage_->getNextMsgId();
                msg.textId = 1;
                msg.lastTime = 1; // the file number for this task
                msg.timeLeft = msg.msgId * 100;
                char buf[20];
                sprintf(buf,"%u",msg.msgId);
                msg.userData = buf;
                msg.state = MsgState::input;
                smsc_log_debug(log_,"message %u/%u/%u prepared",dlvId_,regionId_,msg.msgId);
                ml.push_back( md );
            }
            smsc_log_debug(log_,"adding new messages to storage...");
            requester_->addNewMessages(getCurrentTime(),ml,ml.begin(),ml.end());
            smsc_log_debug(log_,"upload dlv %u/%u finished",dlvId_,regionId_);
            return 0;
        }

    private:
        static smsc::logger::Logger*  log_;

    private:
        NewMessageStorage* storage_;
        dlvid_type        dlvId_;
        regionid_type      regionId_;
        unsigned           count_;
    };

public:
    NewMessageStorage() : msgid_(0)
    {
    }

    virtual void requestNewMessages( MessageUploadRequester& req,
                                     dlvid_type             dlvId,
                                     regionid_type           regionId,
                                     unsigned                count )
    {
        MessageUploadTask* mut = new GenTask(*this,req,dlvId,regionId,count);
        pool_.startTask( mut );
    }

    msgid_type getNextMsgId() {
        MutexGuard mg(seqMutex_);
        return ++msgid_;
    }


private:
    smsc::core::threads::ThreadPool pool_;
    smsc::core::synchronization::Mutex seqMutex_;
    msgid_type msgid_;
};

smsc::logger::Logger* NewMessageStorage::GenTask::log_ = 0;


class Delivery 
{
public:
    Delivery( const DlvInfo& dlvInfo,
              StoreLog&      storeLog,
              MessageSource& messageSource ) :
    dlvInfo_(dlvInfo), cache_(dlvInfo_,storeLog,messageSource) {}

    inline dlvid_type getDlvId() const { return dlvInfo_.getDlvId(); }
    inline unsigned getPriority() const { return dlvInfo_.getPriority(); }
    bool isActive() const { return true; }
    inline RegionalStoragePtr getRegionalStorage( regionid_type regionId ) {
        return cache_.getRegionalStorage(regionId);
    }

private:
    DlvInfo      dlvInfo_;
    MessageCache cache_;
};


class ReceiptReceiver
{
public:
    virtual void receiptReceived( msgtime_type    currentTime,
                                  const DlvMsgId& msgId,
                                  uint8_t         state,
                                  int             smppStatus ) = 0;
};


class SmscConnector : protected smsc::core::threads::Thread
{
private:
    typedef std::list< DlvMsgId > ResponseList;
    typedef std::multimap< msgtime_type, ResponseList::iterator > ResponseMap;

public:
    SmscConnector( ReceiptReceiver& rr ) :
    rr_(&rr),
    log_(smsc::logger::Logger::getInstance("smsconn")),
    stopping_(true) {}

    virtual ~SmscConnector() {
        stop();
        WaitFor();
    }

    /// send msg and returns the number of chunks sent or 0.
    int send( msgtime_type currentTime, Delivery& dlv, regionid_type regionId, Message& msg )
    {
        if ( msg.msgId % 30 == 7 ) {
            return 0;
        } else if ( msg.msgId % 50 == 19 ) {
            throw smsc::util::Exception("message %u cannot be sent",msg.msgId);
        }
        DlvMsgId dlvmsg;
        dlvmsg.dlvId = dlv.getDlvId();
        dlvmsg.regId = regionId;
        dlvmsg.msgId = msg.msgId;
        smsc_log_debug(log_,"msg %u/%u/%u is sent",
                       dlvmsg.dlvId, dlvmsg.regId, dlvmsg.msgId );
        scheduleResponse( currentTime + msg.msgId % 5 + 3, dlvmsg );
        return 1;
    }

    void start() {
        if (!stopping_) return;
        MutexGuard mg(mon_);
        if (!stopping_) return;
        stopping_ = false;
        Start();
    }

    void stop() {
        if (stopping_) return;
        MutexGuard mg(mon_);
        if (stopping_) return;
        stopping_ = true;
        mon_.notifyAll();
    }

protected:

    void scheduleResponse( msgtime_type respTime, const DlvMsgId& dlvmsg )
    {
        MutexGuard mg(mon_);
        ResponseMap::iterator iter = responseMap_.upper_bound(respTime);
        ResponseList::iterator j = responseList_.insert( iter == responseMap_.end() ?
                                                         responseList_.end() :
                                                         iter->second,
                                                         dlvmsg );
        responseMap_.insert( iter, std::make_pair(respTime,j) );
        if ( log_->isDebugEnabled() ) {
            const msgtime_type now = getCurrentTime();
            smsc_log_debug(log_,"response for dlv=%u, msg=%u, reg=%u scheduled for %+d",
                           dlvmsg.dlvId, dlvmsg.msgId, dlvmsg.regId, respTime - now );
        }
        mon_.notifyAll();
    }


    virtual int Execute()
    {
        smsc_log_debug(log_,"started");
        while (!stopping_) {

            const msgtime_type now = getCurrentTime();
            ResponseList workingList;
            {
                MutexGuard mg(mon_);
                ResponseMap::iterator iter = responseMap_.upper_bound(now);
                if ( iter == responseMap_.begin() ) {
                    // no elements at this time
                    const msgtime_type diff = 
                        ( responseMap_.empty() ? 5 :
                          responseMap_.begin()->first - now );
                    smsc_log_debug(log_,"next response will be in %d seconds",diff);
                    if ( diff > 0 ) {
                        mon_.wait( diff*1000 );
                    }
                    continue;
                }

                // moving working elements to the working list
                workingList.splice(workingList.begin(),responseList_,
                                   responseList_.begin(),
                                   iter == responseMap_.end() ?
                                   responseList_.end() :
                                   iter->second );
                responseMap_.erase(responseMap_.begin(), iter);
            }

            // processing working list
            for ( ResponseList::iterator i = workingList.begin();
                  i != workingList.end(); ++i ) {
                try {
                    rr_->receiptReceived( now, *i, MsgState::delivered, 0 );
                    smsc_log_debug(log_,"receipt %u/%u processed",
                                   i->dlvId, i->msgId );
                } catch ( std::exception& e ) {
                    smsc_log_warn(log_,"receipt %u/%u cannot be processed",
                                  i->dlvId, i->msgId );
                }
            }

        }
        smsc_log_debug(log_,"finished");
        return 0;
    }

protected:
    smsc::core::synchronization::EventMonitor mon_;
    ResponseList          responseList_;
    ResponseMap           responseMap_;
    ReceiptReceiver*      rr_;
    smsc::logger::Logger* log_;
    bool                  stopping_;
};


class Region
{
public:
    Region( regionid_type regionId, unsigned bw ) :
    regionId_(regionId), bw_(bw) {}

    regionid_type getRegionId() const { return regionId_; }
    unsigned      getBandwidth() const { return bw_; }

private:
    regionid_type  regionId_;
    unsigned       bw_; //  sms/sec
};


class RegionSender
{
    static const int BASEINCREMENT = 1000;

public:
    RegionSender( SmscConnector* conn, Region* r ) :
    conn_(conn), region_(r),
    log_(smsc::logger::Logger::getInstance("regsend")),
    taskList_(*this,BASEINCREMENT*2,smsc::logger::Logger::getInstance("tsklist")) {}

    ~RegionSender() { delete region_; }

    inline regionid_type getRegionId() const {
        return region_->getRegionId(); 
    }

    inline unsigned getBandwidth() const {
        return region_->getBandwidth();
    }

    inline unsigned isReady( unsigned deltaus )
    {
        return speedControl_.isReady(deltaus);
    }

    void addDelivery( Delivery* dlv )
    {
        if (dlv) {
            MutexGuard mg(lock_);
            taskList_.add( dlv );
            RegionalStoragePtr ptr = dlv->getRegionalStorage(getRegionId());
            /*
            if (!ptr.get()) {
                ptr.reset( new RegionalStorage( dlv->getDlvInfo(),
                                                getRegionId(),
                                                storeLog_,
                                                conn_->getMessageSource() ) );
            }
             */
        }
    }

    inline void suspend( unsigned deltaus )
    {
        speedControl_.suspend(deltaus);
    }

    unsigned processRegion( msgtime_type currentTime, unsigned deltaus )
    {
        smsc_log_debug(log_,"rgn=%u processing",getRegionId());
        MutexGuard mg(lock_);
        static const unsigned sleepTime = 100000U;
        currentTime_ = currentTime;
        return taskList_.processOnce(deltaus,sleepTime);
    }

private:
    void scoredObjToString( std::string& s, const Delivery& dlv )
    {
        char buf[20];
        sprintf(buf,"dlv=%u",dlv.getDlvId());
        s.append(buf);
    }

    unsigned scoredObjIsReady( unsigned deltaus, Delivery& dlv )
    {
        try {
            if ( dlv.isActive() ) {
                ptr_ = dlv.getRegionalStorage( getRegionId() );
                if ( ! ptr_.get() ) {
                    smsc_log_debug(log_,"rgn=%u, dlv=%u, region storage is not found",
                                   getRegionId(), dlv.getDlvId() );
                } else if ( ptr_->getNextMessage(currentTime_,msg_) ) {
                    return 0;
                } else {
                    smsc_log_debug(log_,"rgn=%u, dlv=%u, no msg is ready",
                                   getRegionId(), dlv.getDlvId() );
                }
            } else {
                smsc_log_debug(log_,"rgn=%u, dlv=%u is not ready",
                               getRegionId(), dlv.getDlvId() );
            }
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"rgn=%u, exc in objIsReady dlv=%u: %s",
                          getRegionId(), dlv.getDlvId(), e.what());
        }
        return 3000000U;
    }


    int processScoredObj( unsigned deltaus, Delivery& dlv )
    {
        try {

            const int nchunks = conn_->send( currentTime_, dlv, getRegionId(), msg_ );
            if ( nchunks > 0 ) {
                // message has been put into output queue
                ptr_->messageSent( msg_.msgId, currentTime_ );
                return BASEINCREMENT/nchunks/dlv.getPriority();
            } else {
                smsc_log_warn(log_,"rgn=%u, dlv=%u, procObj nchunks=%d",
                              getRegionId(), dlv.getDlvId(), nchunks);
            }

        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"rgn=%u, dlv=%u, procObj exc: %s",
                          getRegionId(), dlv.getDlvId(), e.what());
        }
        // FIXME: message could not be sent, mark it as failed?
        ptr_->retryMessage( msg_.msgId, currentTime_, 60, 8 );
        return -BASEINCREMENT;
    }


private:
    typedef Delivery  ScoredObjType;
    friend class ScoredList< RegionSender >;

private:
    SmscConnector*                     conn_;
    Region*                            region_;
    smsc::logger::Logger*              log_;
    ScoredList< RegionSender >         taskList_;
    smsc::core::synchronization::Mutex lock_;
    SpeedControl<1000000U,uint64_t>    speedControl_;
    msgtime_type                       currentTime_;
    RegionalStoragePtr                 ptr_;
    Message                            msg_;
};


class MessageDispatcher;

class RegionHolder
{
private:
    typedef std::map< regionid_type, RegionSender* > MapType;

public:
    explicit RegionHolder( MessageDispatcher& md ) : md_(&md) {}

    ~RegionHolder() {
        stop();
        for ( MapType::iterator i = regions_.begin(); i != regions_.end(); ++i ) {
            delete i->second;
        }
        for ( std::vector< SmscConnector* >::iterator i = conns_.begin(); i != conns_.end(); ++i ) {
            delete *i;
        }
    }

    RegionSender* addRegion( Region* r );

    void stop()
    {
        for ( std::vector< SmscConnector* >::iterator i = conns_.begin(); i != conns_.end(); ++i ) {
            (*i)->stop();
        }
    }

private:
    MapType                       regions_;
    MessageDispatcher*            md_;
    std::vector< SmscConnector* > conns_;
};



class MessageDispatcher : protected smsc::core::threads::Thread, public ReceiptReceiver
{
    static const int BASEINCREMENT = 20000;
    typedef IntHash< Delivery* > DlvMap;

public:
    MessageDispatcher() :
    log_(smsc::logger::Logger::getInstance("msgdisp")),
    regionHolder_(new RegionHolder(*this)),
    scoredList_(*this,BASEINCREMENT*2,smsc::logger::Logger::getInstance("rsnlist")),
    stopping_(true),
    changed_(false)
    {}


    virtual ~MessageDispatcher() 
    {
        stop();
        /*
        for ( unsigned i = 0; i < scoredList_.size(); ++i ) {
            delete scoredList_[i];
        }
        scoredList_.clear();
         */
    }

    void addRegion( Region* reg ) 
    {
        if (!reg) return;
        RegionSender* rs = regionHolder_->addRegion(reg);
        if (rs) {
            MutexGuard mg(mon_);
            scoredList_.add(rs); 
            // adding all deliveries to this sender
            int key;
            Delivery* value;
            for ( DlvMap::Iterator i(deliveries_); i.Next(key,value); ) {
                rs->addDelivery( value );
            }
            mon_.notifyAll();
            changed_ = true;
        }
    }

    void addDelivery( Delivery* dlv ) {
        if (dlv) {
            MutexGuard mg(mon_);
            Delivery** ptr = deliveries_.GetPtr( dlv->getDlvId() );
            if ( ptr ) {
                smsc_log_warn(log_,"delivery %u already exists", dlv->getDlvId());
                delete dlv;
                return;
            }
            deliveries_.Insert( dlv->getDlvId(), dlv );
            // adding this delivery to all regions
            const size_t regionCount = scoredList_.size();
            for ( size_t i = 0; i < regionCount; ++i ) {
                scoredList_[i]->addDelivery( dlv );
            }
            mon_.notifyAll();
            changed_ = true;
        }
    }

    void start() {
        if (!stopping_) return;
        MutexGuard mg(mon_);
        if (!stopping_) return;
        stopping_ = false;
        Start();
    }


    virtual void receiptReceived( msgtime_type    currentTime,
                                  const DlvMsgId& dlvmsg,
                                  uint8_t         state,
                                  int             smppState )
    {
        RegionalStoragePtr ptr;
        Delivery** dlv;
        do {
            MutexGuard mg(mon_);
            dlv = deliveries_.GetPtr(dlvmsg.dlvId);
            if (!dlv || !*dlv) break;
            ptr = (*dlv)->getRegionalStorage( dlvmsg.regId );
        } while ( false );

        if (!dlv) {
            smsc_log_warn(log_,"delivery %u is not found",dlvmsg.dlvId);
            return;
        }
        if (!ptr.get()) {
            smsc_log_warn(log_,"region %u is not found in delivery %u",
                          dlvmsg.regId, dlvmsg.dlvId );
            return;
        }
        // working with region
        ptr->finalizeMessage( dlvmsg.msgId, currentTime, state, smppState );
    }


private:
    void stop() {
        if (stopping_) return;
        MutexGuard mg(mon_);
        if (stopping_) return;
        stopping_ = true;
        mon_.notifyAll();
        WaitFor();
    }

    virtual int Execute() {
        smsc_log_info(log_,"started");
        usectime_type movingStart = currentTimeMicro();
        usectime_type nextWakeTime;
        const unsigned sleepTime = 100000U;
        while ( !stopping_ ) {

            currentTime_ = currentTimeMicro();
            currentMsgTime_ = getCurrentTime();

            int waitTime = int((nextWakeTime - currentTime_)/1000);
            if ( waitTime > 0 || changed_ ) {
                if (waitTime<10) waitTime = 10;
                MutexGuard mg(mon_);
                if (!changed_) {
                    mon_.wait(waitTime);
                    continue;
                }
                changed_ = false;
            }

            unsigned deltaTime = unsigned(currentTime_ - movingStart);
            if ( deltaTime > 1000000000U ) {
                movingStart += deltaTime;
                deltaTime = 0;
                smsc_log_debug(log_,"moving start to %llu",movingStart);
            }
            smsc_log_debug(log_,"new pass at %u", deltaTime );

            // sending message
            MutexGuard mg(mon_);
            nextWakeTime = currentTime_ + scoredList_.processOnce( deltaTime, sleepTime );
        }
        smsc_log_info(log_,"finished");
        return 0;
    }


    usectime_type currentTimeMicro() const {
        return usectime_type(TimeSourceSetup::AbsUSec::getUSec());
    }


    void scoredObjToString( std::string& s, const RegionSender& rs )
    {
        char buf[20];
        sprintf(buf,"regsend=%u",rs.getRegionId());
        s.append(buf);
    }


    unsigned scoredObjIsReady( unsigned deltaus, RegionSender& rs )
    {
        return rs.isReady(deltaus);
    }


    int processScoredObj( unsigned deltaus, RegionSender& rs )
    {
        const int inc = int(BASEINCREMENT/rs.getBandwidth());
        try {
            unsigned wantToSleep = rs.processRegion( currentMsgTime_,
                                                     deltaus);
            if ( wantToSleep > 0 ) {
                rs.suspend(deltaus+wantToSleep);
                return -inc;
            }
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"region %u sending failed: %s",rs.getRegionId(),e.what());
            rs.suspend(deltaus+1000000U);
            return -BASEINCREMENT;
        }
        return inc;
    }


private:
    typedef RegionSender ScoredObjType;

private:
    friend class ScoredList< MessageDispatcher >;
    smsc::logger::Logger*                     log_;
    smsc::core::synchronization::EventMonitor mon_;
    std::auto_ptr< RegionHolder >             regionHolder_;
    ScoredList< MessageDispatcher >           scoredList_;
    DlvMap                                    deliveries_;
    usectime_type                             currentTime_; // updated from execute
    msgtime_type                              currentMsgTime_;
    bool stopping_;
    bool changed_;
};


RegionSender* RegionHolder::addRegion( Region* r ) 
{
    if (!r) return 0;
    if ( regions_.find(r->getRegionId()) != regions_.end() ) {
        delete r;
        return 0;
    }
    SmscConnector* conn = new SmscConnector(*md_);
    conns_.push_back(conn);
    RegionSender* rs = new RegionSender(conn,r);
    regions_.insert(std::make_pair(r->getRegionId(),rs));
    conn->start();
    return rs;
}



int main()
{
    // smsc::logger::Logger::initForTest( smsc::logger::Logger::LEVEL_DEBUG );
    smsc::logger::Logger::Init();
    smsc::logger::Logger* logmain = smsc::logger::Logger::getInstance("main");

    initgmt20100101();

    smsc::core::synchronization::EventMonitor mon;
    smsc::core::synchronization::MutexGuard mg(mon);

    // typedef smsc::util::TimeSourceSetup::AbsSec AbsSec;

    // initializing regions
    /*
    const unsigned commonSpeedFactor = 10;
    for ( unsigned i = 0; i < REGIONS; ++i ) {
        regionSpeed[i] = commonSpeedFactor / ((i % 10)+1);
        regionSent[i] = 0;
    }
     */

    StoreLog storeLog("./storelog.bin");
    smsc_log_info(logmain,"storelog created");
    NewMessageStorage messageSource;
    MessageDispatcher md;

    md.start();

    {
        Delivery* dlv = new Delivery(DlvInfo(15U),storeLog,messageSource);
        md.addDelivery(dlv);
    }
    {
        Delivery* dlv = new Delivery(DlvInfo(22U),storeLog,messageSource);
        md.addDelivery(dlv);
    }

    md.addRegion( new Region(1U, 5U) );
    md.addRegion( new Region(2U,10U) );
    md.addRegion( new Region(3U, 2U) );
    

    // std::auto_ptr<RegionProc> proc(new RegionProc());

    /*
    std::auto_ptr<MessageCache> store( makeStore(dlvInfo,storeLog,messageSource,proc.get()) );
    smsc_log_info(logmain,"messagecache created");

    msgtime_type startTime = getCurrentTime();
    // int regionId = 1;
    for ( unsigned i = 0; i < 200; ++i ) {
        // get regionId
        const msgtime_type currentTime = getCurrentTime();
        const unsigned deltaTime = currentTime - startTime;
        unsigned sleepTime = proc->processOnce( deltaTime, 100U );
        if ( sleepTime > 10 ) {
            mon.wait(sleepTime);
        }
    }
    smsc_log_info(logmain,"\n-----\nRolling over\n-----");
    store->rollOver();
     */

    sleep(10);
    return 0;
}
