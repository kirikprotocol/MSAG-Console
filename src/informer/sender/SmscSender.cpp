#include "RegionSender.h"
#include "SmscSender.h"
#include "informer/io/InfosmeException.h"
#include "informer/io/Typedefs.h"
#include "informer/data/InfosmeCore.h"
#include "smpp/smpp_structures.h"
#include "system/status.h"
#include "sms/sms.h"
#include "sms/IllFormedReceiptParser.h"

namespace eyeline {
namespace informer {

SmscSender::SmscSender( InfosmeCore& core,
                        const std::string& smscId,
                        const SmscConfig& cfg ) :
log_(smsc::logger::Logger::getInstance("smscsend")),
core_(&core),
smscId_(smscId), session_(0),
scoredList_(*this,2*maxScoreIncrement,
            smsc::logger::Logger::getInstance("reglist")),
isStopping_(true),
ussdPushOp_(cfg.ussdPushOp),
ussdPushVlrOp_(cfg.ussdPushVlrOp)
{
    session_.reset( new smsc::sme::SmppSession(cfg.smeConfig,this) );
}


SmscSender::~SmscSender()
{
    stop();
    if (session_.get()) session_->close();
}


int SmscSender::send( RegionalStorage& ptr, Message& msg )
{
    smsc_log_error(log_,"FIXME: send(R=%u/D=%u/M=%llu)",
                   unsigned(ptr.getRegionId()),
                   unsigned(ptr.getDlvId()),
                   ulonglong(msg.msgId));
    const char* what = "";
    int res = smsc::system::Status::SYSERR;
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

        int seqNum = session_->getNextSeq();

        smsc_log_error(log_,"FIXME: complete sending");

    } while ( false );

