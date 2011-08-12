#include "ReceiptProcessor.h"
#include "RegionSender.h"
#include "SmscSender.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/CoreSmscStats.h"
#include "informer/data/DeliveryInfo.h"
#include "informer/io/FileGuard.h"
#include "informer/io/FileReader.h"
#include "informer/io/IOConverter.h"
#include "informer/io/InfosmeException.h"
#include "informer/io/SpeedControl.h"
#include "informer/io/Typedefs.h"
#include "informer/io/UTF8.h"
#include "smpp/smpp_structures.h"
#include "sms/IllFormedReceiptParser.h"
#include "sms/sms.h"
#include "system/status.h"
#include "informer/snmp/SnmpManager.h"
#include "informer/data/DeadLockWatch.h"

using smsc::core::synchronization::MutexGuard;

namespace {

using namespace eyeline::informer;

struct RSScan
{
    RSScan( std::vector< regionid_type >& r ) : regions(r) {}
    bool operator () ( const RegionSenderPtr rs ) {
        if (rs.get()) { regions.push_back(rs->getRegionId()); }
        return false;
    }
    std::vector< regionid_type >& regions;
};

struct RSBandwidth
{
    RSBandwidth() : bandwidth(0) {}
    bool operator () ( const RegionSenderPtr rs ) {
        if (rs.get()) { bandwidth += rs->getBandwidth(); }
        return false;
    }
    unsigned bandwidth;
};

struct ResetSpeed
{
    ResetSpeed( usectime_type cur ) : now(cur) {}
    bool operator () ( const RegionSenderPtr& rs ) {
        if (rs.get()) { const_cast<RegionSender&>(*rs).resetSpeedControl(now); }
        return false;
    }
    usectime_type now;
};

}



namespace eyeline {
namespace informer {




class SmscSender::SmscJournal : protected smsc::core::threads::Thread
{
    static const unsigned LENSIZE = 2;
public:

    class SJReader : public FileReader::RecordReader
    {
    public:
        SJReader( SmscSender& sender ) : sender_(sender), unique_(0) {}

        virtual bool isStopping() {
            return getCS()->isStopping(); // sender_.isStopping_;
        }

        virtual size_t recordLengthSize() const {
            return LENSIZE;
        }

        virtual size_t readRecordLength( size_t filePos, char* buf, size_t buflen )
        {
            size_t rl(FromBuf(buf,buflen).get16());
            if (rl>100) {
                throw InfosmeException(EXC_BADFILE,"record at %llu has invalid len: %u",
                                       ulonglong(filePos), unsigned(rl));
            }
            return rl;
        }

        virtual bool readRecordData( size_t filePos, char* buf, size_t buflen )
        {
            FromBuf fb(buf,buflen);
            ReceiptData rd;
            rd.drmId.dlvId = fb.get32();
            rd.drmId.regId = fb.get32();
            bool insert;
            if ( rd.drmId.regId == anyRegionId ) {
                // remove record
                insert = false;
            } else {
                insert = true;
                rd.drmId.msgId = fb.get64();
                rd.endTime     = fb.get32();
                rd.responded   = fb.get16();
                rd.status = smsc::system::Status::OK;
                rd.retry = false;
            }
            rd.rcptId.setMsgId(fb.getCString());
            if ( !insert ) {
                ReceiptList::iterator iter;
                if ( sender_.receiptHash_.Pop(rd.rcptId.msgId,iter) ) {
                    --unique_;
                    sender_.receiptList_.erase(iter);
                }
            } else {
                if ( !sender_.receiptHash_.GetPtr(rd.rcptId.msgId)) {
                    ++unique_;
                    sender_.receiptHash_.Insert(rd.rcptId.msgId,
                                                sender_.receiptList_.insert(sender_.receiptList_.begin(),rd));
                    return true;
                }
            }
            return false;
        }
    public:
        SmscSender& sender_;
        size_t      unique_; // number of unique records
    };


    SmscJournal( SmscSender& sender ) :
    sender_(sender),
    log_(0),
    speedControl_(std::max(getCS()->getSmscJournalRollingSpeed(),1U)*1000),
    isStopping_(false),
    mon_(MTXWHEREAMI)
    {
        char buf[128];
        ::snprintf(buf,sizeof(buf),"sjr.%s",sender.getSmscId().c_str());
        log_ = smsc::logger::Logger::getInstance(buf);
    }

    ~SmscJournal() {
        stop();
        fg_.close();
    }

    void init()
    {
        std::string jpath(makePath());
        readRecordsFrom(jpath+".old");
        readRecordsFrom(jpath);
        fg_.create(jpath.c_str(),0666,true);
        fg_.seek(0,SEEK_END);
    }

    void start() {
        // if (!isStopping_) return;
        if (getCS()->isStopping()) return;
        isStopping_ = false;
        Start();
    }

    void stop() {
        smsc_log_info(log_,"S='%s' waiting until journal is stopped",sender_.smscId_.c_str());
        isStopping_ = true;
        {
            MutexGuard mg(mon_);
            mon_.notifyAll();
        }
        WaitFor();
    }


    unsigned journalReceiptData( const ReceiptData& rd )
    {
        // assert(rd.responded > 0);
        char buf[100];
        ToBuf tb(buf,sizeof(buf));
        tb.skip(LENSIZE);
        tb.set32(rd.drmId.dlvId);
        tb.set32(rd.drmId.regId);
        tb.set64(rd.drmId.msgId);
        tb.set32(rd.endTime);
        tb.set16(rd.responded);
        tb.setCString(rd.rcptId.msgId);
        const size_t pos = tb.getPos();
        tb.setPos(0);
        tb.set16(uint16_t(pos-LENSIZE));
        smsc_log_debug(log_,"S='%s' storing D=%u/R=%u/M=%llu msgId='%s' %u bytes",
                       sender_.smscId_.c_str(),
                       rd.drmId.dlvId, rd.drmId.regId, rd.drmId.msgId,
                       rd.rcptId.msgId, unsigned(pos) );
        MutexGuard mg(mon_);
        fg_.write(buf,pos);
        return unsigned(pos);
    }

    void deleteReceiptData( const ResponseData& rd )
    {
        char buf[100];
        ToBuf tb(buf,sizeof(buf));
        tb.skip(LENSIZE);
        tb.set32(0);           // unused
        tb.set32(anyRegionId); // delete record
        tb.setCString(rd.rcptId.msgId);
        const size_t pos = tb.getPos();
        tb.setPos(0);
        tb.set16(uint16_t(pos-LENSIZE));
        smsc_log_debug(log_,"S='%s' removing msgId='%s' %u bytes",
                       sender_.smscId_.c_str(),
                       rd.rcptId.msgId, unsigned(pos) );
        MutexGuard mg(mon_);
        fg_.write(buf,pos);
    }

protected:

    void rollOver()
    {
        const std::string jpath(makePath());
        smsc_log_debug(log_,"rolling over '%s'",jpath.c_str());
        if (-1 == rename(jpath.c_str(),(jpath+".old").c_str())) {
            throw ErrnoException(errno,"rename('%s')",jpath.c_str());
        }
        FileGuard fg;
        fg.create(jpath.c_str(),0666);
        {
            smsc::core::synchronization::MutexGuard mg(mon_);
            fg_.swap(fg);
        }
        smsc_log_debug(log_,"file '%s' rolled, pos=%llu",
                       jpath.c_str(),ulonglong(fg.getPos()));
    }


    std::string makePath() const {
        return getCS()->getStorePath() + "journals/smsc" + sender_.smscId_ + ".journal";
    }


