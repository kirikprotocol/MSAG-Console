#ifndef _INFORMER_SMSCSENDER_H
#define _INFORMER_SMSCSENDER_H

#include <memory>
#include "ResponseData.h"
#include "ScoredPtrList.h"
#include "core/buffers/CyclicQueue.hpp"
#include "core/synchronization/EventMonitor.hpp"
#include "core/threads/Thread.hpp"
#include "informer/data/RetryPolicy.h"
#include "informer/io/Typedefs.h"
#include "informer/opstore/RegionalStorage.h"
#include "logger/Logger.h"
#include "sme/SmppBase.hpp"

namespace smsc {
namespace sms {
class IllFormedReceiptParser;
}
}

namespace eyeline {
namespace informer {

class ReceiptProcessor;
class RegionSender;
class Message;

struct SmscConfig
{
    smsc::sme::SmeConfig smeConfig;
    int interConnectPeriod;
    int ussdPushOp;
    int ussdPushVlrOp;
    timediff_type minValidityTime;
    timediff_type maxValidityTime;
};


/// sending messages to one smsc
class SmscSender : public smsc::core::threads::Thread, public smsc::sme::SmppPduEventListener
{
    friend class ScoredPtrList< SmscSender >;
    typedef smsc::core::buffers::CyclicQueue< ResponseData > DataQueue;
    typedef std::list< ReceiptData > ReceiptList;
    class SmscJournal;

    struct ResponseTimer;
    typedef std::multimap< msgtime_type, ResponseTimer > RespWaitQueue;

    struct DRMTrans : public DlvRegMsgId {
        unsigned                nchunks;
        bool                    trans;
        msgtime_type            endTime; // receipt wait time
        RespWaitQueue::iterator respTimer;
    };

    struct ResponseTimer
    {
        int                      seqNum;
        DRMTrans*                drmPtr;
    };

public:
    SmscSender( ReceiptProcessor&       rproc,
                const std::string&      smscId,
                const SmscConfig&       config,
                smsc::util::config::Config* retryConfig );

    virtual ~SmscSender();

    const std::string& getSmscId() const { return smscId_; }

    const RetryPolicy& getRetryPolicy() const { return retryPolicy_; }

    void start();
    void stop();
    /// sent when something is changing
    void wakeUp();

    /// sending one message, filling nchunks.
    /// @param untilAE -- number of seconds until activeEnd (or 0 if unlimited).
    /// @return SMPP status
    int send( RegionalStorage& dlv, Message& msg, int untilAE, int& nchunks );

    /// return smpp status
    int sendTestSms( const char*   sourceAddr,
                     personid_type subscriber,
                     const char*   text,
                     bool          isFlash,
                     int           deliveryMode );

    /// a method allows to wait until sender stops it work
    /// NOTE: post-requisite -- task is released!
    void updateConfig( const SmscConfig& config,
                       smsc::util::config::Config* retryConfig );
    // void waitUntilReleased();

    /// this two methods are invoked from locked state.
    void detachRegionSender( RegionSender& rs );
    void attachRegionSender( RegionSender& rs );
    void getRegionList( std::vector< regionid_type >& regions );

private:
    virtual void handleEvent( smsc::sme::SmppHeader* pdu );
    virtual void handleError( int errorcode );
    void handleReceipt( smsc::sme::SmppHeader* pdu );
    void handleResponse( smsc::sme::SmppHeader* pdu );
    bool queueData( const ResponseData& rd );

    /// process input queue
    /// @return true if response with status=THROTTLED encountered.
    bool processQueue( DataQueue& queue );

    /// sending messages
    virtual int Execute();

    // virtual void onThreadPoolStartTask();
    // virtual void onRelease();

    void connectLoop();
    void sendLoop();

    typedef RegionSender* ScoredPtrType;

    unsigned scoredObjIsReady( unsigned unused, ScoredPtrType regionSender );
    int processScoredObj( unsigned unused, ScoredPtrType regionSender, unsigned& objSleep );
    void scoredObjToString( std::string& s, ScoredPtrType regionSender );

    void processExpiredTimers();
    // void journalReceiptData( const ReceiptData& rd );

    bool getNextRollingData( ReceiptData& rd, bool firstPass );

private:
    smsc::logger::Logger*                     log_;
    ReceiptProcessor&                         rproc_;
    RetryPolicy                               retryPolicy_;
    smsc::sms::IllFormedReceiptParser*        parser_;
    std::string                               smscId_;
    SmscConfig                                smscConfig_;

    smsc::core::synchronization::Mutex                reconfLock_;
    std::auto_ptr<smsc::sme::SmppSession>             session_;
    ScoredPtrList< SmscSender >                       scoredList_; // not owned
    usectime_type                                     currentTime_;

    smsc::core::buffers::IntHash< DRMTrans >          seqnumHash_;
    std::multimap<msgtime_type, ResponseTimer>        respWaitQueue_;
    // FIXME: optimize make timers ala resptimer
    std::multimap<msgtime_type, ReceiptId>            rcptWaitQueue_;

    // when process receipt/response.
    smsc::core::synchronization::EventMonitor          receiptMon_;
    ReceiptList                                        receiptList_;
    ReceiptList::iterator                              rollingIter_;
    smsc::core::buffers::Hash< ReceiptList::iterator > receiptHash_;

    SmscJournal*                                       journal_;
    DataQueue*                                         rQueue_;

    // queueMon only to put into queue and notify
    smsc::core::synchronization::EventMonitor          queueMon_;
    std::auto_ptr<DataQueue>                           wQueue_;
    bool                                               awaken_;
    bool                                               isStopping_;
};

} // informer
} // smsc

#endif
