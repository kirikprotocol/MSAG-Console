#include "ReceiptProcessor.h"
#include "RegionSender.h"
#include "SmscSender.h"
#include "informer/data/CommonSettings.h"
#include "informer/data/DeliveryInfo.h"
#include "informer/io/FileGuard.h"
#include "informer/io/FileReader.h"
#include "informer/io/IOConverter.h"
#include "informer/io/InfosmeException.h"
#include "informer/io/Typedefs.h"
#include "smpp/smpp_structures.h"
#include "sms/IllFormedReceiptParser.h"
#include "sms/sms.h"
#include "system/status.h"

namespace {

using namespace eyeline::informer;

struct RSScan
{
    RSScan( std::vector< regionid_type >& r ) : regions(r) {}
    bool operator () ( const RegionSender* rs ) {
        if (rs) { regions.push_back(rs->getRegionId()); }
        return false;
    }
    std::vector< regionid_type >& regions;
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
            return sender_.isStopping_;
        }

        virtual size_t recordLengthSize() const {
            return LENSIZE;
        }

        virtual size_t readRecordLength( size_t filePos, FromBuf& fb ) {
            size_t rl(fb.get16());
            if (rl>100) {
                throw InfosmeException(EXC_BADFILE,"record at %llu has invalid len: %u",
                                       ulonglong(filePos), unsigned(rl));
            }
            return rl;
        }

        virtual void readRecordData( size_t filePos, FromBuf& fb ) {
            ReceiptData rd;
            rd.drmId.dlvId = fb.get32();
            rd.drmId.regId = fb.get32();
            rd.drmId.msgId = fb.get64();
            rd.responded   = fb.get16();
            rd.status = smsc::system::Status::OK;
            rd.retry = false;
            rd.rcptId.setMsgId(fb.getCString());
            if ( !sender_.receiptHash_.GetPtr(rd.rcptId.msgId)) {
                ++unique_;
                sender_.receiptHash_.Insert(rd.rcptId.msgId,
                                            sender_.receiptList_.insert(sender_.receiptList_.end(),rd));
            }
        }
    public:
        SmscSender& sender_;
        size_t      unique_; // number of unique records
    };


    SmscJournal( SmscSender& sender ) :
    sender_(sender),
    isStopping_(true)
    {}

    ~SmscJournal() {
        stop();
        fg_.close();
    }

    void init()
    {
        std::string jpath(makePath());
        readRecordsFrom(jpath+".old");
        readRecordsFrom(jpath);
        fg_.create(jpath.c_str(),0666,true,true);
        fg_.seek(0,SEEK_END);
    }

    void start() {
        if (!isStopping_) return;
        isStopping_ = false;
        Start();
    }

    void stop() {
        if (isStopping_) return;
        isStopping_ = true;
        {
            smsc::core::synchronization::MutexGuard mg(sender_.receiptMon_);
            sender_.receiptMon_.notifyAll();
        }
        WaitFor();
    }


    void journalReceiptData( const ReceiptData& rd )
    {
        assert(rd.responded > 0);
        char buf[100];
        ToBuf tb(buf,sizeof(buf));
        tb.skip(LENSIZE);
        tb.set32(rd.drmId.dlvId);
        tb.set32(rd.drmId.regId);
        tb.set64(rd.drmId.msgId);
        tb.set16(rd.responded);
        tb.setCString(rd.rcptId.msgId);
        const size_t pos = tb.getPos();
        tb.setPos(0);
        tb.set16(uint16_t(pos-LENSIZE));
        smsc::core::synchronization::MutexGuard mg(lock_);
        fg_.write(buf,pos);
    }

