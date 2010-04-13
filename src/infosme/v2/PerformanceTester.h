#ifndef INFOSME_PERFORMANCETESTER_H
#define INFOSME_PERFORMANCETESTER_H

#include <string>
#include "sme/SmppBase.hpp"
#include "logger/Logger.h"
#include "core/synchronization/Mutex.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/buffers/FastMTQueue.hpp"
#include "MsecTime.h"

namespace smsc {
namespace infosme {

class InfoSmePduListener;

class PerformanceTester : protected smsc::core::threads::Thread
{
private:
    static smsc::logger::Logger* log_;
    static void initLog();

public:
    PerformanceTester( const std::string& smscId,
                       InfoSmePduListener& listener );
    virtual ~PerformanceTester();

    void connect();
    smsc::smpp::SmppHeader* sendPdu( smsc::smpp::SmppHeader* pdu );
    void sendDeliverySmResp( smsc::smpp::PduDeliverySmResp& resp );
    void sendDataSmResp( smsc::smpp::PduDataSmResp& resp );

    void start();
    void stop();

private:
    int32_t getNextSeqNum();
    virtual int Execute();
    /// send and receive
    void pushEvents( smsc::smpp::SmppHeader* pdu,
                     smsc::smpp::PduXSmResp* resp,
                     smsc::smpp::PduXSm* recp );
    
private:
    std::string         smscId_;
    InfoSmePduListener* listener_; // not owned
    smsc::core::synchronization::Mutex seqLock_;
    int32_t                            seqNum_;

    struct SmppEntry {
        SmppEntry() {}
        SmppEntry( msectime_type t, smsc::smpp::SmppHeader* h ) : recvTime(t), pdu(h) {}
    public:
        msectime_type           recvTime;
        smsc::smpp::SmppHeader* pdu;
    };
    typedef smsc::core::buffers::FastMTQueue< SmppEntry > InputQueue;
    InputQueue inputQueue_;
    smsc::core::synchronization::EventMonitor mon_;

    // statistics
    uint64_t  sent_;  // number of outgoing submits
    uint64_t  resp_;  // number of incoming responses
    uint64_t  recp_;  // number of incoming receipts

    bool  started_;
};

}
}

#endif /* INFOSME_PERFORMANCE_TESTER_H */