    void readRecordsFrom( const std::string& jpath )
    {
        smsc_log_debug(log_,"reading journal '%s'",jpath.c_str());
        FileGuard fg;
        try {
            fg.ropen(jpath.c_str());
        } catch ( std::exception& e ) {
            smsc_log_debug(log_,"cannot read '%s', exc: %s", jpath.c_str(), e.what());
            return;
        }
        SJReader sjreader(sender_);
        TmpBuf<char,8192> buf;
        FileReader fileReader(fg);
        try {
            const size_t total = fileReader.readRecords(buf,sjreader);
            smsc_log_info(log_,"journal '%s' has been read, %u/%u total/unique records",jpath.c_str(),
                          unsigned(total), unsigned(sjreader.unique_) );
        } catch ( FileReadException& e ) {
            smsc_log_error(log_,"file '%s', (FIXME) exc: %s", jpath.c_str(), e.what());
            // FIXME: the smsc journal is corrupted, should we trunk the file?
            if ( getCS()->getSnmp() ) {
                getCS()->getSnmp()->sendTrap( SnmpTrap::TYPE_FILEIO,
                                              SnmpTrap::SEV_MAJOR,
                                              "smscjnl",
                                              jpath.c_str(),
                                              e.what() );
            }
            // FIXME: for now we do not throw
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"file '%s', exc: %s", jpath.c_str(), e.what());
        }
    }


