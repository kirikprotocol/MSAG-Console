#ifndef SMSC_INFOSME_SMSCCONNECTOR
#define SMSC_INFOSME_SMSCCONNECTOR

#include <logger/Logger.h>
#include <sms/sms.h>
#include <sme/SmppBase.hpp>
#include "util/config/Manager.h"
#include "util/config/ConfigView.h"
#include "core/threads/Thread.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "InfoSmePduListener.h"
#include "core/buffers/Array.hpp"
#include "core/buffers/IntHash.hpp"
#include "core/buffers/XHash.hpp"
#include "TaskTypes.hpp"

const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;
const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;

namespace smsc {
namespace infosme {

using smsc::logger::Logger;
using smsc::core::synchronization::EventMonitor;
using smsc::core::synchronization::Mutex;
using smsc::util::config::ConfigView;

/*
class InfoSmeConfig : public smsc::sme::SmeConfig {
public:
    InfoSmeConfig(ConfigView& config) throw(ConfigException);
    // virtual ~InfoSmeConfig();
private:
    char* strHost;
    char* strSid;
    char* strPassword;
    char* strSysType;
    char* strOrigAddr;
};
 */

using smsc::sme::SmppSession;

/*
struct SeqNum {
  SeqNum():seqNum(0) {};
  SeqNum(uint64_t seqnum, const std::string& smscid):seqNum(sn), smscId(smscid) {};
  uint64_t seqNum;
  string smscId;
};*/

class Task;
class TaskProcessor;
class StatisticsManager;

class SmscConnector : public smsc::core::threads::Thread 
{
private:
    typedef enum { processResponseMethod, processReceiptMethod } EventMethod;
    class EventRunner;
    struct JStoreWrapper;
    class RegionTrafficControl;

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
    static SmeConfig readSmeConfig( ConfigView& config ); // throw ConfigException

    SmscConnector(TaskProcessor& processor,
                  const smsc::sme::SmeConfig& cfg,
                  const string& smscId );
    virtual ~SmscConnector();

    int Execute();
    void stop();
    void reconnect();
    void updateConfig( const smsc::sme::SmeConfig& config );
    bool isStopped() const;
    int getSeqNum();
    uint32_t sendSms(const string& org, const string& dst, const string& txt, bool flash);
    const string& getSmscId() const { return smscId_; };

    // put resp
    bool invokeProcessReceipt( const ResponseData& data );
    bool invokeProcessResponse( const ResponseData& data );
    bool send( Task* task, Message& message );
    void processWaitingEvents( time_t tm );

private:
    bool convertMSISDNStringToAddress(const char* string, smsc::sms::Address& address);

    // merge state for given receipt
    bool mergeReceiptState( const ReceiptId& receiptId, ReceiptData& rd );
    void processResponse( const ResponseData& rd, bool internal = false );
    void processReceipt( const ResponseData& rd, bool internal = false );
    bool send( const std::string& abonent,
               const std::string& message,
               const TaskInfo& info,
               int seqNum );

private:
    string smscId_;
    Logger* log_;
    TaskProcessor& processor_;
    InfoSmePduListener listener_;
    SmppSession session_;
    Mutex sendLock_;
    EventMonitor connectMonitor_;
    int timeout_;
    bool stopped_;
    bool connected_;
    // how many dependant objects are working on the connectors,
    // guarded by connectMonitor_
    int usage_;
    smsc::core::buffers::IntHash<TaskMsgId>   taskIdsBySeqNum;
    smsc::core::synchronization::EventMonitor taskIdsBySeqNumMonitor;
    smsc::core::buffers::XHash<ReceiptId, ReceiptData, ReceiptId>  receipts; 
    smsc::core::synchronization::Mutex              receiptsLock;

    smsc::core::synchronization::Mutex              responseWaitQueueLock;
    smsc::core::synchronization::Mutex              receiptWaitQueueLock;
    smsc::core::buffers::Array<ResponseTimer>       responseWaitQueue;
    smsc::core::buffers::Array<ReceiptTimer>        receiptWaitQueue;
    int                                             responseWaitTime;
    int                                             receiptWaitTime;
    // JStore<ReceiptId, TaskMsgId, ReceiptId>         jstore;
    JStoreWrapper*                                  jstore_;
    RegionTrafficControl*                           trafficControl_;
};

} //infosme
} //smsc

#endif

