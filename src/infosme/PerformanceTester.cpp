#include "PerformanceTester.h"
#include "InfoSmePduListener.h"
#include "util/TimeSource.h"

namespace {

int64_t messageId = 0;
smsc::core::synchronization::Mutex lock;

int64_t getNextMessageId() {
  smsc::core::synchronization::MutexGuard mg(lock);
  return ++messageId;
}

}

namespace smsc {

using namespace smpp;

namespace infosme {

smsc::logger::Logger* PerformanceTester::log_ = 0;

void PerformanceTester::initLog()
{
    if (!log_) {
        MutexGuard mg(lock);
        if (!log_) log_ = smsc::logger::Logger::getInstance("perftest");
    }
}

PerformanceTester::PerformanceTester( const std::string& smscId,
                                      InfoSmePduListener& listener ) :
smscId_(smscId),
listener_(&listener),
seqNum_(0),
sent_(0),
resp_(0),
recp_(0),
started_(false)
{
    if (!log_) initLog();
    smsc_log_info(log_,"performanceTest %s created",smscId_.c_str());
}


PerformanceTester::~PerformanceTester()
{
    stop();
}


void PerformanceTester::connect()
{
    smsc_log_info(log_,"perftest connected %s",smscId_.c_str());
}


SmppHeader* PerformanceTester::sendPdu( SmppHeader* pdu )
{
    if (!started_) return 0;
    PduXSm* sm;
    std::auto_ptr<PduXSmResp> smresp;
    std::auto_ptr<PduXSm> recp;
    switch ( pdu->get_commandId() ) {
    case SmppCommandSet::SUBMIT_SM: {
        sm = (PduXSm*)pdu;
        char msgId[40];
        sprintf(msgId,"msg%llu",getNextMessageId());
        {
            smresp.reset(new PduXSmResp);
            SmppHeader& header = smresp->get_header();
            header.set_sequenceNumber(pdu->get_sequenceNumber());
            header.set_commandId(SmppCommandSet::SUBMIT_SM_RESP);
            smresp->set_messageId(msgId);
        }
        // listener_->handleEvent((SmppHeader*)smresp.release());
        if (sm->get_message().get_registredDelivery()) {
            // receipt requested
            recp.reset(new PduXSm);
            SmppHeader& header = recp->get_header();
            header.set_sequenceNumber(getNextSeqNum());
            header.set_commandId(SmppCommandSet::DELIVERY_SM);
            PduPartSm& msg = recp->get_message();
            msg.set_source(sm->get_message().get_dest());
            msg.set_dest(sm->get_message().get_source());
            msg.set_esmClass(0x4);
            msg.set_priorityFlag(0);
            SmppOptional& opts = recp->get_optional();
            opts.set_receiptedMessageId(msgId);
            opts.set_messageState(SmppMessageState::DELIVERED);
            // opts.set_networkErrorCode(0);
            // listener_->handleEvent((SmppHeader*)recp.release());
        }
        break;
    }
    case SmppCommandSet::DATA_SM:
    default:
        throw smsc::util::Exception("not implemented");
    };

    // send it
    pushEvents(pdu,smresp.release(),recp.release());
    return 0;
}


void PerformanceTester::sendDeliverySmResp( PduDeliverySmResp& resp )
{
    smsc_log_debug(log_,"sending delivery resp #%u",resp.get_header().get_sequenceNumber());
}


void PerformanceTester::sendDataSmResp( PduDataSmResp& resp )
{
    smsc_log_debug(log_,"sending datasm resp #%u",resp.get_header().get_sequenceNumber());
}


void PerformanceTester::start()
{
    if (started_) return;
    MutexGuard mg(mon_);
    if (started_) return;
    started_ = true;
    sent_ = resp_ = recp_ = 0;
    Start();
}


void PerformanceTester::stop()
{
    if (!started_) return;
    {
        MutexGuard mg(mon_);
        if (!started_) return;
        started_ = false;
        mon_.notifyAll();
    }
    WaitFor();
}


int32_t PerformanceTester::getNextSeqNum()
{
    MutexGuard mg(seqLock_);
    return ++seqNum_;
}


int PerformanceTester::Execute()
{
    smsc_log_info(log_,"PerformanceTester for %s is started", smscId_.c_str());
    SmppHeader* hdr;
    typedef smsc::util::TimeSourceSetup::AbsUSec USec;
    typedef USec::usec_type usec_type;
    usec_type startTime = USec::getUSec();
    const usec_type statInterval = 10 * USec::ticksPerSec;
    while (true) {
        const usec_type now = USec::getUSec();
        {
            MutexGuard mg(mon_);

            const usec_type interval = now - startTime;
            if ( interval >= statInterval ) {
                // statistics
                const usec_type intms = (interval+500)/1000;
                const usec_type ints = (interval+USec::ticksPerSec/2)/USec::ticksPerSec;
                const usec_type halfInt = ints/2;
                smsc_log_info(log_, "Statistics (%s): interval=%llu ms qsz=%u\n"
                              "  Total      : sent=%llu resp=%llu recp=%llu\n"
                              "  Speed (1/s): sent=%llu resp=%llu recp=%llu",
                              smscId_.c_str(),
                              intms,
                              events_.evaluateCount(),
                              sent_, resp_, recp_,
                              (sent_+halfInt)/ints,
                              (resp_+halfInt)/ints,
                              (recp_+halfInt)/ints );
                // reset statistics
                startTime = now;
                sent_ = resp_ = recp_ = 0;
            }

            if ( ! events_.Pop(hdr) ) {
                if (!started_) break;
                // we have to wait a little
                mon_.wait(100);
                continue;
            }
        }
        try {
            listener_->handleEvent(hdr);
        } catch ( std::exception& e ) {
            smsc_log_warn(log_,"exception in handling: %s",e.what());
        } catch (...) {
            smsc_log_warn(log_,"unknown exception in handling");
        }
    }
    smsc_log_info(log_,"PerformanceTester for %s is stopped", smscId_.c_str());
    return 0;
}


void PerformanceTester::pushEvents( SmppHeader* submit, PduXSmResp* resp, PduXSm* recp )
{
    MutexGuard mg(mon_);
    if (!started_) {
        delete resp;
        delete recp;
        return;
    }
    if (submit) {
        ++sent_;
        smsc_log_debug(log_,"sending submit #%u",submit->get_sequenceNumber());
    }
    if (resp) {
        ++resp_;
        smsc_log_debug(log_,"recving submit_resp #%u, msgid='%s'",
                       resp->get_header().get_sequenceNumber(),
                       resp->get_messageId());
        events_.Push((SmppHeader*)resp);
    }
    if (recp) {
        ++recp_;
        smsc_log_debug(log_,"recving delivery(receipt) #%u, msgid='%s'",
                       recp->get_header().get_sequenceNumber(),
                       recp->get_optional().get_receiptedMessageId());
        events_.Push((SmppHeader*)recp);
    }
    mon_.notify();
}

}
}