    virtual int Execute()
    {
        smsc_log_debug(log_,"S='%s' journal roller started", sender_.smscId_.c_str());
        while ( !getCS()->isStopping() && !isStopping_ ) {
            bool firstPass = true;
            speedControl_.suspend( currentTimeMicro() % flipTimePeriod );
            ReceiptData rd;
            smsc_log_debug(log_,"S='%s' rolling pass started",
                           sender_.smscId_.c_str());
            do {
                try {
                    if (!sender_.getNextRollingData(rd,firstPass)) {
                        break;
                    }
                    firstPass = false;
                    if (getCS()->isStopping() || isStopping_) { break; }
                    if (!rd.responded) { continue; }
                    speedControl_.consumeQuant( journalReceiptData(rd) );
                } catch ( std::exception& e ) {
                    smsc_log_warn(log_,"S='%s' journal roller exc: %s",
                                  sender_.smscId_.c_str(), e.what());
                }
                const usectime_type sleepTime = 
                    speedControl_.isReady( currentTimeMicro() % flipTimePeriod,
                                           maxSnailDelay );
                if ( sleepTime > 10000 ) {
                    MutexGuard mg(mon_);
                    mon_.wait(unsigned(sleepTime/1000));
                }

            } while (true);
            const bool stopping = getCS()->isStopping() || isStopping_;
            smsc_log_debug(log_,"S='%s' rolling pass done, stop=%d",
                           sender_.smscId_.c_str(),stopping );
            if (!stopping) {
                try {
                    rollOver();
                    MutexGuard mg(mon_);
                    mon_.wait(getCS()->getSmscJournalRollingPeriod()*1000);
                } catch ( std::exception& e ) {
                    smsc_log_warn(log_,"S='%s' journal rolling exc: %s",
                                  sender_.smscId_.c_str(), e.what());
                }
            }
        }
        smsc_log_debug(log_,"S='%s' journal roller stopped",sender_.smscId_.c_str());
        return 0;
    }

private:
    SmscSender&                               sender_;
    smsc::logger::Logger*                     log_;
    SpeedControl<usectime_type,tuPerSec>      speedControl_;
    bool                                      isStopping_;
    smsc::core::synchronization::EventMonitor mon_;
    FileGuard                                 fg_;
};


// =========================================================================

namespace {
    
smsc::logger::Logger* checkSmscName( const std::string& smscId )
{
    if ( smscId.size() >= SMSC_ID_LENGTH ) {
        throw InfosmeException(EXC_BADNAME,
                               "SMSC id '%s' is too long", smscId.c_str());
    }
    char c;
    if ( ! isGoodAsciiName(smscId.c_str(),&c) ) {
        throw InfosmeException(EXC_BADNAME,
                               "SMSC id '%s' contains forbidden character '%c'",
                               smscId.c_str(), c );
    }
    return smsc::logger::Logger::getInstance(("s." + smscId).c_str());
}

}

SmscSender::SmscSender( ReceiptProcessor&  core,
                        const std::string& smscId,
                        const SmscConfig&  cfg,
                        smsc::util::config::Config* retryConfig ) :
log_(checkSmscName(smscId)),
reflock_(MTXWHEREAMI),
ref_(0),
rproc_(core),
parser_(0),
smscId_(smscId),
smscConfig_(cfg),
reconfLock_(MTXWHEREAMI),
scoredList_(*this, 2*maxScoreIncrement,0/*log_*/),
connTime_(0),
maxBandwidth_(0),
smsCounter_(5*tuPerSec,50),
receiptMon_( MTXWHEREAMI ),
journal_(0),
queueMon_( MTXWHEREAMI ),
awaken_(false),
isStopping_(false),
hasSessionError_(false)
{
    journal_ = new SmscJournal(*this);
    dlwatch_ = new DeadLockWatch( getCS()->getDLWatcher(),
                                  "SMSC",
                                  smscId.c_str(),
                                  getCS()->getDeadLockWatchPeriod() );
    // session_.reset( new smsc::sme::SmppSession(cfg.smeConfig,this) );
    parser_ = new smsc::sms::IllFormedReceiptParser();
    wQueue_.reset(new DataQueue());
    journal_->init();
    isStopping_ = true;
    // restore receipt wait queue
    const msgtime_type curTime = currentTimeSeconds();
    for ( ReceiptList::iterator i = receiptList_.begin();
          i != receiptList_.end(); ++i ) {
        rcptWaitQueue_.insert(std::make_pair(i->endTime,i->rcptId));
        smsc_log_debug(log_,"S='%s' restore rcpt timer=%+d msgid='%s' R=%u/D=%u/M=%llu",
                       smscId_.c_str(),
                       int(i->endTime - curTime), i->rcptId.msgId,
                       i->drmId.regId, i->drmId.dlvId, i->drmId.msgId );
    }
    retryPolicy_.init(retryConfig);
}


SmscSender::~SmscSender()
{
    stop();
    if (session_.get()) {
        session_->close();
    }
    if (parser_) delete parser_;
    if (wQueue_.get()) {
        assert(wQueue_->Count() == 0);
        wQueue_.reset(0);
    }
    if (journal_) {
        delete journal_;
    }
    if (dlwatch_) {
        delete dlwatch_;
    }
}


int SmscSender::send( RegionalStorage& ptr, Message& msg,
                      int untilAE, int& nchunks )
{
    const DeliveryInfo& info = ptr.getDlvInfo();

    smsc_log_debug(log_,"send(R=%u/D=%u/M=%llu )",
                   unsigned(ptr.getRegionId()),
                   unsigned(info.getDlvId()),
                   ulonglong(msg.msgId));
    char whatbuf[150];
    const char* what = "";
    int res = smsc::system::Status::OK;
    int seqNum = 0;
    nchunks = 0;
    do {

        if (getCS()->isStopping() || isStopping_) {
            what = "stopped";
            res = smsc::system::Status::SMENOTCONNECTED;
            break;
        }

        if ( session_->isClosed() || hasSessionError_ ) {
            what = "not connected";
            res = smsc::system::Status::SMENOTCONNECTED;
            break;
        }

        // check the number of seqnums
        if ( unsigned(seqnumHash_.Count()) > smscConfig_.unrespondedMessagesMax ) {
            what = "too many unresp msgs";
            res = smsc::system::Status::MSGQFUL;
            break;
        }

        // NOTE: it is guaranteed that msg.lastTime is currentTime
        const msgtime_type now = msg.lastTime;

        // calculate the validityTime of the message
        timediff_type validityTime = std::min(msg.timeLeft,info.getValidityPeriod());

        if ( validityTime > smscConfig_.maxValidityTime ) {
            validityTime = smscConfig_.maxValidityTime;
        }
        if ( untilAE && validityTime > untilAE ) {
            validityTime = untilAE;
        }

        smsc_log_debug(log_,"R=%u/D=%u/M=%llu S.minVT=%d S.maxVT=%d msg.TTL=%d d.validty=%d untilAE=%d -> VT=%u",
                       ptr.getRegionId(), info.getDlvId(),
                       ulonglong(msg.msgId),
                       smscConfig_.minValidityTime,
                       smscConfig_.maxValidityTime,
                       msg.timeLeft,
                       info.getValidityPeriod(),
                       untilAE, validityTime );

        if (validityTime < smscConfig_.minValidityTime) {
            what = "too small validity time for smsc";
            res = smsc::system::Status::DELIVERYTIMEDOUT;
            break;
        }

        if (validityTime < 20) {
            // less than half a minute, cannot send
            what = "validity time is less than 20 sec";
            res = smsc::system::Status::DELIVERYTIMEDOUT;
            break;
        }

        // receive and register the seqnum
        do {
            seqNum = session_->getNextSeq();
        } while (seqNum == 0);
        DRMTrans* drm = seqnumHash_.GetPtr(seqNum);
        if (drm) {
            // we have to cleanup respTimer
            if (drm->respTimer != respWaitQueue_.end()) {
                smsc_log_debug(log_,"S='%s' clean spurious resp timer seq=%u R=%u/D=%u/M=%llu",
                               smscId_.c_str(),
                               drm->respTimer->second,
                               drm->regId, drm->dlvId, drm->msgId );
                respWaitQueue_.erase(drm->respTimer);
            }
        } else {
            drm = &seqnumHash_.Insert(seqNum,DRMTrans());
        }

        // making responseTimer
        {
            ResponseTimer rt;
            const msgtime_type endTime = now + getCS()->getResponseWaitTime();
            rt.seqNum = seqNum;
            drm->respTimer = respWaitQueue_.insert(respWaitQueue_.end(),
                                                   std::make_pair(endTime,rt));
        }

        // filling drm
        drm->dlvId = info.getDlvId();
        drm->regId = ptr.getRegionId();
        drm->msgId = msg.msgId;
        drm->nchunks = 0;
        if ( ! msg.flags.hasTransactional(drm->trans) ) {
            drm->trans = info.isTransactional();
        }
        drm->endTime = now + validityTime + getCS()->getReceiptExtraWaitTime();
        smsc_log_debug(log_,"S='%s' add resp timer=%+d seq=%u R=%u/D=%u/M=%llu rcpt.endTime=%+d",
                       smscId_.c_str(),
                       int(drm->respTimer->first - now),
                       seqNum,
                       drm->regId, drm->dlvId, drm->msgId,
                       int(drm->endTime - now) );

        // prepare the sms
        try {

            // convert subscribers to addresses
            smsc::sms::Address da;
            {
                uint8_t len;
                uint64_t addr;
                char buf[20];
                addr = subscriberToAddress(msg.subscriber,len,da.type,da.plan);
                sprintf(buf,"%*.*llu",len,len,ulonglong(addr));
                da.setValue(len,buf);
            }

            smsc::sms::SMS sms;
            smsc::sms::Address oa;
            if ( msg.flags.getSourceAddress(oa) ) {
                // FIXME: check if the address is allowed
            } else {
                info.getSourceAddress(oa);
            }
            sms.setOriginatingAddress(oa);
            sms.setDestinationAddress(da);
            sms.setArchivationRequested(false);
            sms.setDeliveryReport(1);

            sms.setValidTime(now + validityTime);

            do {
                // NOTE: svctype is a mandatory parameter
                bool rip;
                if ( ! msg.flags.hasReplaceIfPresent(rip) ) {
                    rip = info.isReplaceIfPresent();
                }
                sms.setIntProperty( smsc::sms::Tag::SMPP_REPLACE_IF_PRESENT_FLAG,
                                    rip ? 1 : 0 );
                if ( rip ) {
                    // set svctype from msg/dlv only if replace_if_present is set
                    const char* mst = msg.flags.getSvcType();
                    if ( mst && mst[0] ) {
                        sms.setEServiceType(mst);
                        break;
                    } else {
                        char svcType[DLV_SVCTYPE_LENGTH];
                        info.getSvcType(svcType);
                        if ( svcType[0] ) {
                            sms.setEServiceType(svcType);
                            break;
                        }
                    }
                }
                // otherwise set common svctype
                sms.setEServiceType(getCS()->getSvcType());
            } while (false);
            sms.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID, getCS()->getProtocolId());
            sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS, drm->trans ? 2 : 0);
            sms.setIntProperty(smsc::sms::Tag::SMPP_PRIORITY, 0);
            sms.setIntProperty(smsc::sms::Tag::SMPP_REGISTRED_DELIVERY, 1);
            {
                bool flash;
                if ( ! msg.flags.hasFlash(flash) ) {
                    flash = info.isFlash();
                }
                if (flash) {
                    sms.setIntProperty(smsc::sms::Tag::SMPP_DEST_ADDR_SUBUNIT,1);
                }
            }

            const char* out = msg.text.getText();
            if (!out) {
                what = "text is null";
                res = smsc::system::Status::SYSERR;
                break;
            }
            try {
                nchunks = info.evaluateNchunks(out,strlen(out),&sms);
            } catch ( std::exception& e ) {
                what = "wrong message body";
                res = smsc::system::Status::SYSERR;
                break;
            }

            bool useDataSm;
            if ( ! msg.flags.hasUseDataSm(useDataSm) ) {
                useDataSm = info.useDataSm();
            }

            {
                DlvMode dlvMode;
                if ( ! msg.flags.getDeliveryMode(dlvMode) ) {
                    dlvMode = info.getDeliveryMode();
                }

                if ( dlvMode != DLVMODE_SMS ) {
                    // force submitsm
                    useDataSm = false;

                    // ussdpush
                    const int ussdop = ( dlvMode == DLVMODE_USSDPUSH ?
                                         smscConfig_.ussdPushOp :
                                         smscConfig_.ussdPushVlrOp );
                    if (ussdop == -1) {
                        smsc_log_warn(log_,"S='%s': ussd not supported, R=%u/D=%u/M=%llu",
                                      smscId_.c_str(),
                                      ptr.getRegionId(),
                                      info.getDlvId(),
                                      ulonglong(msg.msgId));
                        res = smsc::system::Status::SYSERR;
                        break;
                    }
                
                    try {
                        sms.setIntProperty(smsc::sms::Tag::SMPP_USSD_SERVICE_OP,ussdop);
                    } catch ( std::exception& e ) {
                        smsc_log_error(log_,"S='%s': ussdpush: cannot set ussd op %u",ussdop);
                        res = smsc::system::Status::SYSERR;
                        break;
                    }
                } // ussdpush
            }

            // smpp unknown optionals
            const char* unknownBuf;
            const size_t unknownLen = msg.flags.getExtraTLV(unknownBuf);
            if ( unknownLen ) {
                sms.setBinProperty(smsc::sms::Tag::SMSC_UNKNOWN_OPTIONALS,
                                   unknownBuf,unsigned(unknownLen));
            }
            
            drm->nchunks = nchunks;
            smsc_log_debug(log_,"S='%s' R=%u/D=%u/M=%llu %s seq=%u .%u.%u.%*.*s -> .%u.%u.%*.*s",
                           smscId_.c_str(),
                           ptr.getRegionId(),
                           info.getDlvId(),
                           ulonglong(msg.msgId),
                           useDataSm ? "data_sm" : "submit_sm",
                           seqNum,
                           oa.type, oa.plan, oa.length, oa.length, oa.value,
                           da.type, da.plan, da.length, da.length, da.value );
            if (useDataSm) {
                sms.setIntProperty( smsc::sms::Tag::SMPP_QOS_TIME_TO_LIVE, validityTime );
                smsc::smpp::PduDataSm dataSm;
                dataSm.get_header().set_sequenceNumber(seqNum);
                dataSm.get_header().set_commandId(smsc::smpp::SmppCommandSet::DATA_SM);
                fillDataSmFromSms(&dataSm,&sms);
                session_->getAsyncTransmitter()->sendPdu(&(dataSm.get_header()));
            } else {
                smsc::smpp::PduSubmitSm submitSm;
                submitSm.get_header().set_sequenceNumber(seqNum);
                submitSm.get_header().set_commandId(smsc::smpp::SmppCommandSet::SUBMIT_SM);
                fillSmppPduFromSms(&submitSm, &sms);
                // if (log_->isDebugEnabled()) {
                // submitSm.dump(log_,0);
                // }
                session_->getAsyncTransmitter()->sendPdu(&(submitSm.get_header()));
            }
            smsCounter_.accumulate(currentTime_,nchunks);
            rproc_.incOutgoing(nchunks);
            break;

        } catch ( std::exception& e ) {
            snprintf(whatbuf,sizeof(whatbuf),"exc: %s",e.what());
            what = whatbuf;
            res = smsc::system::Status::SYSERR;
        }

    } while ( false );

    if (res==smsc::system::Status::OK && nchunks>0) {
        if (log_->isDebugEnabled()) {
            uint8_t len, ton, npi;
            uint64_t addr = subscriberToAddress(msg.subscriber,len,ton,npi);
            smsc_log_debug(log_,"S='%s' R=%u/D=%u/M=%llu A=.%u.%u.%*.*llu seq=%u sent",
                           smscId_.c_str(),
                           ptr.getRegionId(),
                           info.getDlvId(),
                           ulonglong(msg.msgId),
                           ton,npi,len,len,ulonglong(addr), seqNum);
        }
        return res;
    }

    if (seqNum!=0) {
        DRMTrans drm;
        if (seqnumHash_.Pop(seqNum,drm)) {
            if (drm.respTimer != respWaitQueue_.end()) {
                smsc_log_debug(log_,"S='%s' on fail remove resp timer seq=%u R=%u/D=%u/M=%llu ",
                               smscId_.c_str(), seqNum,
                               drm.regId, drm.dlvId, drm.msgId );
                respWaitQueue_.erase(drm.respTimer);
            }
        }
    }

    uint8_t len, ton, npi;
    uint64_t addr = subscriberToAddress(msg.subscriber,len,ton,npi);
    smsc_log_error(log_,"S='%s' R=%u/D=%u/M=%llu A=.%u.%u.%0*.*llu failed(%d): %s",
                   smscId_.c_str(),
                   ptr.getRegionId(),
                   info.getDlvId(),
                   ulonglong(msg.msgId),
                   ton,npi,len,len,ulonglong(addr), res, what);
    return res;
}


