#include "PerformanceTester.h"
#include "InfoSmePduListener.h"
#include <map>

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
        smsc::core::synchronization::MutexGuard mg(lock);
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
    smsc::core::synchronization::MutexGuard mg(mon_);
    if (started_) return;
    started_ = true;
    sent_ = resp_ = recp_ = 0;
    Start();
}


void PerformanceTester::stop()
{
    if (!started_) return;
    {
        smsc::core::synchronization::MutexGuard mg(mon_);
        if (!started_) return;
        started_ = false;
        mon_.notifyAll();
    }
    WaitFor();
}


int32_t PerformanceTester::getNextSeqNum()
{
    smsc::core::synchronization::MutexGuard mg(seqLock_);
    return ++seqNum_;
}


int PerformanceTester::Execute()
{
    smsc_log_info(log_,"PerformanceTester for %s is started", smscId_.c_str());
    SmppHeader* hdr;
    msectime_type startTime = currentTimeMillis();
    const msectime_type statInterval = 10000;
    typedef std::multimap<msectime_type,SmppHeader*> TimeQueue;
    TimeQueue timeQueue;
    msectime_type nextTime = startTime;
    while (true) {

        const msectime_type now = currentTimeMillis();
        bool isStopping = false;
        {
            smsc::core::synchronization::MutexGuard mg(mon_);

            const msectime_type interval = now - startTime;
            if ( interval >= statInterval ) {
                // statistics
                const unsigned ints = unsigned((interval+500)/1000);
                const unsigned halfInt = ints/2;
                smsc_log_info(log_, "Statistics (%s): interval=%llums iqsz=%u oqsz=%u\n"
                              "  Total      : sent=%llu resp=%llu recp=%llu\n"
                              "  Speed (1/s): sent=%llu resp=%llu recp=%llu",
                              smscId_.c_str(),
                              interval,
                              unsigned(inputQueue_.evaluateCount()),
                              unsigned(timeQueue.size()),
                              sent_, resp_, recp_,
                              (sent_+halfInt)/ints,
                              (resp_+halfInt)/ints,
                              (recp_+halfInt)/ints );
                // reset statistics
                startTime = now;
                sent_ = resp_ = recp_ = 0;
            }
            isStopping = !started_;
            int waitTime = int(nextTime - now);
            if ( started_ && waitTime > 30 ) {
                mon_.wait(waitTime);
                continue;
            }
        }

        // reading all input queue
        {
            SmppEntry entry;
            while ( inputQueue_.Pop(entry) ) {
                timeQueue.insert(std::make_pair(entry.recvTime,entry.pdu));
            }
        }

        // processing all sms
        TimeQueue::iterator imax = isStopping ? timeQueue.end() : timeQueue.upper_bound(now);

        for ( TimeQueue::const_iterator i = timeQueue.begin();
              i != imax; ++i ) {

            try {
                listener_->handleEvent(i->second);
            } catch ( std::exception& e ) {
                smsc_log_warn(log_,"exception in handling: %s",e.what());
            } catch (...) {
                smsc_log_warn(log_,"unknown exception in handling");
            }
        }
        timeQueue.erase(timeQueue.begin(), imax);
        if ( !timeQueue.empty() ) {
            nextTime = timeQueue.begin()->first;
        } else {
            nextTime = now + 100;
        }
        if (isStopping) break;
    }
    smsc_log_info(log_,"PerformanceTester for %s is stopped", smscId_.c_str());
    return 0;
}


void PerformanceTester::pushEvents( SmppHeader* submit, PduXSmResp* resp, PduXSm* recp )
{
    {
        smsc::core::synchronization::MutexGuard mg(mon_);
        if (!started_) {
            delete resp;
            delete recp;
            return;
        }
        if (submit) ++sent_;
        if (resp) ++resp_;
        if (recp) ++recp_;
        if (inputQueue_.evaluateCount() > 100) {
            mon_.notify();
        }
    }
    if (submit) {
        smsc_log_debug(log_,"sending submit #%u",submit->get_sequenceNumber());
    }
    const msectime_type now = currentTimeMillis();
    if (resp) {
        smsc_log_debug(log_,"recving submit_resp #%u, msgid='%s'",
                       resp->get_header().get_sequenceNumber(),
                       resp->get_messageId());
        const unsigned deltaResp = 10 + 
            unsigned(reinterpret_cast<uint64_t>(static_cast<void*>(resp)) % 100);
        inputQueue_.Push(SmppEntry(now+deltaResp,(SmppHeader*)resp));
    }
    if (recp) {
        smsc_log_debug(log_,"recving delivery(receipt) #%u, msgid='%s'",
                       recp->get_header().get_sequenceNumber(),
                       recp->get_optional().get_receiptedMessageId());
        const unsigned deltaRecp = 3000 +
            unsigned(reinterpret_cast<uint64_t>(static_cast<void*>(resp)) % 3000);
        inputQueue_.Push(SmppEntry(now+deltaRecp,(SmppHeader*)recp));
    }
}

}
}
