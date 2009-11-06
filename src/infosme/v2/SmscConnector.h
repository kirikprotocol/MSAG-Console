#ifndef SMSC_INFOSME2_SMSCCONNECTOR_H
#define SMSC_INFOSME2_SMSCCONNECTOR_H

#include <memory>
#include "logger/Logger.h"
#include "sme/SmppBase.hpp"

// const int   MAX_ALLOWED_PAYLOAD_LENGTH = 65535;
// const int   MAX_ALLOWED_MESSAGE_LENGTH = 254;

namespace smsc {

namespace util { namespace config { class ConfigView; }}

namespace infosme2 {

class Task;
class Message;

class SmscConnector
{
/*
private:
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
*/

public:
    static smsc::sme::SmeConfig readSmeConfig( smsc::util::config::ConfigView& config ); // throw ConfigException

    SmscConnector( const std::string& smscId,
                   const smsc::sme::SmeConfig& cfg );
    virtual ~SmscConnector();

    void updateConfig( const smsc::sme::SmeConfig& config );

    const std::string& getId() const { return id_; }

    /*
    int Execute();
    void stop();
    void reconnect();
    bool isStopped() const;
    int getSeqNum();
    uint32_t sendSms(const string& org, const string& dst, const string& txt, bool flash);
    const string& getSmscId() const { return smscId_; };

    // put resp
    bool invokeProcessReceipt( const ResponseData& data );
    bool invokeProcessResponse( const ResponseData& data );
    bool send( Task* task, Message& message, const smsc::util::config::region::Region* region );
    void processWaitingEvents( time_t tm );
     */

    bool send( const Task& task, const Message& msg );

private:
    /*
    bool convertMSISDNStringToAddress(const char* string, smsc::sms::Address& address);

    // merge state for given receipt
    bool mergeReceiptState( const ReceiptId& receiptId, ReceiptData& rd );
    void processResponse( const ResponseData& rd, bool internal = false );
    void processReceipt( const ResponseData& rd, bool internal = false );
    bool send( const std::string& abonent,
               const std::string& message,
               const TaskInfo& info,
               int seqNum );
    // cleanup all hashes except receipts
    void clearHashes();
     */
private:
    smsc::logger::Logger* log_;
    std::string           id_;
    /*
    TaskProcessor& processor_;
    int timeout_;

    // monitor which guards state change
    EventMonitor stateMonitor_;
    bool stopped_;
    bool connected_;

    // monitor which guards session destruction and self destruction
    EventMonitor destroyMonitor_;
    InfoSmePduListener listener_;
    std::auto_ptr<SmppSession> session_;
    int usage_; // how many dependant objects are on this
    JStoreWrapper*                                  jstore_;
    RegionTrafficControl*                           trafficControl_;

    smsc::core::buffers::IntHash<TaskMsgId>         taskIdsBySeqNum;
    smsc::core::synchronization::EventMonitor       taskIdsBySeqNumMonitor;
    smsc::core::buffers::XHash<ReceiptId, ReceiptData, ReceiptId>  receipts; 
    smsc::core::synchronization::Mutex              receiptsLock;

    smsc::core::synchronization::Mutex              responseWaitQueueLock;
    smsc::core::synchronization::Mutex              receiptWaitQueueLock;
    smsc::core::buffers::Array<ResponseTimer>       responseWaitQueue;
    smsc::core::buffers::Array<ReceiptTimer>        receiptWaitQueue;
     */
};

} //infosme2
} //smsc

#endif