int SmscSender::sendTestSms( const char*        sourceAddress,
                             personid_type      subscriber,
                             const char*        text,
                             bool               isFlash,
                             int                deliveryMode )
{
    if ( !sourceAddress || !*sourceAddress ) {
        throw InfosmeException(EXC_LOGICERROR,"bad address NULL/empty");
    }
    if ( !text || !*text ) {
        throw InfosmeException(EXC_LOGICERROR,"bad text NULL/empty");
    }

    smsc::sms::Address oa;
    try {
        oa = smsc::sms::Address(sourceAddress);
        if (oa.length <= 1 && oa.type == 0 && oa.plan == 0 ) {
            throw InfosmeException(EXC_BADADDRESS,"bad address '%s'",sourceAddress);
        }
    } catch ( InfosmeException& e ) {
        throw;
    } catch ( std::exception& e ) {
        throw InfosmeException(EXC_BADADDRESS,"bad address '%s' exc: %s",sourceAddress,e.what());
    }

    smsc::sms::Address da;
    {
        uint8_t len;
        uint64_t addr;
        char buf[20];
        addr = subscriberToAddress(subscriber,len,da.type,da.plan);
        sprintf(buf,"%*.*llu",len,len,ulonglong(addr));
        da.setValue(len,buf);
        smsc_log_info(log_,"== sendTestSms(%s,.%u.%u.%*.*llu,'%s')",
                      sourceAddress, da.type, da.plan, len, len, ulonglong(addr),
                      text );
    }

    smsc::smpp::PduSubmitSm sbm;
    smsc::smpp::PduPartSm& msg = sbm.get_message();
    msg.set_source(smsc::smpp::Address2PduAddress(oa));
    msg.set_dest(smsc::smpp::Address2PduAddress(da));
    msg.set_esmClass(2); // transactional
    if ( isFlash ) {
        sbm.get_optional().set_destAddrSubunit(1);
    }
    
    const size_t textLen = strlen(text);
    if ( smsc::util::hasHighBit(text,textLen) ) {
        msg.set_dataCoding( smsc::util::DataCoding::UCS2 );
        TmpBuf<char,2048> ucstext;
        getCS()->getUTF8().convertToUcs2(text,textLen,ucstext);
        const size_t buflen = ucstext.GetPos();
        assert(buflen%2==0);
        const short* start = reinterpret_cast<short*>(ucstext.get());
        for ( short* end = reinterpret_cast<short*>(ucstext.GetCurPtr());
              end != start; ) {
            --end;
            *end = htons(*end);
        }
        sbm.get_optional().set_messagePayload(ucstext.get(),int(buflen));
    } else {
        msg.set_dataCoding(smsc::util::DataCoding::LATIN1);
        sbm.get_optional().set_messagePayload(text,int(textLen));
    }
    sbm.get_header().set_commandId(smsc::smpp::SmppCommandSet::SUBMIT_SM);

    MutexGuard mg(reconfLock_);
    
    if ( deliveryMode != DLVMODE_SMS ) {
        const int ussdop = ( deliveryMode == DLVMODE_USSDPUSH ?
                             smscConfig_.ussdPushOp : smscConfig_.ussdPushVlrOp );
        if (ussdop == -1) {
            throw InfosmeException(EXC_ACCESSDENIED,"ussd not supported");
        }
        sbm.get_optional().set_ussdServiceOp(ussdop);
    }

    if ( !session_.get() ||
         session_->isClosed() ||
         hasSessionError_ ) {
        return smsc::system::Status::SMENOTCONNECTED;
    }

    sbm.get_header().set_sequenceNumber(session_->getNextSeq());

    smsc::smpp::PduSubmitSmResp* resp = session_->getSyncTransmitter()->submit(sbm);
    if ( !resp ) {
        throw InfosmeException(EXC_EXPIRED, "no resp");
    }
    const int ret = resp->get_header().get_commandStatus();
    disposePdu((smsc::smpp::SmppHeader*)resp);
    return ret;
}


void SmscSender::updateConfig( const SmscConfig& config,
                               smsc::util::config::Config* retryConfig )
{
    stop();
    MutexGuard mg(reconfLock_);
    retryPolicy_.init(retryConfig);
    smscConfig_ = config;
    session_.reset(new smsc::sme::SmppSession(smscConfig_.smeConfig,this));
}