    return -res;
}


void SmscSender::updateConfig( const SmscConfig& config )
{
    stop();
    MutexGuard mg(reconfLock_);
    session_.reset(new smsc::sme::SmppSession(config.smeConfig,this));
    ussdPushOp_ = config.ussdPushOp;
    ussdPushVlrOp_ = config.ussdPushVlrOp;
}


/*
void SmscSender::waitUntilReleased()
{
    if (isReleased) return;
    stop();
    while (true) {
        MutexGuard mg(mon_);
        if (isReleased) break;
        mon_.wait(100);
    }
}
 */


void SmscSender::detachRegionSender( RegionSender& rs )
{
    smsc_log_debug(log_,"S='%s' detaching regsend R=%u",smscId_.c_str(),unsigned(rs.getRegionId()));
    {
        MutexGuard mg(reconfLock_);
        scoredList_.remove(ScoredList<SmscSender>::isEqual(&rs));
    }
    // MutexGuard mg(queueMon_);
    // queueMon_.notify();
}


void SmscSender::attachRegionSender( RegionSender& rs )
{
    smsc_log_debug(log_,"S='%s' attaching regsend R=%u",smscId_.c_str(),unsigned(rs.getRegionId()));
    {
        MutexGuard mg(reconfLock_);
        scoredList_.add(&rs);
    }
    MutexGuard mg(queueMon_);
    queueMon_.notify();
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
    smsc_log_warn(log_,"FIXME: receipt");
    assert(pdu);
    bool needResponse = true;
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
    
    if ( isReceipt ) {
        ResponseData rd;
        uint8_t msgState;
        int err;
        const char* msgid = parser_->parseSms(sms,rd.msgid,msgState,err);
        if (msgid && *msgid != '\0') {
            // msgid is ok
            rd.receipt = true;
            rd.retry = false;

            bool delivered = false;
            switch (msgState) {
            case smsc::smpp::SmppMessageState::DELIVERED:
                delivered = true;
                break;
            case smsc::smpp::SmppMessageState::EXPIRED:
            case smsc::smpp::SmppMessageState::DELETED:
                rd.retry = true;
                break;
            case smsc::smpp::SmppMessageState::ENROUTE:
            case smsc::smpp::SmppMessageState::UNKNOWN:
            case smsc::smpp::SmppMessageState::ACCEPTED:
            case smsc::smpp::SmppMessageState::REJECTED:
            case smsc::smpp::SmppMessageState::UNDELIVERABLE:
                // what to do?
                break;
            default:
                smsc_log_warn(log_,"S='%s' sms msgid='%s' seq=%u invalid receipt state=%d",
                              smscId_.c_str(), msgid, pdu->get_sequenceNumber(), msgState);
                break;
            } // switch msgState

            rd.seqNum = 0;
            rd.status = pdu->get_commandStatus();
            if ( rd.status == smsc::system::Status::OK && !delivered ) {
                smsc_log_warn(log_,"S='%s' sms msgid='%s' seq=%u receipt has status=OK but not delivered",
                              smscId_.c_str(), msgid, pdu->get_sequenceNumber());
                rd.status = smsc::system::Status::UNKNOWNERR;
            }
            if (msgid != rd.msgid) {
                rd.setMsgId(msgid);
            }
            needResponse = queueData(rd);
        }
    }
    
    if (needResponse) {
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
    rd.receipt = rd.retry = false;
    rd.seqNum = pdu->get_sequenceNumber();
    rd.status = pdu->get_commandStatus();
    bool accepted = ( rd.status == smsc::system::Status::OK );
    const char* msgid = reinterpret_cast<smsc::sme::PduXSmResp*>(pdu)->get_messageId();
    {
        const char* passedid = msgid;
        if (!msgid || *msgid == '\0') {
            accepted = false;
            passedid = "";
        }
        rd.setMsgId(passedid);
    }
    if (!accepted) {
        smsc_log_info(log_,"S='%s' sms msgid='%s' seq=%u wasn't accepted, errcode=%d",
                      smscId_.c_str(), msgid, rd.seqNum, rd.status );
    }
    queueData(rd);
}


bool SmscSender::queueData( const ResponseData& rd )
{
    if (isStopping_) return false;
    MutexGuard mg(queueMon_);
    wQueue_->Push(rd);
    queueMon_.notify();
    return true;
}


void SmscSender::processQueue( DataQueue& queue )
{
    ResponseData rd;
    while ( queue.Pop(rd) ) {
        smsc_log_debug(log_,"FIXME: S='%s' processing RD(seq=%u,status=%d,msgid='%s',rcpt=%d,retry=%d)",
                       smscId_.c_str(),rd.seqNum,rd.status,rd.msgid,rd.receipt,rd.retry);
    }
}


void SmscSender::start()
{
    if (!isStopping_) return;
    MutexGuard mg(queueMon_);
    if (!isStopping_) return;
    isStopping_ = false;
    Start();
}


void SmscSender::stop()
{
    if (!isStopping_) {
        MutexGuard mg(queueMon_);
        isStopping_ = true;
        queueMon_.notifyAll();
    }
    WaitFor();
}


int SmscSender::Execute()
{
    while ( !isStopping_ ) {
        connectLoop();
        if (isStopping_) break;
        sendLoop();
    }
    return 0;
}


/*
void SmscSender::onThreadPoolStartTask()
{
    isStopping_ = false;
    isReleased = false;
}
 */


/*
void SmscSender::onRelease()
{
    MutexGuard mg(mon_);
    isReleased = true;
    mon_.notify();
}
 */


void SmscSender::connectLoop()
{
    while ( !isStopping_ ) {
        MutexGuard mg(queueMon_);
        if ( !session_.get() ) {
            throw InfosmeException("logic error: session is not configured");
            // isStopping_ = true;
        } else if ( !session_->isClosed() ) {
            // session connected
            break;
        }
        try {
            session_->connect();
            if (!session_->isClosed()) break;
        } catch ( std::exception& e ) {
            smsc_log_error(log_,"connection failed: %s", e.what());
        }
        // smsc_log_debug(log_,"FIXME: configure timeout b/w attempts");
        queueMon_.wait(10000);
    }
}


void SmscSender::sendLoop()
{
    const unsigned sleepTime = 5000000U; // 5 sec

    currentTime_ = currentTimeMicro();
    // usectime_type movingStart = currentTime_;
    usectime_type nextWakeTime = currentTime_;
    while ( !isStopping_ ) {

        if ( !session_.get() || session_->isClosed() ) break;

        // sleeping until next wake time
        currentTime_ = currentTimeMicro();
        int waitTime = int((nextWakeTime - currentTime_)/1000U); // in msec

        if (rQueue_->Count() == 0) {

            MutexGuard mg(queueMon_);

            if (wQueue_->Count()>0) {
                // taking wqueue
                std::swap(rQueue_,wQueue_);
            } else if (waitTime > 0) {

                if (waitTime < 10) waitTime = 10;
                smsc_log_debug(log_,"S='%s' is going to sleep %d msec",
                               smscId_.c_str(),waitTime);
                queueMon_.wait(waitTime);
                if (wQueue_->Count()>0) {
                    std::swap(rQueue_,wQueue_);
                }
                continue;
            }
        }

        MutexGuard mg(reconfLock_);  // prevent reconfiguration

        // 1. FIXME: fully process rQueue
        if (rQueue_->Count() > 0) {
            processQueue(*rQueue_);
        }
        if (waitTime>0) {
            continue;
        }

        nextWakeTime = currentTime_ + scoredList_.processOnce(0, sleepTime);
        processWaitingEvents();
    }
    // FIXME: process waiting data from queue
    if (session_.get() && !session_->isClosed()) session_->close();
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

}
}