protected:

    void rollOver()
    {
        const std::string jpath(makePath());
        smsc_log_info(sender_.log_,"rolling over '%s'",jpath.c_str());
        if (-1 == rename(jpath.c_str(),(jpath+".old").c_str())) {
            throw ErrnoException(errno,"rename('%s')",jpath.c_str());
        }
        FileGuard fg;
        fg.create(jpath.c_str(),0666);
        {
            smsc::core::synchronization::MutexGuard mg(lock_);
            fg_.swap(fg);
        }
        smsc_log_debug(sender_.log_,"file '%s' rolled",jpath.c_str());
    }


    std::string makePath() const {
        return sender_.rproc_.getCS().getStorePath() + "journals/smsc" + sender_.smscId_ + ".journal";
    }


    void readRecordsFrom( const std::string& jpath )
    {
        FileGuard fg;
        try {
            fg.ropen(jpath.c_str());
        } catch ( std::exception& e ) {
            smsc_log_warn(sender_.log_,"cannot read '%s': %s", jpath.c_str(), e.what());
            return;
        }
        SJReader sjreader(sender_);
        smsc::core::buffers::TmpBuf<char,8192> buf;
        FileReader fileReader(fg);
        try {
            const size_t total = fileReader.readRecords(buf,sjreader);
            smsc_log_info(sender_.log_,"journal '%s' has been read, %u/%u total/unique records",jpath.c_str(),
                          unsigned(total), unsigned(sjreader.unique_) );
        } catch ( FileDataException& e ) {
            smsc_log_warn(sender_.log_,"file '%s': %s", jpath.c_str(), e.what());
            // FIXME: should we trunk the file?
        } catch ( std::exception& e ) {
            smsc_log_error(sender_.log_,"file '%s': %s", jpath.c_str(), e.what());
        }
    }


    virtual int Execute()
    {
        smsc_log_debug(sender_.log_,"S='%s' journal roller started", sender_.smscId_.c_str());
        while ( !isStopping_ ) {
            bool firstPass = true;
            ReceiptList& rl = sender_.receiptList_;
            ReceiptList::iterator& iter = sender_.rollingIter_;
            do {
                ReceiptData rd;
                if (isStopping_) { break; }
                {
                    smsc::core::synchronization::MutexGuard mg(sender_.receiptMon_);
                    if (firstPass) {
                        iter = rl.begin();
                        firstPass = false;
                    }
                    if (iter == rl.end()) { break; }
                    if (!iter->responded) {
                        ++iter;
                        continue;
                    }
                    rd = *iter;
                    ++iter;
                }
                if (isStopping_) { break; }
                journalReceiptData(rd);
                smsc_log_debug(sender_.log_,"FIXME: S='%s' place limit on throughput",sender_.smscId_.c_str());
            } while (true);
            smsc_log_debug(sender_.log_,"S='%s' rolling pass done", sender_.smscId_.c_str());
            if (!isStopping_) { rollOver(); }
            smsc::core::synchronization::MutexGuard mg(sender_.receiptMon_);
            sender_.receiptMon_.wait(10000);
        }
        return 0;
    }

private:
    SmscSender& sender_;
    bool        isStopping_;
    smsc::core::synchronization::Mutex lock_;
    FileGuard   fg_;
};


// =========================================================================

SmscSender::SmscSender( ReceiptProcessor&  core,
                        const std::string& smscId,
                        const SmscConfig&  cfg ) :
log_(smsc::logger::Logger::getInstance("smscsend")),
rproc_(core),
parser_(0),
smscId_(smscId),
smscConfig_(cfg),
scoredList_(*this,2*maxScoreIncrement,
            smsc::logger::Logger::getInstance("reglist")),
journal_(0),
awaken_(false),
isStopping_(true)
{
    {
        char c;
        if ( ! isGoodAsciiName(smscId_.c_str(),&c) ) {
            smsc_log_error(log_,"SMSC id '%s' contains forbidden character '%c'",
                           smscId_.c_str(), c );
            abort();
        }
    }

    journal_ = new SmscJournal(*this);
    // session_.reset( new smsc::sme::SmppSession(cfg.smeConfig,this) );
    parser_ = new smsc::sms::IllFormedReceiptParser();
    wQueue_.reset(new DataQueue());
    journal_->init();
    // restore receipt wait queue
    const msgtime_type now = msgtime_type(currentTimeMicro() / tuPerSec);
    ReceiptTimer rt;
    rt.endTime = now + rproc_.getCS().getReceiptWaitTime();
    for ( ReceiptList::iterator i = receiptList_.begin();
          i != receiptList_.end(); ++i ) {
        rt.rcptId = i->rcptId;
        rcptWaitQueue_.Push(rt);
    }
}


SmscSender::~SmscSender()
{
    stop();
    if (session_.get()) session_->close();
    if (parser_) delete parser_;
    /*
    if (rQueue_) {
        assert(rQueue_->Count() == 0);
        delete rQueue_;
    }
     */
    if (wQueue_.get()) {
        assert(wQueue_->Count() == 0);
        wQueue_.reset(0);
        // delete wQueue_;
    }
    if (journal_) {
        delete journal_;
    }
}


