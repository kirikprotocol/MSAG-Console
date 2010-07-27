#ifndef SMSC_INFOSME_SMSCCONNECTOR
#define SMSC_INFOSME_SMSCCONNECTOR

#include <memory>
#include <logger/Logger.h>
#include <sms/sms.h>
#include <sme/SmppBase.hpp>
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "InfoSmePduListener.h"
#include "PerformanceTester.h"
#include "core/buffers/Array.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/XHash.hpp"
#include "TaskTypes.hpp"

const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;
const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;

namespace smsc {

namespace util {
namespace config {
namespace region {
class Region;
}
}
}

namespace infosme {

using smsc::logger::Logger;
using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::Mutex;
using smsc::util::config::ConfigView;
using smsc::sme::SmppSession;

class Task;
class TaskProcessor;
class StatisticsManager;

class SmscConnector : protected smsc::core::threads::Thread 
{
private:
    typedef enum { processResponseMethod, processReceiptMethod } EventMethod;
    class EventRunner;
    struct JStoreWrapper;
    // class RegionTrafficControl;

    struct ReceiptData
    {
        bool receipted, delivered, retry;

        ReceiptData(bool receipted=false, bool delivered=false, bool retry=false)
         : receipted(receipted), delivered(delivered), retry(retry) {};
        ReceiptData(const ReceiptData& receipt)
          : receipted(receipt.receipted), delivered(receipt.delivered), retry(receipt.retry) {};

        ReceiptData& operator=(const ReceiptData& receipt) {
            receipted = receipt.receipted;
            delivered = receipt.delivered;
            retry = receipt.retry;
            return *this;
        }
    };

    struct ResponseTimer
    {
        time_t          timer;
        int             seqNum;

        ResponseTimer(time_t timer = 0):timer(timer) {}
        ResponseTimer(time_t timer, int seqNum) : timer(timer), seqNum(seqNum) {}
        ResponseTimer(const ResponseTimer& rt) : timer(rt.timer), seqNum(rt.seqNum) {}
    };

    struct ReceiptTimer
    {
        time_t      timer;
        ReceiptId   receiptId;

        ReceiptTimer(time_t timer=0) : timer(timer) {}
        ReceiptTimer(time_t timer, const ReceiptId& rcptId) : timer(timer), receiptId(rcptId) {}
    };


public:
    struct SmscConfig {
        smsc::sme::SmeConfig smeConfig;
        int  ussdPushOp;
        int  ussdPushVlrOp;
    };

    static SmscConfig readSmeConfig( ConfigView& config ); // throw ConfigException

    SmscConnector(TaskProcessor& processor,
                  const string& smscId,
                  bool doPerfTests = false );
    virtual ~SmscConnector();

    void start();
    void stop();
    void reconnect();
    void updateConfig( const SmscConfig& config );
    bool isStopped() const;
    int getSeqNum();
    uint32_t sendSms(const string& org, const string& dst, const string& txt, bool flash);
    const string& getSmscId() const { return smscId_; };

    // put resp
    bool invokeProcessReceipt( const ResponseData& data );
    bool invokeProcessResponse( const ResponseData& data );
    // bool send( Task* task, Message& message, const smsc::util::config::region::Region* region );
    void processWaitingEvents( time_t tm );

    /// send a message
    /// @return number of chunks sent or 0
    int send( Task& task, Message& msg );

private:
    virtual int Execute();

    // merge state for given receipt
    bool mergeReceiptState( const ReceiptId& receiptId, ReceiptData& rd );
    void processResponse( const ResponseData& rd, bool internal = false );
    void processReceipt( const ResponseData& rd, bool internal = false );
    // cleanup all hashes except receipts
    void clearHashes();

private:
    string smscId_;
    Logger* log_;
    TaskProcessor& processor_;
    int timeout_;
    int ussdPushOp_;
    int ussdPushVlrOp_;

    // monitor which guards state change
    EventMonitor stateMonitor_;
    bool stopped_;
    bool connected_;

    // monitor which guards session destruction and self destruction
    EventMonitor destroyMonitor_;
    InfoSmePduListener listener_;
    std::auto_ptr<SmppSession>                      session_;
    std::auto_ptr<PerformanceTester>                performanceTester_;
    int usage_; // how many dependant objects are on this
    JStoreWrapper*                                  jstore_;
    // RegionTrafficControl*                           trafficControl_;

    smsc::core::buffers::IntHash<TaskMsgId>         taskIdsBySeqNum;
    smsc::core::synchronization::EventMonitor       taskIdsBySeqNumMonitor;
    smsc::core::buffers::XHash<ReceiptId, ReceiptData, ReceiptId>  receipts; 
    smsc::core::synchronization::Mutex              receiptsLock;

    smsc::core::synchronization::Mutex              responseWaitQueueLock;
    smsc::core::synchronization::Mutex              receiptWaitQueueLock;
    smsc::core::buffers::Array<ResponseTimer>       responseWaitQueue;
    smsc::core::buffers::Array<ReceiptTimer>        receiptWaitQueue;
};

} //infosme
} //smsc

#endif