void SmscSender::detachRegionSender( RegionSender& rs )
{
    smsc_log_debug(log_,"S='%s' detaching regsend R=%u",smscId_.c_str(),unsigned(rs.getRegionId()));
    {
        RegionSenderPtr rp(&rs);
        MutexGuard mg(reconfLock_);
        scoredList_.remove(ScoredPtrList<SmscSender>::isEqual(rp));
    }
    updateBandwidth();
}


void SmscSender::attachRegionSender( RegionSender& rs )
{
    smsc_log_debug(log_,"S='%s' attaching regsend R=%u",smscId_.c_str(),unsigned(rs.getRegionId()));
    {
        RegionSenderPtr rp(&rs);
        MutexGuard mg(reconfLock_);
        if ( scoredList_.has(ScoredPtrList<SmscSender>::isEqual(rp)) ) {
            return;
        }
        scoredList_.add(rp);
    }
    updateBandwidth();
    wakeUp();
}


void SmscSender::getRegionList( std::vector< regionid_type >& regions )
{
    MutexGuard mg(reconfLock_);
    regions.reserve(scoredList_.size());
    ::RSScan rss(regions);
    // we use remove to scan the list
    scoredList_.remove( rss );
}


void SmscSender::updateBandwidth()
{
    MutexGuard mg(reconfLock_);
    ::RSBandwidth rsbw;
    scoredList_.remove(rsbw);
    maxBandwidth_ = rsbw.bandwidth;
}


void SmscSender::getSmscStats( usectime_type currentTime,
                               CoreSmscStats& stats )
{
    MutexGuard mg(reconfLock_);
    if (connTime_) {
        stats.liveTime = 
            timediff_type( std::max(currentTime-connTime_,
                                    usectime_type(0LL)) / tuPerSec );
    } else {
        stats.liveTime = -1;
    }
    stats.nRegions = unsigned(scoredList_.size());
    stats.maxBandwidth = maxBandwidth_;
    stats.avgInterval = msgtime_type(smsCounter_.getInterval() / tuPerSec);
    stats.currentLoad = unsigned(smsCounter_.advanceTime(currentTime));
    stats.nResponses = unsigned(respWaitQueue_.size());
    stats.nReceipts = unsigned(rcptWaitQueue_.size());
}


void SmscSender::handleEvent( smsc::sme::SmppHeader* pdu )
{
    smsc_log_debug(log_,"S='%s' pdu received, cmdid=%x", smscId_.c_str(), pdu->get_commandId());
    switch (pdu->get_commandId()) {
    case smsc::smpp::SmppCommandSet::DELIVERY_SM:
    case smsc::smpp::SmppCommandSet::DATA_SM:
        handleReceipt(pdu);
        break;
    case smsc::smpp::SmppCommandSet::SUBMIT_SM_RESP:
    case smsc::smpp::SmppCommandSet::DATA_SM_RESP:
        handleResponse(pdu);
        break;
    case smsc::smpp::SmppCommandSet::ENQUIRE_LINK:
    case smsc::smpp::SmppCommandSet::ENQUIRE_LINK_RESP:
        break;
    default:
        smsc_log_warn(log_,"S='%s' unsupported pdu cmdid=%0x",
                      smscId_.c_str(), pdu->get_commandId());
    }
    disposePdu(pdu);
}


void SmscSender::handleError( int errorCode )
{
    smsc_log_error(log_,"S='%s' transport error, code=%d", smscId_.c_str(), errorCode);
    hasSessionError_ = true; // session_->close();
}


void SmscSender::handleReceipt( smsc::sme::SmppHeader* pdu )
{
    assert(pdu);
    int badstat = smsc::system::Status::INVDCS;
    try {

        smsc::sms::SMS sms;
        switch (pdu->get_commandId()) {
        case smsc::smpp::SmppCommandSet::DELIVERY_SM:
            fetchSmsFromSmppPdu(reinterpret_cast<smsc::smpp::PduXSm*>(pdu),&sms);
            break;
        case smsc::smpp::SmppCommandSet::DATA_SM:
            fetchSmsFromDataSmPdu(reinterpret_cast<smsc::smpp::PduDataSm*>(pdu),&sms);
            break;
        default:
            return;
        }
    
        const bool isReceipt = 
            sms.hasIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS) ?
            ((sms.getIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS) & 0x3c) == 0x4) :
            false;
        
        if ( !isReceipt ) {
            badstat = smsc::system::Status::INVCMDID;
            throw InfosmeException(EXC_BADFORMAT,"is not a receipt");
        }
    
        uint8_t msgState;
        int err;
        ResponseData rd;
        const char* msgid = parser_->parseSms(sms,rd.rcptId.msgId,msgState,err);
        // preprocessing err
        err &= 0xffff;
        if ( err & 0x8000 ) {
            static const int completer = -1 & ~0xffff;
            err |= completer;
        }
            
        if ( !msgid || *msgid == '\0') {
            // broken msgid
            badstat = smsc::system::Status::INVMSGID;
            throw InfosmeException(EXC_BADFORMAT,"msgid is empty");
        }
    
        // msgid is ok
        rd.seqNum = 0;    // receipt
        rd.status = pdu->get_commandStatus();
    
        bool delivered = false;
        bool retry = false;
        switch (msgState) {
        case smsc::smpp::SmppMessageState::DELIVERED:
            delivered = true;
            break;
        case smsc::smpp::SmppMessageState::EXPIRED:
        case smsc::smpp::SmppMessageState::DELETED:
            retry = true;
            break;
        case smsc::smpp::SmppMessageState::ENROUTE:
        case smsc::smpp::SmppMessageState::ACCEPTED:
        case smsc::smpp::SmppMessageState::UNKNOWN:
            smsc_log_warn(log_,"S='%s' sms msgid='%s' seq=%u has intermediate receipt state %d, skipped",
                          smscId_.c_str(), msgid, pdu->get_sequenceNumber(), msgState);
            badstat = smsc::system::Status::SYSERR;
            throw InfosmeException(EXC_BADFORMAT,"has intermediate state");
        case smsc::smpp::SmppMessageState::REJECTED:
        case smsc::smpp::SmppMessageState::UNDELIVERABLE:
            // permanent error
            break;
        default:
            smsc_log_warn(log_,"S='%s' sms msgid='%s' seq=%u invalid receipt state=%d",
                          smscId_.c_str(), msgid, pdu->get_sequenceNumber(), msgState);
            break;
        } // switch msgState
        
        if (!delivered) {
            if (rd.status == smsc::system::Status::OK) {
                // replacing with network errcode
                rd.status = err;
            }
            if ( rd.status == smsc::system::Status::OK ) {
                if (retry) {
                    rd.status = smsc::system::Status::DELIVERYTIMEDOUT;
                } else {
                    rd.status = smsc::system::Status::UNKNOWNERR;
                }
                // smsc_log_warn(log_,"S='%s' sms msgid='%s' seq=%u receipt has status=OK but not delivered",
                // smscId_.c_str(), msgid, pdu->get_sequenceNumber());
            }
        }
    
        rd.rcptId.setMsgId(msgid);
        rd.retry = retry;
    
        if (queueData(rd)) {
            switch (pdu->get_commandId()) {
            case smsc::smpp::SmppCommandSet::DELIVERY_SM: {
                smsc::smpp::PduDeliverySmResp smResp;
                smResp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DELIVERY_SM_RESP);
                smResp.set_messageId("");
                smResp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
                session_->getAsyncTransmitter()->sendDeliverySmResp(smResp);
                break;
            }
            case smsc::smpp::SmppCommandSet::DATA_SM: {
                smsc::smpp::PduDataSmResp smResp;
                smResp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DATA_SM_RESP);
                smResp.set_messageId("");
                smResp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
                session_->getAsyncTransmitter()->sendDataSmResp(smResp);
                break;
            }
            default:
                break;
            }
        }
    } catch ( std::exception& e ) {
        smsc_log_error(log_,"S='%s' receipt seq=%u exc: %s", smscId_.c_str(), pdu->get_sequenceNumber(), e.what());
        switch ( pdu->get_commandId() ) {
        case smsc::smpp::SmppCommandSet::DELIVERY_SM: {
            smsc::smpp::PduDeliverySmResp smResp;
            smResp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DELIVERY_SM_RESP);
            smResp.set_messageId("");
            smResp.get_header().set_commandStatus(badstat);
            smResp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
            session_->getAsyncTransmitter()->sendDeliverySmResp(smResp);
            break;
        }
        case smsc::smpp::SmppCommandSet::DATA_SM: {
            smsc::smpp::PduDataSmResp smResp;
            smResp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DATA_SM_RESP);
            smResp.set_messageId("");
            smResp.get_header().set_commandStatus(badstat);
            smResp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
            session_->getAsyncTransmitter()->sendDataSmResp(smResp);
            break;
        }
        default:
            break;
        }
    }
}