int SmscSender::send( RegionalStorage& ptr, Message& msg, int& nchunks )
{
    const DeliveryInfo& info = ptr.getDlvInfo();

    smsc_log_info(log_,"send(R=%u/D=%u/M=%llu)",
                  unsigned(ptr.getRegionId()),
                  unsigned(info.getDlvId()),
                  ulonglong(msg.msgId));
    char whatbuf[150];
    const char* what = "";
    int res = smsc::system::Status::OK;
    int seqNum;
    nchunks = 0;
    do {

        if (isStopping_) {
            what = "stopped";
            res = smsc::system::Status::SMENOTCONNECTED;
            break;
        }

        if ( session_->isClosed() ) {
            what = "not connected";
            res = smsc::system::Status::SMENOTCONNECTED;
            break;
        }

        const CommonSettings& cs = rproc_.getCS();

        // check the number of seqnums
        if ( unsigned(seqnumHash_.Count()) > cs.getUnrespondedMessagesMax() ) {
            what = "too many unresp msgs";
            // FIXME: which error code to select?
            // res = smsc::system::Status::MSGQFUL;
            res = smsc::system::Status::THROTTLED;
            break;
        }

        const msgtime_type now = msgtime_type(currentTimeMicro() / tuPerSec);
        if ( msg.lastTime + msg.timeLeft < now ) {
            what = "msg is already expired";
            res = smsc::system::Status::EXPIRED;
            break;
        }

        // receive and register the seqnum
        do {
            seqNum = session_->getNextSeq();
        } while (seqNum == 0);
        DRMTrans* drm = seqnumHash_.GetPtr(seqNum);
        if (!drm) {
            drm = &seqnumHash_.Insert(seqNum,DRMTrans());
        }
        drm->dlvId = info.getDlvId();
        drm->regId = ptr.getRegionId();
        drm->msgId = msg.msgId;
        drm->nchunks = 0;
        drm->trans = info.isTransactional();

        {
            ResponseTimer rt;
            rt.endTime = now + cs.getResponseWaitTime();
            rt.seqNum = seqNum;
            respWaitQueue_.Push(rt);
        }

        // prepare the sms
        try {

            // convert subscribers to addresses
            smsc::sms::Address oa, da;
            {
                uint8_t len;
                uint64_t addr;
                char buf[20];
                addr = subscriberToAddress(info.getSourceAddress(),len,oa.type,oa.plan);
                sprintf(buf,"%0*.*llu",len,len,ulonglong(addr));
                oa.setValue(len,buf);
                addr = subscriberToAddress(msg.subscriber,len,da.type,da.plan);
                sprintf(buf,"%0*.*llu",len,len,ulonglong(addr));
                da.setValue(len,buf);
            }

            smsc::sms::SMS sms;
            sms.setOriginatingAddress(oa);
            sms.setDestinationAddress(da);
            sms.setArchivationRequested(false);
            sms.setDeliveryReport(1);
            // FIXME: apply activity period
            sms.setValidTime(msg.lastTime + msg.timeLeft);
            sms.setIntProperty( smsc::sms::Tag::SMPP_REPLACE_IF_PRESENT_FLAG,
                                info.isReplaceIfPresent() ? 1 : 0 );
            sms.setEServiceType( (info.isReplaceIfPresent() && info.getSvcType()[0]) ?
                                 info.getSvcType() : cs.getSvcType() );
            sms.setIntProperty(smsc::sms::Tag::SMPP_PROTOCOL_ID, cs.getProtocolId());
            sms.setIntProperty(smsc::sms::Tag::SMPP_ESM_CLASS,
                               info.isTransactional() ? 2 : 0);
            sms.setIntProperty(smsc::sms::Tag::SMPP_PRIORITY, 0);
            sms.setIntProperty(smsc::sms::Tag::SMPP_REGISTRED_DELIVERY, 1);
            if (info.isFlash()) {
                sms.setIntProperty(smsc::sms::Tag::SMPP_DEST_ADDR_SUBUNIT,1);
            }

            const char* out = msg.text->getText();
            size_t outLen = strlen(msg.text->getText());
            std::auto_ptr<char> msgBuf;
            // FIXME: move this algorithm into regionalstorage
            if (smsc::util::hasHighBit(out,outLen)) {
                size_t msgLen = outLen*2;
                msgBuf.reset(new char[msgLen]);
                // FIXME: replace with conversion from UTF8.
                // FIXME: move conversion into glossary loading.
                // FIXME: it will require to glossary char* -> std::string modification
                ConvertMultibyteToUCS2(out, outLen, (short*)msgBuf.get(), msgLen,
                                       CONV_ENCODING_CP1251);
                sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, DataCoding::UCS2);
                out = msgBuf.get();
                outLen = msgLen;
            } else {
                sms.setIntProperty(smsc::sms::Tag::SMPP_DATA_CODING, DataCoding::LATIN1);
            }

            try {
                if (outLen <= MAX_ALLOWED_MESSAGE_LENGTH && !info.useDataSm()) {
                    sms.setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, out, (unsigned)outLen);
                    sms.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH, (unsigned)outLen);
                } else if ( info.getDeliveryMode() != DLVMODE_SMS ) {
                    // ussdpush or ussdpushvlr
                    if (outLen > MAX_ALLOWED_MESSAGE_LENGTH ) {
                        smsc_log_warn(log_,"ussdpush: max allowed msg length reached: %u",unsigned(outLen));
                        outLen = MAX_ALLOWED_MESSAGE_LENGTH;
                    }
                    sms.setBinProperty(smsc::sms::Tag::SMPP_SHORT_MESSAGE, out, (unsigned)outLen);
                    sms.setIntProperty(smsc::sms::Tag::SMPP_SM_LENGTH, (unsigned)outLen);
                } else {
                    if (outLen > MAX_ALLOWED_PAYLOAD_LENGTH) {
                        outLen = MAX_ALLOWED_PAYLOAD_LENGTH;
                    }
                    sms.setBinProperty(smsc::sms::Tag::SMPP_MESSAGE_PAYLOAD, out, (unsigned)outLen);
                }
            } catch ( std::exception& e ) {
                what = "wrong message body";
                res = smsc::system::Status::SYSERR;
                break;
            }

            if ( info.getDeliveryMode() != DLVMODE_SMS ) {
                // ussdpush
                const int ussdop = ( info.getDeliveryMode() == DLVMODE_USSDPUSH ?
                                     smscConfig_.ussdPushOp : smscConfig_.ussdPushVlrOp );
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

            const unsigned chunkLen = cs.getMaxMessageChunkSize();
            if (chunkLen>0 && outLen > chunkLen) {
                // SMS will be splitted into nchunks chunks (estimation)
                nchunks = unsigned(outLen-1) / chunkLen + 1;
            } else {
                nchunks = 1;
            }

            drm->nchunks = nchunks;
            smsc_log_debug(log_,"S='%s' R=%u/D=%u/M=%llu %s seq=%u .%u.%u.%*.*s -> .%u.%u.%*.*s",
                           smscId_.c_str(),
                           ptr.getRegionId(),
                           info.getDlvId(),
                           ulonglong(msg.msgId),
                           info.useDataSm() ? "data_sm" : "submit_sm",
                           seqNum,
                           oa.type, oa.plan, oa.length, oa.length, oa.value,
                           da.type, da.plan, da.length, da.length, da.value );
            if (info.useDataSm()) {
                msgtime_type validityDate = msg.timeLeft + msg.lastTime;
                validityDate = ((validityDate <= now) ? 0 : (validityDate-now));
                // FIXME: apply activity period
                sms.setIntProperty( smsc::sms::Tag::SMPP_QOS_TIME_TO_LIVE, validityDate );
                PduDataSm dataSm;
                dataSm.get_header().set_sequenceNumber(seqNum);
                dataSm.get_header().set_commandId(smsc::smpp::SmppCommandSet::DATA_SM);
                fillDataSmFromSms(&dataSm,&sms);
                session_->getAsyncTransmitter()->sendPdu(&(dataSm.get_header()));
            } else {
                PduSubmitSm submitSm;
                submitSm.get_header().set_sequenceNumber(seqNum);
                submitSm.get_header().set_commandId(smsc::smpp::SmppCommandSet::SUBMIT_SM);
                fillSmppPduFromSms(&submitSm, &sms);
                session_->getAsyncTransmitter()->sendPdu(&(submitSm.get_header()));
            }
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
            smsc_log_debug(log_,"S='%s' R=%u/D=%u/M=%llu A=.%u.%u.%0*.*llu seq=%u sent",
                           smscId_.c_str(),
                           ptr.getRegionId(),
                           info.getDlvId(),
                           ulonglong(msg.msgId),
                           ton,npi,len,len,ulonglong(addr), seqNum);
        }
        return res;
    }

    if (seqNum!=0) {
        seqnumHash_.Delete(seqNum);
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


void SmscSender::updateConfig( const SmscConfig& config )
{
    stop();
    MutexGuard mg(reconfLock_);
    smscConfig_ = config;
    session_.reset(new smsc::sme::SmppSession(smscConfig_.smeConfig,this));
}


void SmscSender::detachRegionSender( RegionSender& rs )
{
    smsc_log_debug(log_,"S='%s' detaching regsend R=%u",smscId_.c_str(),unsigned(rs.getRegionId()));
    MutexGuard mg(reconfLock_);
    scoredList_.remove(ScoredList<SmscSender>::isEqual(&rs));
}


void SmscSender::attachRegionSender( RegionSender& rs )
{
    smsc_log_debug(log_,"S='%s' attaching regsend R=%u",smscId_.c_str(),unsigned(rs.getRegionId()));
    {
        MutexGuard mg(reconfLock_);
        scoredList_.add(&rs);
    }
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
    session_->close();
}


void SmscSender::handleReceipt( smsc::sme::SmppHeader* pdu )
{
    assert(pdu);
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
        return; 
    }

    uint8_t msgState;
    int err;
    ResponseData rd;
    const char* msgid = parser_->parseSms(sms,rd.rcptId.msgId,msgState,err);
        
    if ( !msgid || *msgid == '\0') {
        // broken msgid
        return;
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
    case smsc::smpp::SmppMessageState::UNKNOWN:
    case smsc::smpp::SmppMessageState::ACCEPTED:
    case smsc::smpp::SmppMessageState::REJECTED:
    case smsc::smpp::SmppMessageState::UNDELIVERABLE:
        // permanent failure
        break;
    default:
        smsc_log_warn(log_,"S='%s' sms msgid='%s' seq=%u invalid receipt state=%d",
                      smscId_.c_str(), msgid, pdu->get_sequenceNumber(), msgState);
        break;
    } // switch msgState
    
    if ( rd.status == smsc::system::Status::OK && !delivered ) {
        smsc_log_warn(log_,"S='%s' sms msgid='%s' seq=%u receipt has status=OK but not delivered",
                      smscId_.c_str(), msgid, pdu->get_sequenceNumber());
        rd.status = smsc::system::Status::UNKNOWNERR;
    }

    rd.rcptId.setMsgId(msgid);
    rd.retry = retry;

    if (queueData(rd)) {
        switch (pdu->get_commandId()) {
        case smsc::smpp::SmppCommandSet::DELIVERY_SM: {
            PduDeliverySmResp smResp;
            smResp.get_header().set_commandId(smsc::smpp::SmppCommandSet::DELIVERY_SM_RESP);
            smResp.set_messageId("");
            smResp.get_header().set_sequenceNumber(pdu->get_sequenceNumber());
            session_->getAsyncTransmitter()->sendDeliverySmResp(smResp);
            break;
        }
        case smsc::smpp::SmppCommandSet::DATA_SM: {
            PduDataSmResp smResp;
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
        smsc_log_info(log_,"S='%s' sms msgid='%s' seq=%u wasn't accepted, errcode=%d",
                      smscId_.c_str(), rd.rcptId.msgId, rd.seqNum, rd.status );
    }
    rd.retry = rd.wantRetry(rd.status);
    queueData(rd);
}


bool SmscSender::queueData( const ResponseData& rd )
{
    if (isStopping_) return false;
    smsc_log_debug(log_,"S='%s' %s pending seq=%u status=%d msgid='%s'",
                   smscId_.c_str(),
                   rd.seqNum ? "response" : "receipt",
                   rd.seqNum, rd.status, rd.rcptId.msgId );
    MutexGuard mg(queueMon_);
    if (isStopping_) return false;
    wQueue_->Push(rd);
    queueMon_.notify();
    return true;
}


void SmscSender::processQueue( DataQueue& queue )
{
    ResponseData rd;
    // FIXME: preprocess queue to match resp+rcpt

    while ( queue.Pop(rd) ) {

        smsc_log_debug(log_,"FIXME: S='%s' processing RD(seq=%u,status=%d,msgid='%s',retry=%d)",
                       smscId_.c_str(),rd.seqNum,rd.status,rd.rcptId.msgId,rd.retry);

        if ( rd.seqNum ) {
            // it is a response

            DRMTrans drm;
            if ( !seqnumHash_.Pop(rd.seqNum,drm) ) {
                smsc_log_warn(log_,"S='%s' resp seq=%u has no drm mapping", smscId_.c_str(), rd.seqNum);
                continue;
            }
            // FIXME: notify on seqnumHash_.Count() change

            if (drm.trans) {
                smsc_log_info(log_,"S='%s' resp seq=%u R=%u/D=%u/M=%llu ends transaction",
                              smscId_.c_str(), rd.seqNum,
                              drm.regId,
                              drm.dlvId,
                              ulonglong(drm.msgId));
                rproc_.receiveReceipt( drm, retryPolicy_, rd.status, rd.retry, drm.nchunks );
            } else if ( *rd.rcptId.msgId == '\0' ) {
                smsc_log_warn(log_,"FIXME: S='%s' resp seq=%u R=%u/D=%u/M=%llu has no msgId, finalize?",
                              smscId_.c_str(), rd.seqNum,
                              drm.regId,
                              drm.dlvId,
                              ulonglong(drm.msgId));
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

                // finalize message, ignoring resp status
                smsc_log_info(log_,"FIXME: S='%s' R=%u/D=%u/M=%llu ignoring resp status=%d,retry=%d, using status=%d,retry=%d",
                              drm.regId,
                              drm.dlvId,
                              ulonglong(drm.msgId),
                              rd.status, rd.retry, iter->status, iter->retry );
                rproc_.receiveReceipt( drm, retryPolicy_, iter->status, iter->retry, drm.nchunks );
                continue;

            }

            if ( rd.status != smsc::system::Status::OK ) {
                rproc_.receiveReceipt( drm, retryPolicy_, rd.status, rd.retry, drm.nchunks );
                continue;
            }

            if ( !rproc_.receiveResponse( drm ) ) {
                continue;
            }

            // receipt hash has no mapping, adding one
            iter = rcptList.insert(rcptList.begin(),ReceiptData());
            iter->drmId = drm;
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
            ReceiptTimer rt;
            const msgtime_type now = msgtime_type(currentTimeMicro() / tuPerSec);
            rt.endTime = now + rproc_.getCS().getReceiptWaitTime();
            rt.rcptId = rd.rcptId;
            rcptWaitQueue_.Push(rt);

        } else {
            // receipt

            ReceiptList rcptList;
            ReceiptList::iterator* piter = receiptHash_.GetPtr(rd.rcptId.msgId);
            if (piter) {
                ReceiptList::iterator iter = *piter;
                if (iter->responded) {
                    // we have already received a response
                    if ( rd.retry ||
                         rd.status == smsc::system::Status::OK ||
                         smsc::system::Status::isErrorPermanent(rd.status) ) {
                        // if receipt is in bad state -- finalize
                        receiptHash_.Delete(rd.rcptId.msgId);
                        {
                            smsc::core::synchronization::MutexGuard mg(receiptMon_);
                            if (rollingIter_ == iter) { ++rollingIter_; }
                            rcptList.splice(rcptList.begin(),receiptList_,iter);
                        }
                        rproc_.receiveReceipt(iter->drmId, retryPolicy_, rd.status, rd.retry, iter->responded );
                    } else {
                        smsc_log_warn(log_,"FIXME: S='%s' strange receipt for R=%u/D=%u/M=%llu status=%d,msgid='%s',retry=%d",
                                      iter->drmId.regId,
                                      iter->drmId.dlvId,
                                      iter->drmId.msgId,
                                      rd.status,rd.rcptId.msgId,rd.retry);
                        iter->status = rd.status;
                    }
                } else {
                    // still not responded, modify receipt status
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
}


void SmscSender::start()
{
    if (!isStopping_) return;
    {
        MutexGuard mg(queueMon_);
        if (!isStopping_) return;
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
    if (isStopping_) return;
    {
        MutexGuard mg(queueMon_);
        if (isStopping_) return;
        isStopping_ = true;
        awaken_ = true;
        queueMon_.notifyAll();
    }
    journal_->stop();
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
    while ( !isStopping_ ) {
        try {
            connectLoop();
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"exc in connectLoop: %s", e.what());
        }
        if (isStopping_) break;
        journal_->start();
        try {
            sendLoop();
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"exc in sendLoop: %s", e.what());
        }
        journal_->stop();
    }
    return 0;
}


void SmscSender::connectLoop()
{
    while ( !isStopping_) {

        const usectime_type startingConn = currentTimeMicro();
        msgtime_type interConnect;
        {
            MutexGuard mg(reconfLock_);
            smsc_log_debug(log_,"S='%s': trying to connect",smscId_.c_str());
            if ( !session_.get() ) {
                throw InfosmeException(EXC_LOGICERROR,"session is not configured");
            } else if ( !session_->isClosed() ) {
                // session connected
                break;
            }
            try {
                interConnect = smscConfig_.interConnectPeriod;
                session_->connect();
                if (!session_->isClosed()) break;
            } catch ( std::exception& e ) {
                smsc_log_error(log_,"connection failed: %s", e.what());
            }
        }
        // connection failed, waiting
        MutexGuard mg(queueMon_);
        if (isStopping_) break;
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
    // usectime_type movingStart = currentTime_;
    usectime_type nextWakeTime = currentTime_;
    std::auto_ptr<DataQueue> rQueue(new DataQueue);
    while ( !isStopping_ ) {

        currentTime_ = currentTimeMicro();
        if (rQueue->Count() == 0) {

            MutexGuard mg(queueMon_);
            if (wQueue_->Count()>0) {
                // taking wqueue
                std::swap(rQueue,wQueue_);
            } else if (awaken_) {
                awaken_ = false;

            } else {

                // sleeping until next wake time
                int waitTime = int((nextWakeTime - currentTime_ + 1000)/1000U); // in msec
                if (waitTime>0) {
                    if (waitTime < 10) waitTime = 10;
                    smsc_log_debug(log_,"S='%s' is going to sleep %d msec",
                                   smscId_.c_str(),waitTime);
                    queueMon_.wait(waitTime);
                    if (wQueue_->Count()>0) {
                        std::swap(rQueue,wQueue_);
                    }
                    continue;
                }
            }
        }


        MutexGuard mg(reconfLock_);  // prevent reconfiguration
        if ( !session_.get() || session_->isClosed() ) break;
        if (rQueue->Count() > 0) {
            processQueue(*rQueue.get());
        }
        nextWakeTime = currentTime_ + scoredList_.processOnce(0/* not used*/,
                                                              sleepTime);
        processWaitingEvents();
    }
    {
        MutexGuard mg(reconfLock_);
        if (session_.get() && !session_->isClosed()) session_->close();
        if (rQueue->Count() > 0) processQueue(*rQueue.get());
    }
    {
        MutexGuard mg(queueMon_);
        std::swap(rQueue,wQueue_);
    }
    if (rQueue->Count() > 0) processQueue(*rQueue.get());
}


unsigned SmscSender::scoredObjIsReady( unsigned, ScoredObjType& regionSender )
{
    const unsigned ret = regionSender.isReady(currentTime_);
    smsc_log_debug(log_,"R=%u waits %u usec until ready()", regionSender.getRegionId(), ret);
    return ret;
}


int SmscSender::processScoredObj( unsigned, ScoredObjType& regionSender )
{
    unsigned inc = maxScoreIncrement/regionSender.getBandwidth();
    try {
        const unsigned wantToSleep = regionSender.processRegion(currentTime_);
        smsc_log_debug(log_,"R=%u processed, sleep=%u", regionSender.getRegionId(), wantToSleep);
        if (wantToSleep>0) {
            // all deliveries want to sleep
            regionSender.suspend(currentTime_ + wantToSleep);
            return -inc;
        }
    } catch ( std::exception& e ) {
        smsc_log_debug(log_,"R=%u send exc: %s", regionSender.getRegionId(), e.what());
        regionSender.suspend(currentTime_ + 1000000U);
    }
    return inc;
}


void SmscSender::scoredObjToString( std::string& s, ScoredObjType& regionSender )
{
    s += regionSender.toString();
}


void SmscSender::processWaitingEvents()
{
    smsc_log_error(log_,"FIXME: S='%s'@%p process waiting events at %llu",
                   smscId_.c_str(), this, currentTime_);
}


void SmscSender::journalReceiptData( const ReceiptData& rd )
{
    assert(rd.responded);
}

}
}