void SmscSender::handleResponse( smsc::sme::SmppHeader* pdu )
{
    assert(pdu);
    ResponseData rd;
    rd.seqNum = pdu->get_sequenceNumber();
    if (!rd.seqNum) {
        smsc_log_error(log_,"S='%s': logic error resp w/ seq=0",smscId_.c_str());
        return;
    }
    rd.status = pdu->get_commandStatus();
    bool accepted = ( rd.status == smsc::system::Status::OK );
    const char* msgid = reinterpret_cast<smsc::sme::PduXSmResp*>(pdu)->get_messageId();
    {
        const char* passedid = msgid;
        if (!msgid || *msgid == '\0') {
            accepted = false;
            passedid = "";
            if (rd.status == smsc::system::Status::OK) {
                smsc_log_debug(log_,"S='%s' resp seq=%u empty msgid, replace status -> %d",
                               smscId_.c_str(), rd.seqNum, smsc::system::Status::RX_T_APPN);
                rd.status = smsc::system::Status::RX_T_APPN;
            }
        }
        rd.rcptId.setMsgId(passedid);
    }
    if (!accepted) {
        smsc_log_info(log_,"S='%s' sms msgid='%s' seq=%u wasn't accepted, status=%d",
                      smscId_.c_str(), rd.rcptId.msgId, rd.seqNum, rd.status );
    }
    rd.retry = (rd.status != smsc::system::Status::OK);
    queueData(rd);
}


bool SmscSender::queueData( const ResponseData& rd )
{
    // if (getCS()->isStopping() || isStopping_ ) return false;
    smsc_log_debug(log_,"S='%s' %s pending seq=%u status=%d msgid='%s'",
                   smscId_.c_str(),
                   rd.seqNum ? "response" : "receipt",
                   rd.seqNum, rd.status, rd.rcptId.msgId );
    MutexGuard mg(queueMon_);
    if (getCS()->isStopping() || isStopping_ ) return false;
    wQueue_->Push(rd);
    queueMon_.notify();
    return true;
}


bool SmscSender::processQueue( DataQueue& queue )
{
    ResponseData rd;
    // FIXME: optimize queue (preprocess) to match resp+rcpt
    bool throttled = false;
    const msgtime_type now = currentTimeSeconds();
    while ( queue.Pop(rd) ) {

        smsc_log_debug(log_,"S='%s' processing RD(seq=%u,status=%d,msgid='%s',retry=%d)",
                       smscId_.c_str(),rd.seqNum,rd.status,rd.rcptId.msgId,rd.retry);

        if ( rd.seqNum ) {
            // it is a response

            DRMTrans drm;
            if ( !seqnumHash_.Pop(rd.seqNum,drm) ) {
                // not found in seqnum hash, expired
                smsc_log_warn(log_,"S='%s' resp (seq=%u,status=%d,msgid='%s') has no drm mapping, expired?",
                              smscId_.c_str(),rd.seqNum,rd.status,rd.rcptId.msgId);
                continue;
            }
            if (drm.respTimer != respWaitQueue_.end()) {
                // remove timer
                smsc_log_debug(log_,"S='%s' remove resp timer seq=%u R=%u/D=%u/M=%llu",
                               smscId_.c_str(), rd.seqNum,
                               drm.regId, drm.dlvId, drm.msgId );
                respWaitQueue_.erase(drm.respTimer);
            }

            if ( rd.status == smsc::system::Status::THROTTLED ) {
                throttled = true;
            }

            if (drm.trans) {
                smsc_log_info(log_,"S='%s' resp seq=%u R=%u/D=%u/M=%llu ends transaction",
                              smscId_.c_str(), rd.seqNum,
                              drm.regId,
                              drm.dlvId,
                              ulonglong(drm.msgId));
                // FIXME: for now fix SENT counter only for good responses
                const bool fixTransactional = (rd.status == smsc::system::Status::OK);
                rproc_.receiveReceipt( drm, retryPolicy_, rd.status,
                                       rd.retry, drm.nchunks, fixTransactional );
                continue;
            } else if ( *rd.rcptId.msgId == '\0' ) {
                if (rd.status == smsc::system::Status::OK || log_->isDebugEnabled()) {
                    log_->log_( rd.status == smsc::system::Status::OK ?
                                smsc::logger::Logger::LEVEL_WARN :
                                smsc::logger::Logger::LEVEL_DEBUG ,
                                "S='%s' resp seq=%u R=%u/D=%u/M=%llu has no msgId",
                                smscId_.c_str(), rd.seqNum,
                                drm.regId,
                                drm.dlvId,
                                ulonglong(drm.msgId));
                }
                rproc_.receiveReceipt( drm, retryPolicy_, rd.status, rd.retry, drm.nchunks );
                continue;
            }

            // get receipt from receipt cache
            ReceiptList rcptList;
            ReceiptList::iterator iter;
            if ( receiptHash_.Pop(rd.rcptId.msgId,iter) ) {
                // receipt found in cache, so

                // message is finalized as it has both receipt and response
                {
                    smsc::core::synchronization::MutexGuard mg(receiptMon_);
                    if (iter == rollingIter_) { ++rollingIter_; }
                    rcptList.splice(rcptList.begin(),receiptList_,iter);
                }
                // receipt timer removal is not needed here, as it ain't yet

                // finalize message, ignoring resp status
                smsc_log_info(log_,"S='%s' R=%u/D=%u/M=%llu ignoring resp status=%d,retry=%d, using status=%d,retry=%d",
                              smscId_.c_str(),
                              drm.regId,
                              drm.dlvId,
                              ulonglong(drm.msgId),
                              rd.status, rd.retry, iter->status, iter->retry );
                rproc_.receiveReceipt( drm, retryPolicy_, iter->status, iter->retry, drm.nchunks );
                continue;
            }

            if ( rd.status != smsc::system::Status::OK ) {
                // bad response status -- finalization
                rproc_.receiveReceipt( drm, retryPolicy_, rd.status, rd.retry, drm.nchunks );
                continue;
            }

            if ( !rproc_.receiveResponse(drm,drm.nchunks) ) {
                // response processing failed -- no dlv, no msg?
                continue;
            }

            // receipt hash has not been received, adding one
            iter = rcptList.insert(rcptList.begin(),ReceiptData());
            // smsc_log_debug(log_,"S='%s' FIXME: intentionally set immediate timer",smscId_.c_str());
            // drm.endTime = now;
            iter->drmId = drm;
            iter->endTime = drm.endTime;
            iter->rcptId = rd.rcptId;
            iter->status = rd.status;
            iter->responded = drm.nchunks;
            iter->retry = rd.retry;
            receiptHash_.Insert(rd.rcptId.msgId,iter);
            journal_->journalReceiptData(*iter);
            {
                smsc::core::synchronization::MutexGuard mg(receiptMon_);
                receiptList_.splice(receiptList_.begin(),rcptList,iter);
            }

            // adding receipt wait timer
            smsc_log_debug(log_,"S='%s' adding rcpt timer=%+d msgid='%s' R=%u/D=%u/M=%llu",
                           smscId_.c_str(),
                           int(drm.endTime - now),
                           rd.rcptId.msgId,
                           drm.regId, drm.dlvId, drm.msgId );
            rcptWaitQueue_.insert(std::make_pair(drm.endTime,rd.rcptId));

        } else {
            // receipt

            ReceiptList rcptList;
            ReceiptList::iterator* piter = receiptHash_.GetPtr(rd.rcptId.msgId);
            if (piter) {
                ReceiptList::iterator iter = *piter;

                if (iter->responded) {
                    // we have already received a response, finalize

                    // remove the receipt wait timer
                    {
                        std::pair<RcptWaitQueue::iterator, RcptWaitQueue::iterator>
                            range = rcptWaitQueue_.equal_range( iter->endTime );
                        RcptWaitQueue::iterator ir = range.first;
                        for ( ; ir != range.second; ++ir ) {
                            if ( ir->second == iter->rcptId ) {
                                smsc_log_debug(log_,"S='%s' remove rcpt timer=%+d msgid='%s' R=%u/D=%u/M=%llu",
                                               smscId_.c_str(),
                                               int(iter->endTime - now),
                                               iter->rcptId.msgId,
                                               iter->drmId.regId, iter->drmId.dlvId, iter->drmId.msgId );
                                rcptWaitQueue_.erase(ir);
                                break;
                            }
                        }
                        if ( ir == range.second ) {
                            smsc_log_debug(log_,"S='%s' notfound rcpt timer=%+d msgid='%s' R=%u/D=%u/M=%llu",
                                           smscId_.c_str(),
                                           int(iter->endTime - now),
                                           iter->rcptId.msgId,
                                           iter->drmId.regId, iter->drmId.dlvId, iter->drmId.msgId );
                        }
                    }

                    // remove an entry from journal
                    receiptHash_.Delete(rd.rcptId.msgId);
                    {
                        smsc::core::synchronization::MutexGuard mg(receiptMon_);
                        if (rollingIter_ == iter) { ++rollingIter_; }
                        rcptList.splice(rcptList.begin(),receiptList_,iter);
                    }
                    rproc_.receiveReceipt(iter->drmId, retryPolicy_, rd.status, rd.retry, iter->responded );
                    // delete smsc record only after successful processing in core
                    journal_->deleteReceiptData(rd);

                } else {
                    // still not responded, modify receipt status
                    // note: we don't need to clean up rcpt timer here as it is not set yet
                    iter->status = rd.status;
                    iter->retry = rd.retry;
                }
                continue;
            }

            // iter is not found, so it is not responded
            ReceiptList::iterator iter = rcptList.insert(rcptList.begin(),ReceiptData());
            iter->responded = 0; // not responded
            iter->status = rd.status;
            iter->rcptId = rd.rcptId;
            iter->retry = rd.retry;
            receiptHash_.Insert(rd.rcptId.msgId,iter);
            // NOTE: we do not journal this entry as it has not been responded
            {
                smsc::core::synchronization::MutexGuard mg(receiptMon_);
                receiptList_.splice(receiptList_.begin(),rcptList,iter);
            }

        } // if receipt

    } // while pop rd
    return throttled;
}


void SmscSender::start()
{
    // if (!isStopping_) return;
    {
        MutexGuard mg(queueMon_);
        if ( getCS()->isStopping()) return;
        // if (!isStopping_) return;
        isStopping_ = false;
    }
    {
        MutexGuard rmg(reconfLock_);
        session_.reset(new smsc::sme::SmppSession(smscConfig_.smeConfig,this));
    }
    Start();
}


void SmscSender::stop()
{
    // if (isStopping_) return;
    smsc_log_debug(log_,"S='%s' stop received",smscId_.c_str());
    journal_->stop();
    {
        MutexGuard mg(queueMon_);
        // if (isStopping_) return;
        isStopping_ = true;
        awaken_ = true;
        queueMon_.notifyAll();
    }
    WaitFor();
}


void SmscSender::wakeUp()
{
    smsc_log_debug(log_,"awakening");
    MutexGuard mg(queueMon_);
    awaken_ = true;
    queueMon_.notify();
}


int SmscSender::Execute()
{
    while ( !getCS()->isStopping() && !isStopping_ ) {
        try {
            connectLoop();
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"S='%s' connectLoop exc: %s", smscId_.c_str(), e.what());
        }
        if (getCS()->isStopping() || isStopping_ ) break;
        // we are connected
        if ( getCS()->getSnmp() ) {
            getCS()->getSnmp()->sendTrap( SnmpTrap::TYPE_CONNECT,
                                          SnmpTrap::SEV_CLEAR,
                                          "SMSC",
                                          smscId_.c_str(),
                                          "connected" );
        }
        try {
            journal_->start();
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"S='%s' starting journal exc: %s", smscId_.c_str(), e.what());
            break;
        }
        try {
            sendLoop();
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"S='%s' sendLoop exc: %s", smscId_.c_str(), e.what());
        }
        if (!getCS()->isStopping() && !isStopping_ && getCS()->getSnmp() ) {
            getCS()->getSnmp()->sendTrap( SnmpTrap::TYPE_CONNECT,
                                          SnmpTrap::SEV_MINOR,
                                          "SMSC",
                                          smscId_.c_str(),
                                          "disconnected" );
        }

        // we have broken connection here, so we have to report all waiting resps
        // unless we are in stopping state.
        std::multimap<msgtime_type, ResponseTimer>::iterator iter = respWaitQueue_.begin();
        for ( ; iter != respWaitQueue_.end(); ++iter ) {
            if (getCS()->isStopping() || isStopping_) break;
            ResponseData rd;
            ResponseTimer& rt = iter->second;
            rd.seqNum = rt.seqNum;
            rd.status = smsc::system::Status::SMENOTCONNECTED;
            rd.rcptId.setMsgId("");
            rd.retry = true;
            DRMTrans* drm = seqnumHash_.GetPtr(rd.seqNum);
            if (drm && drm->respTimer == iter) {
                // remove ref to this timer
                drm->respTimer = respWaitQueue_.end();
            }
            queueData(rd);
            smsc_log_debug(log_,"S='%s' disconnected resp timer seq=%u",
                           smscId_.c_str(),
                           rd.seqNum);
        }
        respWaitQueue_.erase( respWaitQueue_.begin(), iter );
        journal_->stop();
    }
    return 0;
}


void SmscSender::connectLoop()
{
    bool isFirstAttempt = true;
    while ( !getCS()->isStopping() && !isStopping_) {

        const usectime_type startingConn = currentTimeMicro();
        msgtime_type interConnect;
        {
            if (dlwatch_) { dlwatch_->ping(msgtime_type(startingConn/tuPerSec)); }
            MutexGuard mg(reconfLock_);
            connTime_ = 0;
            smsc_log_debug(log_,"S='%s': trying to connect",smscId_.c_str());
            if ( !session_.get() ) {
                throw InfosmeException(EXC_LOGICERROR,"session is not configured");
            } else if ( !session_->isClosed() ) {
                // session connected
                connTime_ = startingConn;
                hasSessionError_ = false;
                break;
            }
            try {
                interConnect = smscConfig_.interConnectPeriod;
                session_->connect();
                if (!session_->isClosed()) {
                    connTime_ = currentTimeMicro();
                    hasSessionError_ = false;
                    break;
                }
            } catch ( std::exception& e ) {
                smsc_log_error(log_,"session connect exc: %s", e.what());
            }
            // cannot connect
            if ( getCS()->getSnmp() && isFirstAttempt ) {
                isFirstAttempt = false;
                getCS()->getSnmp()->sendTrap( SnmpTrap::TYPE_CONNECT,
                                              SnmpTrap::SEV_MINOR,
                                              "SMSC",
                                              smscId_.c_str(),
                                              "cannot connect" );
            }
        }
        // connection failed, waiting
        MutexGuard mg(queueMon_);
        if (getCS()->isStopping() || isStopping_) break;
        const usectime_type now = currentTimeMicro();
        const int waitTime = int(usectime_type(interConnect)*1000 -
                                 (now - startingConn)/1000);
        // smsc_log_debug(log_,"startcon=%llu now=%llu interconn=%u wait=%d",
        // startingConn,now,interConnect,waitTime);
        if ( waitTime > 0 ) {
            queueMon_.wait(waitTime);
        }
    }
}


void SmscSender::sendLoop()
{
    const unsigned sleepTime = 5000000U; // 5 sec

    currentTime_ = currentTimeMicro();
    {
        // reset speed limits on all RS using SpeedReset predicate
        MutexGuard mg(reconfLock_);
        scoredList_.remove( ResetSpeed(currentTime_) );
    }

    usectime_type nextWakeTime = currentTime_;
    std::auto_ptr<DataQueue> rQueue(new DataQueue);
    while ( !getCS()->isStopping() && !isStopping_ ) {

        currentTime_ = currentTimeMicro();
        if (dlwatch_) { dlwatch_->ping( msgtime_type(currentTime_/tuPerSec)); }

        if (rQueue->Count() == 0) {

            MutexGuard mg(queueMon_);
            if (wQueue_->Count()>0) {
                // taking wqueue
                std::swap(rQueue,wQueue_);
            } else if (awaken_) {
                awaken_ = false;
                nextWakeTime = currentTime_;

            } else {

                // sleeping until next wake time
                int waitTime = int((nextWakeTime - currentTime_ + 1000)/1000U); // in msec
                if (waitTime>0) {
                    if (waitTime < 5) waitTime = 5;
                    // smsc_log_debug(log_,"S='%s' is going to sleep %d msec",
                    // smscId_.c_str(),waitTime);
                    queueMon_.wait(waitTime);
                    if (wQueue_->Count()>0) {
                        std::swap(rQueue,wQueue_);
                    }
                    continue;
                }
            }
        }

        MutexGuard mg(reconfLock_);  // prevent reconfiguration
        if ( !session_.get() ||
             session_->isClosed() ||
             hasSessionError_ ) {
            break; 
        }
        bool throttled = false;
        if (rQueue->Count() > 0) {
            throttled = processQueue(*rQueue.get());
        }
        if (nextWakeTime > currentTime_) continue;
        processExpiredTimers();

        // check the number of seqnums
        if ( throttled ) {
            nextWakeTime = currentTime_ + 1000;
            continue;
        } else if ( unsigned(seqnumHash_.Count()) > smscConfig_.unrespondedMessagesMax ) {
            // too many unresponded, wait one millisec
            nextWakeTime = currentTime_ + 1000;
            continue;
        }

        nextWakeTime = currentTime_ + scoredList_.processOnce(0/* not used*/,
                                                              sleepTime);
    }
    {
        MutexGuard mg(reconfLock_);
        if (session_.get() && !session_->isClosed()) {
            session_->close();
        }
        if (rQueue->Count() > 0) processQueue(*rQueue.get());
    }
    {
        MutexGuard mg(queueMon_);
        std::swap(rQueue,wQueue_);
    }
    if (rQueue->Count() > 0) processQueue(*rQueue.get());
}


unsigned SmscSender::scoredObjIsReady( unsigned, ScoredPtrType regionSender )
{
    if ( ! regionSender->getBandwidth() ) {
        return 10000000U; // waits ten seconds
    }
    const unsigned ret = regionSender->isReady(currentTime_);
    // smsc_log_debug(log_,"R=%u waits %u usec until ready()",
    // regionSender->getRegionId(), ret);
    return ret;
}


int SmscSender::processScoredObj( unsigned, ScoredPtrType regionSender, unsigned& objSleep )
{
    // unsigned inc = maxScoreIncrement/regionSender.getBandwidth();
    unsigned bw = regionSender->getBandwidth();
    if (bw==0) { bw = 1; }
    const int ret = int(maxScoreIncrement/bw);
    objSleep = regionSender->processRegion(currentTime_);
    return ( objSleep > 0 ? -ret : ret );
}


void SmscSender::scoredObjToString( std::string& s, ScoredPtrType regionSender )
{
    s += regionSender->toString();
}


void SmscSender::processExpiredTimers()
{
    // NOTE: i'm too lazy to implement the same code here, so calling queueData
    const msgtime_type now = msgtime_type(currentTime_/tuPerSec);
    if ( getCS()->isStopping() || isStopping_ ) return;
    {
        std::multimap<msgtime_type, ResponseTimer>::iterator iter, upto;
        iter = respWaitQueue_.begin();
        upto = respWaitQueue_.lower_bound(now);
        for ( ; iter != upto; ++iter ) {
            if (getCS()->isStopping() || isStopping_ ) { break; }
            ResponseData rd;
            ResponseTimer& rt = iter->second;
            rd.seqNum = rt.seqNum;
            rd.status = smsc::system::Status::DELIVERYTIMEDOUT;
            rd.rcptId.setMsgId("");
            rd.retry = true;
            DRMTrans* drm = seqnumHash_.GetPtr(rd.seqNum);
            if (drm && drm->respTimer == iter) {
                // remove ref to this timer
                drm->respTimer = respWaitQueue_.end();
            }
            queueData(rd);
            smsc_log_debug(log_,"S='%s' expired resp timer=%+d seq=%u",
                           smscId_.c_str(),
                           int(iter->first - now),
                           rd.seqNum);
        }
        respWaitQueue_.erase(respWaitQueue_.begin(),iter);
    }

    if (getCS()->isStopping() || isStopping_ ) return;
    {
        std::multimap<msgtime_type, ReceiptId>::iterator iter, upto;
        iter = rcptWaitQueue_.begin();
        upto = rcptWaitQueue_.lower_bound(now);
        for ( ; iter != upto; ++iter ) {
            if (getCS()->isStopping() || isStopping_ ) { break; }
            ResponseData rd;
            rd.seqNum = 0; // receipt
            rd.status = smsc::system::Status::DELIVERYTIMEDOUT;
            rd.rcptId.setMsgId(iter->second.msgId);
            rd.retry = true;
            queueData(rd);
            smsc_log_debug(log_,"S='%s' expired rcpt timer=%+d msgId='%s'",
                           smscId_.c_str(), int(iter->first - now),
                           rd.rcptId.msgId );
        }
        rcptWaitQueue_.erase(rcptWaitQueue_.begin(),iter);
    }
}


bool SmscSender::getNextRollingData( ReceiptData& rd, bool firstPass )
{
    MutexGuard mg(receiptMon_);
    if (firstPass) {
        rollingIter_ = receiptList_.begin();
    }
    while ( rollingIter_ != receiptList_.end() ) {
        if ( !rollingIter_->responded ) {
            ++rollingIter_;
            if (getCS()->isStopping() || isStopping_) { return false; }
            continue;
        }
        rd = *rollingIter_;
        ++rollingIter_;
        return true;
    }
    return false;
}


}
}
